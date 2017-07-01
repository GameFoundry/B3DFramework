//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeast.h"
#include "BsCCamera.h"
#include "BsCRenderable.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsPass.h"
#include "BsSamplerState.h"
#include "BsCoreApplication.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsRenderQueue.h"
#include "BsCoreThread.h"
#include "BsProfilerCPU.h"
#include "BsProfilerGPU.h"
#include "BsShader.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsTime.h"
#include "BsRenderableElement.h"
#include "BsCoreObjectManager.h"
#include "BsRenderBeastOptions.h"
#include "BsLight.h"
#include "BsGpuResourcePool.h"
#include "BsRenderTargets.h"
#include "BsRendererUtility.h"
#include "BsAnimationManager.h"
#include "BsSkeleton.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsRendererExtension.h"
#include "BsLightProbeCache.h"
#include "BsReflectionProbe.h"
#include "BsIBLUtility.h"
#include "BsLightGrid.h"
#include "BsSkybox.h"
#include "BsShadowRendering.h"
#include "BsStandardDeferredLighting.h"

using namespace std::placeholders;

namespace bs { namespace ct
{
	// Limited by max number of array elements in texture for DX11 hardware
	constexpr UINT32 MaxReflectionCubemaps = 2048 / 6;

	RenderBeast::RenderBeast()
	{
		mOptions = bs_shared_ptr_new<RenderBeastOptions>();
	}

	const StringID& RenderBeast::getName() const
	{
		static StringID name = "RenderBeast";
		return name;
	}

	void RenderBeast::initialize()
	{
		Renderer::initialize();

		gCoreThread().queueCommand(std::bind(&RenderBeast::initializeCore, this), CTQF_InternalQueue);
	}

	void RenderBeast::destroy()
	{
		Renderer::destroy();

		gCoreThread().queueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreThread().submit(true);
	}

	void RenderBeast::initializeCore()
	{
		RendererUtility::startUp();

		mCoreOptions = bs_shared_ptr_new<RenderBeastOptions>(); 
		mScene = bs_shared_ptr_new<RendererScene>(mCoreOptions);
		mObjectRenderer = bs_new<ObjectRenderer>();

		mSkyboxMat = bs_new<SkyboxMat<false>>();
		mSkyboxSolidColorMat = bs_new<SkyboxMat<true>>();
		mFlatFramebufferToTextureMat = bs_new<FlatFramebufferToTextureMat>();

		mTiledDeferredLightingMats = bs_new<TiledDeferredLightingMaterials>();
		mTileDeferredImageBasedLightingMats = bs_new<TiledDeferredImageBasedLightingMaterials>();

		mPreintegratedEnvBRDF = TiledDeferredImageBasedLighting::generatePreintegratedEnvBRDF();
		mVisibleLightInfo = bs_new<VisibleLightData>();
		mVisibleReflProbeInfo = bs_new<VisibleReflProbeData>();
		mLightGrid = bs_new<LightGrid>();

		GpuResourcePool::startUp();
		PostProcessing::startUp();
		ShadowRendering::startUp(mCoreOptions->shadowMapSize);
		StandardDeferred::startUp();
	}

	void RenderBeast::destroyCore()
	{
		if (mObjectRenderer != nullptr)
			bs_delete(mObjectRenderer);

		mScene = nullptr;

		mReflCubemapArrayTex = nullptr;
		mSkyboxTexture = nullptr;
		mSkyboxFilteredReflections = nullptr;
		mSkyboxIrradiance = nullptr;

		StandardDeferred::shutDown();
		ShadowRendering::shutDown();
		PostProcessing::shutDown();
		GpuResourcePool::shutDown();

		bs_delete(mSkyboxMat);
		bs_delete(mSkyboxSolidColorMat);
		bs_delete(mVisibleLightInfo);
		bs_delete(mVisibleReflProbeInfo);
		bs_delete(mLightGrid);
		bs_delete(mFlatFramebufferToTextureMat);
		bs_delete(mTiledDeferredLightingMats);
		bs_delete(mTileDeferredImageBasedLightingMats);

		mPreintegratedEnvBRDF = nullptr;

		RendererUtility::shutDown();
	}

	void RenderBeast::notifyRenderableAdded(Renderable* renderable)
	{
		mScene->registerRenderable(renderable);

		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		RendererObject* rendererObject = sceneInfo.renderables[renderable->getRendererId()];

		for(auto& entry : rendererObject->elements)
			mObjectRenderer->initElement(*rendererObject, entry);
	}

	void RenderBeast::notifyRenderableRemoved(Renderable* renderable)
	{
		mScene->unregisterRenderable(renderable);
	}

	void RenderBeast::notifyRenderableUpdated(Renderable* renderable)
	{
		mScene->updateRenderable(renderable);
	}

	void RenderBeast::notifyLightAdded(Light* light)
	{
		mScene->registerLight(light);
	}

	void RenderBeast::notifyLightUpdated(Light* light)
	{
		mScene->updateLight(light);
	}

	void RenderBeast::notifyLightRemoved(Light* light)
	{
		mScene->unregisterLight(light);
	}

	void RenderBeast::notifyCameraAdded(Camera* camera)
	{
		mScene->registerCamera(camera);
	}

	void RenderBeast::notifyCameraUpdated(Camera* camera, UINT32 updateFlag)
	{
		mScene->updateCamera(camera, updateFlag);
	}

	void RenderBeast::notifyCameraRemoved(Camera* camera)
	{
		mScene->unregisterCamera(camera);
	}

	void RenderBeast::notifyReflectionProbeAdded(ReflectionProbe* probe)
	{
		mScene->registerReflectionProbe(probe);

		// Find a spot in cubemap array
		const SceneInfo& sceneInfo = mScene->getSceneInfo();

		UINT32 probeId = probe->getRendererId();
		const RendererReflectionProbe* probeInfo = &sceneInfo.reflProbes[probeId];

		UINT32 numArrayEntries = (UINT32)mCubemapArrayUsedSlots.size();
		for(UINT32 i = 0; i < numArrayEntries; i++)
		{
			if(!mCubemapArrayUsedSlots[i])
			{
				mScene->setReflectionProbeArrayIndex(probeId, i, false);
				mCubemapArrayUsedSlots[i] = true;
				break;
			}
		}

		// No empty slot was found
		if (probeInfo->arrayIdx == -1)
		{
			mScene->setReflectionProbeArrayIndex(probeId, numArrayEntries, false);
			mCubemapArrayUsedSlots.push_back(true);
		}

		if(probeInfo->arrayIdx > MaxReflectionCubemaps)
		{
			LOGERR("Reached the maximum number of allowed reflection probe cubemaps at once. "
				"Ignoring reflection probe data.");
		}
	}

	void RenderBeast::notifyReflectionProbeUpdated(ReflectionProbe* probe)
	{
		mScene->updateReflectionProbe(probe);
	}

	void RenderBeast::notifyReflectionProbeRemoved(ReflectionProbe* probe)
	{
		const SceneInfo& sceneInfo = mScene->getSceneInfo();

		UINT32 probeId = probe->getRendererId();
		UINT32 arrayIdx = sceneInfo.reflProbes[probeId].arrayIdx;

		if (arrayIdx != -1)
			mCubemapArrayUsedSlots[arrayIdx] = false;

		mScene->unregisterReflectionProbe(probe);
	}

	void RenderBeast::notifySkyboxAdded(Skybox* skybox)
	{
		mSkybox = skybox;

		SPtr<Texture> skyTex = skybox->getTexture();
		if (skyTex != nullptr && skyTex->getProperties().getTextureType() == TEX_TYPE_CUBE_MAP)
			mSkyboxTexture = skyTex;

		mSkyboxFilteredReflections = nullptr;
		mSkyboxIrradiance = nullptr;
	}

	void RenderBeast::notifySkyboxTextureChanged(Skybox* skybox)
	{
		LightProbeCache::instance().notifyDirty(skybox->getUUID());

		if (mSkybox == skybox)
		{
			mSkyboxTexture = skybox->getTexture();
			mSkyboxFilteredReflections = nullptr;
			mSkyboxIrradiance = nullptr;
		}
	}

	void RenderBeast::notifySkyboxRemoved(Skybox* skybox)
	{
		LightProbeCache::instance().unloadCachedTexture(skybox->getUUID());

		if (mSkybox == skybox)
			mSkyboxTexture = nullptr;
	}

	SPtr<PostProcessSettings> RenderBeast::createPostProcessSettings() const
	{
		return bs_shared_ptr_new<StandardPostProcessSettings>();
	}

	void RenderBeast::setOptions(const SPtr<RendererOptions>& options)
	{
		mOptions = std::static_pointer_cast<RenderBeastOptions>(options);
		mOptionsDirty = true;
	}

	SPtr<RendererOptions> RenderBeast::getOptions() const
	{
		return mOptions;
	}

	void RenderBeast::syncOptions(const RenderBeastOptions& options)
	{
		bool filteringChanged = mCoreOptions->filtering != options.filtering;
		if (options.filtering == RenderBeastFiltering::Anisotropic)
			filteringChanged |= mCoreOptions->anisotropyMax != options.anisotropyMax;

		if (filteringChanged)
			mScene->refreshSamplerOverrides(true);

		*mCoreOptions = options;

		mScene->setOptions(mCoreOptions);
		ShadowRendering::instance().setShadowMapSize(mCoreOptions->shadowMapSize);
	}

	void RenderBeast::renderAll() 
	{
		// Sync all dirty sim thread CoreObject data to core thread
		CoreObjectManager::instance().syncToCore();

		if (mOptionsDirty)
		{
			gCoreThread().queueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		gCoreThread().queueCommand(std::bind(&RenderBeast::renderAllCore, this, gTime().getTime(), gTime().getFrameDelta()));
	}

	void RenderBeast::renderAllCore(float time, float delta)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerGPU().beginFrame();
		gProfilerCPU().beginSample("renderAllCore");

		const SceneInfo& sceneInfo = mScene->getSceneInfo();

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in ct::Material which can only do it when sampler states
		// are actually modified after sync
		mScene->refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mObjectRenderer->setParamFrameParams(time);

		// Retrieve animation data
		AnimationManager::instance().waitUntilComplete();
		const RendererAnimationData& animData = AnimationManager::instance().getRendererData();
		
		sceneInfo.renderableReady.resize(sceneInfo.renderables.size(), false);
		sceneInfo.renderableReady.assign(sceneInfo.renderables.size(), false);
		
		FrameInfo frameInfo(delta, animData);

		// Gather all views
		Vector<RendererView*> views;
		for (auto& rtInfo : sceneInfo.renderTargets)
		{
			SPtr<RenderTarget> target = rtInfo.target;
			const Vector<Camera*>& cameras = rtInfo.cameras;

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				UINT32 viewIdx = sceneInfo.cameraToView.at(cameras[i]);
				RendererView* viewInfo = sceneInfo.views[viewIdx];
				views.push_back(viewInfo);
			}
		}

		mMainViewGroup.setViews(views.data(), (UINT32)views.size());
		mMainViewGroup.determineVisibility(sceneInfo);

		// Render shadow maps
		ShadowRendering::instance().renderShadowMaps(*mScene, mMainViewGroup, frameInfo);

		// Update reflection probes
		updateLightProbes(frameInfo);

		// Render everything
		renderViews(mMainViewGroup, frameInfo);

		gProfilerGPU().endFrame();

		// Present render targets with back buffers
		for (auto& rtInfo : sceneInfo.renderTargets)
		{
			if(rtInfo.target->getProperties().isWindow())
				RenderAPI::instance().swapBuffers(rtInfo.target);
		}

		gProfilerCPU().endSample("renderAllCore");
	}

	void RenderBeast::renderViews(const RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
	{
		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

		// Update GPU light data
		mVisibleLightInfo->update(sceneInfo, viewGroup);

		// Update reflection probe data
		mVisibleReflProbeInfo->update(sceneInfo, viewGroup);

		// Update various buffers required by each renderable
		UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			mScene->prepareRenderable(i, frameInfo);
		}

		UINT32 numViews = viewGroup.getNumViews();
		for (UINT32 i = 0; i < numViews; i++)
		{
			RendererView* view = viewGroup.getView(i);

			if (view->getProperties().isOverlay)
				renderOverlay(view);
			else
				renderView(view, frameInfo.timeDelta);
		}
	}

	void RenderBeast::renderView(RendererView* viewInfo, float frameDelta)
	{
		gProfilerCPU().beginSample("Render");

		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		auto& viewProps = viewInfo->getProperties();
		const Camera* sceneCamera = viewInfo->getSceneCamera();

		SPtr<GpuParamBlockBuffer> perCameraBuffer = viewInfo->getPerViewBuffer();
		perCameraBuffer->flushToGPU();

		Matrix4 viewProj = viewProps.viewProjTransform;
		UINT32 numSamples = viewProps.numSamples;

		bool allowShadows = !viewProps.noShadows;
		if(allowShadows)
		{
			if(sceneCamera == nullptr)
			{
				// Note: In order to support shadows on non-scene views I'd need to be aware of what those views are before
				// rendering, in order to properly generate shadow maps. 
				LOGWRN("Shadows are currently not supported on non-scene views. Disabling shadow rendering.");
				allowShadows = false;
			}
		}

		viewInfo->beginRendering(true);

		// Prepare light grid required for transparent object rendering
		mLightGrid->updateGrid(*viewInfo, *mVisibleLightInfo, *mVisibleReflProbeInfo, viewProps.noLighting);

		SPtr<GpuParamBlockBuffer> gridParams;
		SPtr<GpuBuffer> gridLightOffsetsAndSize, gridLightIndices;
		SPtr<GpuBuffer> gridProbeOffsetsAndSize, gridProbeIndices;
		mLightGrid->getOutputs(gridLightOffsetsAndSize, gridLightIndices, gridProbeOffsetsAndSize, gridProbeIndices, 
			gridParams);

		// Prepare image based material and its param buffer
		ITiledDeferredImageBasedLightingMat* imageBasedLightingMat =
			mTileDeferredImageBasedLightingMats->get(numSamples);

		imageBasedLightingMat->setReflectionProbes(*mVisibleReflProbeInfo, mReflCubemapArrayTex, viewProps.renderingReflections);

		float skyBrightness = 1.0f;
		if (mSkybox != nullptr)
			skyBrightness = mSkybox->getBrightness();

		imageBasedLightingMat->setSky(mSkyboxFilteredReflections, mSkyboxIrradiance, skyBrightness);

		// Assign camera and per-call data to all relevant renderables
		const VisibilityInfo& visibility = viewInfo->getVisibilityMasks();
		UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
		SPtr<GpuParamBlockBuffer> reflParamBuffer = imageBasedLightingMat->getReflectionsParamBuffer();
		SPtr<SamplerState> reflSamplerState = imageBasedLightingMat->getReflectionsSamplerState();
		for (UINT32 i = 0; i < numRenderables; i++)
		{
			if (!visibility.renderables[i])
				continue;

			RendererObject* rendererObject = sceneInfo.renderables[i];
			rendererObject->updatePerCallBuffer(viewProj);

			for (auto& element : sceneInfo.renderables[i]->elements)
			{
				if (element.perCameraBindingIdx != -1)
					element.params->setParamBlockBuffer(element.perCameraBindingIdx, perCameraBuffer, true);

				// Everything below is required only for forward rendering (ATM only used for transparent objects)
				// Note: It would be nice to be able to set this once and keep it, only updating if the buffers actually
				// change (e.g. when growing). Although technically the internal systems should be smart enough to
				// avoid updates unless objects actually changed.
				if (element.gridParamsBindingIdx != -1)
					element.params->setParamBlockBuffer(element.gridParamsBindingIdx, gridParams, true);

				element.gridLightOffsetsAndSizeParam.set(gridLightOffsetsAndSize);
				element.gridLightIndicesParam.set(gridLightIndices);
				element.lightsBufferParam.set(mVisibleLightInfo->getLightBuffer());

				// Image based lighting params
				ImageBasedLightingParams& iblParams = element.imageBasedParams;
				if (iblParams.reflProbeParamsBindingIdx != -1)
					element.params->setParamBlockBuffer(iblParams.reflProbeParamsBindingIdx, reflParamBuffer);

				element.gridProbeOffsetsAndSizeParam.set(gridProbeOffsetsAndSize);

				iblParams.reflectionProbeIndicesParam.set(gridProbeIndices);
				iblParams.reflectionProbesParam.set(mVisibleReflProbeInfo->getProbeBuffer());

				iblParams.skyReflectionsTexParam.set(mSkyboxFilteredReflections);
				iblParams.skyIrradianceTexParam.set(mSkyboxIrradiance);

				iblParams.reflectionProbeCubemapsTexParam.set(mReflCubemapArrayTex);
				iblParams.preintegratedEnvBRDFParam.set(mPreintegratedEnvBRDF);

				iblParams.reflectionProbeCubemapsSampParam.set(reflSamplerState);
				iblParams.skyReflectionsSampParam.set(reflSamplerState);
			}
		}

		SPtr<RenderTargets> renderTargets = viewInfo->getRenderTargets();
		renderTargets->allocate(RTT_GBuffer);
		renderTargets->bindGBuffer();

		// Trigger pre-base-pass callbacks
		auto iterRenderCallback = mCallbacks.begin();

		if (viewProps.triggerCallbacks)
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PreBasePass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		// Render base pass
		const Vector<RenderQueueElement>& opaqueElements = viewInfo->getOpaqueQueue()->getSortedElements();
		for (auto iter = opaqueElements.begin(); iter != opaqueElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, viewProj);
		}

		// Build HiZ buffer
		bool isMSAA = numSamples > 1;
		// TODO - Avoid generating it unless it actually gets used in some system
		if (isMSAA)
		{
			renderTargets->allocate(RTT_ResolvedDepth);
			renderTargets->generate(RTT_ResolvedDepth);
		}

		renderTargets->allocate(RTT_HiZ);
		renderTargets->generate(RTT_HiZ);

		// Trigger post-base-pass callbacks
		if (viewProps.triggerCallbacks)
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PostBasePass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(nullptr);

		// Accumulate all direct lighting into the light accumulation texture
		renderTargets->allocate(RTT_LightAccumulation);

		// Render non-shadowed lights into light accumulation texture (or buffer if MSAA)
		ITiledDeferredLightingMat* lightingMat = mTiledDeferredLightingMats->get(numSamples);
		lightingMat->setLights(*mVisibleLightInfo);
		lightingMat->execute(renderTargets, perCameraBuffer, viewProps.noLighting, !allowShadows);

		// If we're using flattened accumulation buffer for MSAA we need to copy its contents to the MSAA texture before
		// continuing
		if(isMSAA)
		{
			renderTargets->bindLightAccumulation();
			mFlatFramebufferToTextureMat->execute(renderTargets->getLightAccumulationBuffer(), 
				renderTargets->getLightAccumulation());
		}

		// Render shadowed lights into light accumulation texture, using standard deferred
		if (allowShadows)
		{
			renderTargets->allocate(RTT_LightOcclusion);

			UINT32 viewIdx = sceneCamera->getRendererId();

			for(UINT32 i = 0; i < (UINT32)LightType::Count; i++)
			{
				LightType lightType = (LightType)i;

				auto& lights = mVisibleLightInfo->getLights(lightType);
				UINT32 count = mVisibleLightInfo->getNumShadowedLights(lightType);
				UINT32 offset = mVisibleLightInfo->getNumUnshadowedLights(lightType);

				for (UINT32 j = 0; j < count; j++)
				{
					renderTargets->bindLightOcclusion();

					UINT32 lightIdx = offset + j;
					const RendererLight& light = *lights[lightIdx];
					ShadowRendering::instance().renderShadowOcclusion(*mScene, mCoreOptions->shadowFilteringQuality,
						light, viewIdx);

					renderTargets->bindLightAccumulation();
					StandardDeferred::instance().renderLight(lightType, light, *viewInfo, *renderTargets);
				}
			}

			renderTargets->release(RTT_LightOcclusion);
		}

		// Make sure light accumulation buffer isn't bound and can be read from
		rapi.setRenderTarget(nullptr);

		// Render image based lighting, add it to light accumulation and output scene color
		renderTargets->allocate(RTT_SceneColor);
		imageBasedLightingMat->execute(renderTargets, perCameraBuffer, mPreintegratedEnvBRDF);

		renderTargets->release(RTT_LightAccumulation);
		renderTargets->release(RTT_GBuffer);

		renderTargets->bindSceneColor(true);

		// If we're using flattened framebuffer for MSAA we need to copy its contents to the MSAA scene texture before
		// continuing
		if(isMSAA)
			mFlatFramebufferToTextureMat->execute(renderTargets->getSceneColorBuffer(), renderTargets->getSceneColor());

		// Render skybox (if any)
		if (mSkyboxTexture != nullptr)
		{
			mSkyboxMat->bind(perCameraBuffer);
			mSkyboxMat->setParams(mSkyboxTexture, Color::White);
		}
		else
		{
			Color clearColor = viewProps.clearColor;

			mSkyboxSolidColorMat->bind(perCameraBuffer);
			mSkyboxSolidColorMat->setParams(nullptr, clearColor);
		}

		SPtr<Mesh> mesh = gRendererUtility().getSkyBoxMesh();
		gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));

		renderTargets->bindSceneColor(false);

		// Render transparent objects
		// TODO: Transparent objects cannot receive shadows. In order to support this I'd have to render the light occlusion
		// for all lights affecting this object into a single (or a few) textures. I can likely use texture arrays for this,
		// or to avoid sampling many textures, perhaps just jam it all in one or few texture channels. 
		const Vector<RenderQueueElement>& transparentElements = viewInfo->getTransparentQueue()->getSortedElements();
		for (auto iter = transparentElements.begin(); iter != transparentElements.end(); ++iter)
		{
			BeastRenderableElement* renderElem = static_cast<BeastRenderableElement*>(iter->renderElem);
			renderElement(*renderElem, iter->passIdx, iter->applyPass, viewProj);
		}

		// Trigger post-light-pass callbacks
		if (viewProps.triggerCallbacks)
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::PostLightPass)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		// Post-processing and final resolve
		Rect2 viewportArea = viewProps.nrmViewRect;

		if (viewProps.runPostProcessing)
		{
			// Post-processing code also takes care of writting to the final output target
			PostProcessing::instance().postProcess(viewInfo, renderTargets, frameDelta);
		}
		else
		{
			// Just copy from scene color to output if no post-processing
			SPtr<RenderTarget> target = viewProps.target;

			rapi.setRenderTarget(target);
			rapi.setViewport(viewportArea);

			SPtr<Texture> sceneColor = renderTargets->getSceneColor();
			gRendererUtility().blit(sceneColor, Rect2I::EMPTY, viewProps.flipView);
		}

		renderTargets->release(RTT_HiZ);
		renderTargets->release(RTT_SceneColor);

		if (isMSAA)
			renderTargets->release(RTT_ResolvedDepth);

		// Trigger overlay callbacks
		if (viewProps.triggerCallbacks)
		{
			while (iterRenderCallback != mCallbacks.end())
			{
				RendererExtension* extension = *iterRenderCallback;
				if (extension->getLocation() != RenderLocation::Overlay)
					break;

				if (extension->check(*sceneCamera))
					extension->render(*sceneCamera);

				++iterRenderCallback;
			}
		}

		viewInfo->endRendering();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(RendererView* viewInfo)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		viewInfo->getPerViewBuffer()->flushToGPU();
		viewInfo->beginRendering(false);

		auto& viewProps = viewInfo->getProperties();
		const Camera* camera = viewInfo->getSceneCamera();
		SPtr<RenderTarget> target = viewProps.target;
		SPtr<Viewport> viewport = camera->getViewport();

		UINT32 clearBuffers = 0;
		if (viewport->getRequiresColorClear())
			clearBuffers |= FBT_COLOR;

		if (viewport->getRequiresDepthClear())
			clearBuffers |= FBT_DEPTH;

		if (viewport->getRequiresStencilClear())
			clearBuffers |= FBT_STENCIL;

		if (clearBuffers != 0)
		{
			RenderAPI::instance().setRenderTarget(target);
			RenderAPI::instance().clearViewport(clearBuffers, viewport->getClearColor(),
				viewport->getClearDepthValue(), viewport->getClearStencilValue());
		}
		else
			RenderAPI::instance().setRenderTarget(target, 0, RT_COLOR0);

		RenderAPI::instance().setViewport(viewport->getNormArea());

		// Trigger overlay callbacks
		auto iterRenderCallback = mCallbacks.begin();
		while (iterRenderCallback != mCallbacks.end())
		{
			RendererExtension* extension = *iterRenderCallback;
			if (extension->getLocation() != RenderLocation::Overlay)
			{
				++iterRenderCallback;
				continue;
			}

			if (extension->check(*camera))
				extension->render(*camera);

			++iterRenderCallback;
		}

		viewInfo->endRendering();

		gProfilerCPU().endSample("RenderOverlay");
	}
	
	void RenderBeast::renderElement(const BeastRenderableElement& element, UINT32 passIdx, bool bindPass, 
									const Matrix4& viewProj)
	{
		SPtr<Material> material = element.material;

		if (bindPass)
			gRendererUtility().setPass(material, passIdx, element.techniqueIdx);

		gRendererUtility().setPassParams(element.params, passIdx);

		if(element.morphVertexDeclaration == nullptr)
			gRendererUtility().draw(element.mesh, element.subMesh);
		else
			gRendererUtility().drawMorph(element.mesh, element.subMesh, element.morphShapeBuffer, 
				element.morphVertexDeclaration);
	}

	void RenderBeast::updateLightProbes(const FrameInfo& frameInfo)
	{
		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();

		bs_frame_mark();
		{		
			UINT32 currentCubeArraySize = 0;

			if(mReflCubemapArrayTex != nullptr)
				mReflCubemapArrayTex->getProperties().getNumArraySlices();

			bool forceArrayUpdate = false;
			if(mReflCubemapArrayTex == nullptr || (currentCubeArraySize < numProbes && currentCubeArraySize != MaxReflectionCubemaps))
			{
				TEXTURE_DESC cubeMapDesc;
				cubeMapDesc.type = TEX_TYPE_CUBE_MAP;
				cubeMapDesc.format = PF_FLOAT_R11G11B10;
				cubeMapDesc.width = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.numMips = PixelUtil::getMaxMipmaps(cubeMapDesc.width, cubeMapDesc.height, 1, cubeMapDesc.format);
				cubeMapDesc.numArraySlices = std::min(MaxReflectionCubemaps, numProbes + 4); // Keep a few empty entries

				mReflCubemapArrayTex = Texture::create(cubeMapDesc);

				forceArrayUpdate = true;
			}

			auto& cubemapArrayProps = mReflCubemapArrayTex->getProperties();

			TEXTURE_DESC cubemapDesc;
			cubemapDesc.type = TEX_TYPE_CUBE_MAP;
			cubemapDesc.format = PF_FLOAT_R11G11B10;
			cubemapDesc.width = IBLUtility::REFLECTION_CUBEMAP_SIZE;
			cubemapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
			cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
			cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

			SPtr<Texture> scratchCubemap;
			if (numProbes > 0)
				scratchCubemap = Texture::create(cubemapDesc);

			FrameQueue<UINT32> emptySlots;
			for (UINT32 i = 0; i < numProbes; i++)
			{
				const RendererReflectionProbe& probeInfo = sceneInfo.reflProbes[i];

				if (probeInfo.arrayIdx > MaxReflectionCubemaps)
					continue;

				SPtr<Texture> texture = probeInfo.texture;
				if (texture == nullptr)
					texture = LightProbeCache::instance().getCachedRadianceTexture(probeInfo.probe->getUUID());

				if (texture == nullptr || probeInfo.textureDirty)
				{
					texture = Texture::create(cubemapDesc);

					if (!probeInfo.customTexture)
						captureSceneCubeMap(texture, probeInfo.probe->getPosition(), true, frameInfo);
					else
					{
						SPtr<Texture> customTexture = probeInfo.probe->getCustomTexture();
						IBLUtility::scaleCubemap(customTexture, 0, texture, 0);
					}

					IBLUtility::filterCubemapForSpecular(texture, scratchCubemap);
					LightProbeCache::instance().setCachedRadianceTexture(probeInfo.probe->getUUID(), texture);
				}

				mScene->setReflectionProbeTexture(i, texture);

				if(probeInfo.arrayDirty || forceArrayUpdate)
				{
					auto& srcProps = probeInfo.texture->getProperties();
					bool isValid = srcProps.getWidth() == IBLUtility::REFLECTION_CUBEMAP_SIZE && 
						srcProps.getHeight() == IBLUtility::REFLECTION_CUBEMAP_SIZE &&
						srcProps.getNumMipmaps() == cubemapArrayProps.getNumMipmaps() &&
						srcProps.getTextureType() == TEX_TYPE_CUBE_MAP;

					if(!isValid)
					{
						if (!probeInfo.errorFlagged)
						{
							String errMsg = StringUtil::format("Cubemap texture invalid to use as a reflection cubemap. " 
								"Check texture size (must be {0}x{0}) and mip-map count", 
								IBLUtility::REFLECTION_CUBEMAP_SIZE);

							LOGERR(errMsg);
							probeInfo.errorFlagged = true;
						}
					}
					else
					{
						for(UINT32 face = 0; face < 6; face++)
							for(UINT32 mip = 0; mip <= srcProps.getNumMipmaps(); mip++)
								probeInfo.texture->copy(mReflCubemapArrayTex, face, mip, probeInfo.arrayIdx * 6 + face, mip);
					}

					mScene->setReflectionProbeArrayIndex(i, probeInfo.arrayIdx, true);
				}

				// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
			}

			// Get skybox image-based lighting textures if needed/available
			if (mSkybox != nullptr && mSkyboxTexture != nullptr)
			{
				// If haven't assigned them already, do it now
				if (mSkyboxFilteredReflections == nullptr)
				{
					if (!LightProbeCache::instance().isRadianceDirty(mSkybox->getUUID()))
						mSkyboxFilteredReflections = LightProbeCache::instance().getCachedRadianceTexture(mSkybox->getUUID());
					else
					{
						mSkyboxFilteredReflections = Texture::create(cubemapDesc);

						IBLUtility::scaleCubemap(mSkyboxTexture, 0, mSkyboxFilteredReflections, 0);
						IBLUtility::filterCubemapForSpecular(mSkyboxFilteredReflections, scratchCubemap);
						LightProbeCache::instance().setCachedRadianceTexture(mSkybox->getUUID(), mSkyboxFilteredReflections);
					}
				}

				if(mSkyboxIrradiance == nullptr)
				{
					if (!LightProbeCache::instance().isIrradianceDirty(mSkybox->getUUID()))
						mSkyboxIrradiance = LightProbeCache::instance().getCachedIrradianceTexture(mSkybox->getUUID());
					else
					{
						TEXTURE_DESC irradianceCubemapDesc;
						irradianceCubemapDesc.type = TEX_TYPE_CUBE_MAP;
						irradianceCubemapDesc.format = PF_FLOAT_R11G11B10;
						irradianceCubemapDesc.width = IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
						irradianceCubemapDesc.height = IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
						irradianceCubemapDesc.numMips = 0;
						irradianceCubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

						mSkyboxIrradiance = Texture::create(irradianceCubemapDesc);

						IBLUtility::filterCubemapForIrradiance(mSkyboxFilteredReflections, mSkyboxIrradiance);
						LightProbeCache::instance().setCachedIrradianceTexture(mSkybox->getUUID(), mSkyboxFilteredReflections);
					}
				}
			}
			else
			{
				mSkyboxFilteredReflections = nullptr;
				mSkyboxIrradiance = nullptr;
			}

		}
		bs_frame_clear();
	}

	void RenderBeast::captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, bool hdr, const FrameInfo& frameInfo)
	{
		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		auto& texProps = cubemap->getProperties();

		Matrix4 projTransform = Matrix4::projectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
		ConvexVolume localFrustum(projTransform);
		RenderAPI::instance().convertProjectionMatrix(projTransform, projTransform);

		RENDERER_VIEW_DESC viewDesc;
		viewDesc.target.clearFlags = FBT_COLOR | FBT_DEPTH;
		viewDesc.target.clearColor = Color::Black;
		viewDesc.target.clearDepthValue = 1.0f;
		viewDesc.target.clearStencilValue = 0;

		viewDesc.target.nrmViewRect = Rect2(0, 0, 1.0f, 1.0f);
		viewDesc.target.viewRect = Rect2I(0, 0, texProps.getWidth(), texProps.getHeight());
		viewDesc.target.targetWidth = texProps.getWidth();
		viewDesc.target.targetHeight = texProps.getHeight();
		viewDesc.target.numSamples = 1;

		viewDesc.isOverlay = false;
		viewDesc.isHDR = hdr;
		viewDesc.noLighting = false;
		viewDesc.noShadows = true; // Note: If I ever change this I need to make sure that shadow map rendering is aware of this view (currently it is only aware of main camera views)
		viewDesc.triggerCallbacks = false;
		viewDesc.runPostProcessing = false;
		viewDesc.renderingReflections = true;

		viewDesc.visibleLayers = 0xFFFFFFFFFFFFFFFF;
		viewDesc.nearPlane = 0.5f;
		viewDesc.farPlane = 1000.0f;
		viewDesc.flipView = RenderAPI::instance().getAPIInfo().isFlagSet(RenderAPIFeatureFlag::UVYAxisUp);

		viewDesc.viewOrigin = position;
		viewDesc.projTransform = projTransform;
		viewDesc.projType = PT_PERSPECTIVE;

		viewDesc.stateReduction = mCoreOptions->stateReductionMode;
		viewDesc.sceneCamera = nullptr;

		Matrix4 viewOffsetMat = Matrix4::translation(-position);

		RendererView views[6];
		for(UINT32 i = 0; i < 6; i++)
		{
			// Calculate view matrix
			Matrix3 viewRotationMat;
			Vector3 forward;

			Vector3 up = Vector3::UNIT_Y;

			switch (i)
			{
			case CF_PositiveX:
				forward = Vector3::UNIT_X;
				break;
			case CF_NegativeX:
				forward = -Vector3::UNIT_X;
				break;
			case CF_PositiveY:
				forward = Vector3::UNIT_Y;
				up = -Vector3::UNIT_Z;
				break;
			case CF_NegativeY:
				forward = Vector3::UNIT_X; // TODO: Why X here?
				up = Vector3::UNIT_Z;
				break;
			case CF_PositiveZ:
				forward = Vector3::UNIT_Z;
				break;
			case CF_NegativeZ:
				forward = -Vector3::UNIT_Z;
				break;
			}

			Vector3 right = Vector3::cross(up, forward);
			viewRotationMat = Matrix3(right, up, forward); // TODO - Use -forward here? (Works for shadows)

			viewDesc.viewDirection = forward;
			viewDesc.viewTransform = Matrix4(viewRotationMat) * viewOffsetMat;

			// Calculate world frustum for culling
			const Vector<Plane>& frustumPlanes = localFrustum.getPlanes();
			Matrix4 worldMatrix = viewDesc.viewTransform.transpose();

			Vector<Plane> worldPlanes(frustumPlanes.size());
			UINT32 j = 0;
			for (auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.multiplyAffine(plane);
				j++;
			}

			viewDesc.cullFrustum = ConvexVolume(worldPlanes);

			// Set up face render target
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = cubemap;
			cubeFaceRTDesc.colorSurfaces[0].face = i;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			
			viewDesc.target.target = RenderTexture::create(cubeFaceRTDesc);

			views[i].setView(viewDesc);
			views[i].updatePerViewBuffer();

			views[i].determineVisible(sceneInfo.renderables, sceneInfo.renderableCullInfos);
		}

		RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

		RendererViewGroup viewGroup(viewPtrs, 6);
		viewGroup.determineVisibility(sceneInfo);

		renderViews(viewGroup, frameInfo);
	}

}}