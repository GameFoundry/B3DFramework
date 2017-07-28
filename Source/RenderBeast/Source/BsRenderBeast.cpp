//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeast.h"
#include "BsCCamera.h"
#include "BsCRenderable.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsCoreApplication.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
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
#include "BsRenderCompositor.h"
#include "BsMesh.h"
#include "BsRendererTextures.h"

using namespace std::placeholders;

namespace bs { namespace ct
{
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
		GpuResourcePool::startUp();
		RendererTextures::startUp();

		mCoreOptions = bs_shared_ptr_new<RenderBeastOptions>(); 
		mScene = bs_shared_ptr_new<RendererScene>(mCoreOptions);
		mObjectRenderer = bs_new<ObjectRenderer>();

		mMainViewGroup = bs_new<RendererViewGroup>();

		StandardDeferred::startUp();

		RenderCompositor::registerNodeType<RCNodeSceneDepth>();
		RenderCompositor::registerNodeType<RCNodeGBuffer>();
		RenderCompositor::registerNodeType<RCNodeLightAccumulation>();
		RenderCompositor::registerNodeType<RCNodeSceneColor>();
		RenderCompositor::registerNodeType<RCNodeStandardDeferredLighting>();
		RenderCompositor::registerNodeType<RCNodeTiledDeferredLighting>();
		RenderCompositor::registerNodeType<RCNodeTiledDeferredIBL>();
		RenderCompositor::registerNodeType<RCNodeUnflattenLightAccum>();
		RenderCompositor::registerNodeType<RCNodeFinalResolve>();
		RenderCompositor::registerNodeType<RCNodeSkybox>();
		RenderCompositor::registerNodeType<RCNodeUnflattenSceneColor>();
		RenderCompositor::registerNodeType<RCNodePostProcess>();
		RenderCompositor::registerNodeType<RCNodeTonemapping>();
		RenderCompositor::registerNodeType<RCNodeGaussianDOF>();
		RenderCompositor::registerNodeType<RCNodeFXAA>();
		RenderCompositor::registerNodeType<RCNodeResolvedSceneDepth>();
		RenderCompositor::registerNodeType<RCNodeHiZ>();
		RenderCompositor::registerNodeType<RCNodeSSAO>();
		RenderCompositor::registerNodeType<RCNodeClusteredForward>();
	}

	void RenderBeast::destroyCore()
	{
		if (mObjectRenderer != nullptr)
			bs_delete(mObjectRenderer);

		mScene = nullptr;

		RenderCompositor::cleanUp();

		StandardDeferred::shutDown();

		bs_delete(mMainViewGroup);

		RendererTextures::shutDown();
		GpuResourcePool::shutDown();
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
	}

	void RenderBeast::notifyReflectionProbeUpdated(ReflectionProbe* probe)
	{
		mScene->updateReflectionProbe(probe);
	}

	void RenderBeast::notifyReflectionProbeRemoved(ReflectionProbe* probe)
	{
		mScene->unregisterReflectionProbe(probe);
	}

	void RenderBeast::notifyLightProbeVolumeAdded(LightProbeVolume* volume)
	{
		assert(false); // TODO
	}

	void RenderBeast::notifyLightProbeVolumeUpdated(LightProbeVolume* volume)
	{
		assert(false); // TODO
	}

	void RenderBeast::notifyLightProbeVolumeRemoved(LightProbeVolume* volume)
	{
		assert(false); // TODO
	}

	void RenderBeast::notifySkyboxAdded(Skybox* skybox)
	{
		mScene->registerSkybox(skybox);
	}

	void RenderBeast::notifySkyboxTextureChanged(Skybox* skybox)
	{
		mScene->updateSkybox(skybox);
	}

	void RenderBeast::notifySkyboxRemoved(Skybox* skybox)
	{
		mScene->unregisterSkybox(skybox);
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

		ShadowRendering& shadowRenderer = mMainViewGroup->getShadowRenderer();
		shadowRenderer.setShadowMapSize(mCoreOptions->shadowMapSize);
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

		mMainViewGroup->setViews(views.data(), (UINT32)views.size());
		mMainViewGroup->determineVisibility(sceneInfo);

		// Render shadow maps
		ShadowRendering& shadowRenderer = mMainViewGroup->getShadowRenderer();
		shadowRenderer.renderShadowMaps(*mScene, *mMainViewGroup, frameInfo);

		// Update reflection probe and skybox textures, if required
		renderReflectionProbes(frameInfo);
		updateSkybox();

		// Render everything
		renderViews(*mMainViewGroup, frameInfo);

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
				renderView(viewGroup, view, frameInfo);
		}
	}

	void RenderBeast::renderView(const RendererViewGroup& viewGroup, RendererView* viewInfo, const FrameInfo& frameInfo)
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

		viewInfo->beginFrame();

		const VisibleLightData& visibleLightData = viewGroup.getVisibleLightData();
		const VisibleReflProbeData& visibleReflProbeData = viewGroup.getVisibleReflProbeData();

		const LightGrid& lightGrid = viewInfo->getLightGrid();

		SPtr<GpuParamBlockBuffer> gridParams;
		SPtr<GpuBuffer> gridLightOffsetsAndSize, gridLightIndices;
		SPtr<GpuBuffer> gridProbeOffsetsAndSize, gridProbeIndices;
		lightGrid.getOutputs(gridLightOffsetsAndSize, gridLightIndices, gridProbeOffsetsAndSize, gridProbeIndices, 
			gridParams);

		// Prepare refl. probe param buffer
		ReflProbeParamBuffer reflProbeParamBuffer;
		reflProbeParamBuffer.populate(sceneInfo.sky, visibleReflProbeData, sceneInfo.reflProbeCubemapsTex, 
			viewProps.renderingReflections);

		// Assign camera and per-call data to all relevant renderables
		const VisibilityInfo& visibility = viewInfo->getVisibilityMasks();
		UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
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
				element.lightsBufferParam.set(visibleLightData.getLightBuffer());

				// Image based lighting params
				ImageBasedLightingParams& iblParams = element.imageBasedParams;
				if (iblParams.reflProbeParamsBindingIdx != -1)
					element.params->setParamBlockBuffer(iblParams.reflProbeParamsBindingIdx, reflProbeParamBuffer.buffer);

				element.gridProbeOffsetsAndSizeParam.set(gridProbeOffsetsAndSize);

				iblParams.reflectionProbeIndicesParam.set(gridProbeIndices);
				iblParams.reflectionProbesParam.set(visibleReflProbeData.getProbeBuffer());

				iblParams.skyReflectionsTexParam.set(sceneInfo.sky.filteredReflections);
				iblParams.skyIrradianceTexParam.set(sceneInfo.sky.irradiance);

				iblParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
				iblParams.preintegratedEnvBRDFParam.set(RendererTextures::preintegratedEnvGF);
			}
		}

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

		const RenderCompositor& compositor = viewInfo->getCompositor();
		compositor.execute(viewGroup, *viewInfo, sceneInfo, frameInfo, *mCoreOptions);
		viewInfo->getPPInfo().settingDirty = false;

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

		viewInfo->endFrame();

		gProfilerCPU().endSample("Render");
	}

	void RenderBeast::renderOverlay(RendererView* viewInfo)
	{
		gProfilerCPU().beginSample("RenderOverlay");

		viewInfo->getPerViewBuffer()->flushToGPU();
		viewInfo->beginFrame();

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

		viewInfo->endFrame();

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

	void RenderBeast::renderReflectionProbes(const FrameInfo& frameInfo)
	{
		SceneInfo& sceneInfo = mScene->_getSceneInfo();
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.size();

		bs_frame_mark();
		{		
			UINT32 currentCubeArraySize = 0;

			if(sceneInfo.reflProbeCubemapsTex != nullptr)
				currentCubeArraySize = sceneInfo.reflProbeCubemapsTex->getProperties().getNumArraySlices();

			bool forceArrayUpdate = false;
			if(sceneInfo.reflProbeCubemapsTex == nullptr || (currentCubeArraySize < numProbes && currentCubeArraySize != MaxReflectionCubemaps))
			{
				TEXTURE_DESC cubeMapDesc;
				cubeMapDesc.type = TEX_TYPE_CUBE_MAP;
				cubeMapDesc.format = PF_FLOAT_R11G11B10;
				cubeMapDesc.width = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.numMips = PixelUtil::getMaxMipmaps(cubeMapDesc.width, cubeMapDesc.height, 1, cubeMapDesc.format);
				cubeMapDesc.numArraySlices = std::min(MaxReflectionCubemaps, numProbes + 4); // Keep a few empty entries

				sceneInfo.reflProbeCubemapsTex = Texture::create(cubeMapDesc);

				forceArrayUpdate = true;
			}

			auto& cubemapArrayProps = sceneInfo.reflProbeCubemapsTex->getProperties();

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
								probeInfo.texture->copy(sceneInfo.reflProbeCubemapsTex, face, mip, probeInfo.arrayIdx * 6 + face, mip);
					}

					mScene->setReflectionProbeArrayIndex(i, probeInfo.arrayIdx, true);
				}

				// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
			}
		}
		bs_frame_clear();
	}

	void RenderBeast::updateSkybox()
	{
		SkyInfo& sky = mScene->_getSceneInfo().sky;

		// Get skybox image-based lighting textures if needed/available
		if (sky.skybox != nullptr && sky.radiance != nullptr)
		{
			// If haven't assigned them already, do it now
			if (sky.filteredReflections == nullptr)
			{
				if (!LightProbeCache::instance().isRadianceDirty(sky.skybox->getUUID()))
					sky.filteredReflections = LightProbeCache::instance().getCachedRadianceTexture(sky.skybox->getUUID());
				else
				{
					TEXTURE_DESC cubemapDesc;
					cubemapDesc.type = TEX_TYPE_CUBE_MAP;
					cubemapDesc.format = PF_FLOAT_R11G11B10;
					cubemapDesc.width = IBLUtility::REFLECTION_CUBEMAP_SIZE;
					cubemapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
					cubemapDesc.numMips = PixelUtil::getMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
					cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

					sky.filteredReflections = Texture::create(cubemapDesc);

					IBLUtility::scaleCubemap(sky.radiance, 0, sky.filteredReflections, 0);
					IBLUtility::filterCubemapForSpecular(sky.filteredReflections, nullptr);
					LightProbeCache::instance().setCachedRadianceTexture(sky.skybox->getUUID(), sky.filteredReflections);
				}
			}

			if(sky.irradiance == nullptr)
			{
				if (!LightProbeCache::instance().isIrradianceDirty(sky.skybox->getUUID()))
					sky.irradiance = LightProbeCache::instance().getCachedIrradianceTexture(sky.skybox->getUUID());
				else
				{
					TEXTURE_DESC irradianceCubemapDesc;
					irradianceCubemapDesc.type = TEX_TYPE_CUBE_MAP;
					irradianceCubemapDesc.format = PF_FLOAT_R11G11B10;
					irradianceCubemapDesc.width = IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
					irradianceCubemapDesc.height = IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
					irradianceCubemapDesc.numMips = 0;
					irradianceCubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

					sky.irradiance = Texture::create(irradianceCubemapDesc);

					IBLUtility::filterCubemapForIrradiance(sky.filteredReflections, sky.irradiance);
					LightProbeCache::instance().setCachedIrradianceTexture(sky.skybox->getUUID(), sky.filteredReflections);
				}
			}
		}
		else
		{
			sky.filteredReflections = nullptr;
			sky.irradiance = nullptr;
		}
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
		}

		RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

		RendererViewGroup viewGroup(viewPtrs, 6, mCoreOptions->shadowMapSize);
		viewGroup.determineVisibility(sceneInfo);

		renderViews(viewGroup, frameInfo);
	}

}}