//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeast.h"
#include "BsCoreApplication.h"
#include "CoreThread/BsCoreThread.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"
#include "Profiling/BsProfilerCPU.h"
#include "Profiling/BsProfilerGPU.h"
#include "Utility/BsTime.h"
#include "Animation/BsAnimationManager.h"
#include "Animation/BsSkeleton.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsRendererExtension.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRenderSettings.h"
#include "Renderer/BsIBLUtility.h"
#include "Renderer/BsSkybox.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRendererUtility.h"
#include "Utility/BsRendererTextures.h"
#include "Renderer/BsGpuResourcePool.h"
#include "Renderer/BsRendererManager.h"
#include "Shading/BsShadowRendering.h"
#include "Shading/BsStandardDeferred.h"
#include "Shading/BsTiledDeferred.h"
#include "BsRenderBeastOptions.h"
#include "BsRenderBeastIBLUtility.h"
#include "BsRenderCompositor.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Resources/BsBuiltinResources.h"

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

		LoadedRendererTextures textures;
		HTexture bokehFlare = gBuiltinResources().getTexture(BuiltinTexture::BokehFlare);
		if(bokehFlare.isLoaded(false))
			textures.bokehFlare = bokehFlare->getCore();

		gCoreThread().queueCommand([this, textures]() { initializeCore(textures); }, CTQF_InternalQueue);
	}

	void RenderBeast::destroy()
	{
		Renderer::destroy();

		gCoreThread().queueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreThread().submit(true);
	}

	void RenderBeast::initializeCore(const LoadedRendererTextures& rendererTextures)
	{
		const RenderAPICapabilities& caps = gCaps();

		if(
			!caps.hasCapability(RSC_COMPUTE_PROGRAM) ||
			!caps.hasCapability(RSC_LOAD_STORE) ||
			!caps.hasCapability(RSC_TEXTURE_VIEWS))
		{
			mFeatureSet = RenderBeastFeatureSet::DesktopMacOS;
		}

		// Ensure profiler methods can be called from start-up methods
		gProfilerGPU().beginFrame();

		RendererUtility::startUp();
		GpuSort::startUp();
		GpuResourcePool::startUp();
		IBLUtility::startUp<RenderBeastIBLUtility>();
		RendererTextures::startUp(rendererTextures);

		mCoreOptions = bs_shared_ptr_new<RenderBeastOptions>();
		mScene = bs_shared_ptr_new<RendererScene>(mCoreOptions);

		mMainViewGroup = bs_new<RendererViewGroup>(nullptr, 0, true);

		StandardDeferred::startUp();
		ParticleRenderer::startUp();
		GpuParticleSimulation::startUp();

		gProfilerGPU().endFrame(true);

		RenderCompositor::registerNodeType<RCNodeSceneDepth>();
		RenderCompositor::registerNodeType<RCNodeBasePass>();
		RenderCompositor::registerNodeType<RCNodeLightAccumulation>();
		RenderCompositor::registerNodeType<RCNodeSceneColor>();
		RenderCompositor::registerNodeType<RCNodeDeferredDirectLighting>();
		RenderCompositor::registerNodeType<RCNodeIndirectDiffuseLighting>();
		RenderCompositor::registerNodeType<RCNodeDeferredIndirectSpecularLighting>();
		RenderCompositor::registerNodeType<RCNodeFinalResolve>();
		RenderCompositor::registerNodeType<RCNodeSkybox>();
		RenderCompositor::registerNodeType<RCNodePostProcess>();
		RenderCompositor::registerNodeType<RCNodeTonemapping>();
		RenderCompositor::registerNodeType<RCNodeGaussianDOF>();
		RenderCompositor::registerNodeType<RCNodeBokehDOF>();
		RenderCompositor::registerNodeType<RCNodeFXAA>();
		RenderCompositor::registerNodeType<RCNodeResolvedSceneDepth>();
		RenderCompositor::registerNodeType<RCNodeHiZ>();
		RenderCompositor::registerNodeType<RCNodeSSAO>();
		RenderCompositor::registerNodeType<RCNodeClusteredForward>();
		RenderCompositor::registerNodeType<RCNodeSSR>();
		RenderCompositor::registerNodeType<RCNodeMSAACoverage>();
		RenderCompositor::registerNodeType<RCNodeParticleSimulate>();
		RenderCompositor::registerNodeType<RCNodeParticleSort>();
		RenderCompositor::registerNodeType<RCNodeHalfSceneColor>();
		RenderCompositor::registerNodeType<RCNodeBloom>();
		RenderCompositor::registerNodeType<RCNodeEyeAdaptation>();
		RenderCompositor::registerNodeType<RCNodeScreenSpaceLensFlare>();
		RenderCompositor::registerNodeType<RCNodeSceneColorDownsamples>();
		RenderCompositor::registerNodeType<RCNodeMotionBlur>();
		RenderCompositor::registerNodeType<RCNodeChromaticAberration>();
		RenderCompositor::registerNodeType<RCNodeFilmGrain>();
		RenderCompositor::registerNodeType<RCNodeTemporalAA>();
	}

	void RenderBeast::destroyCore()
	{
		// Make sure all tasks finish first
		processTasks(true);

		mScene = nullptr;

		RenderCompositor::cleanUp();

		GpuParticleSimulation::shutDown();
		ParticleRenderer::shutDown();
		StandardDeferred::shutDown();

		bs_delete(mMainViewGroup);

		RendererTextures::shutDown();
		IBLUtility::shutDown();
		GpuResourcePool::shutDown();
		GpuSort::shutDown();
		RendererUtility::shutDown();
	}

	void RenderBeast::notifyRenderableAdded(Renderable* renderable)
	{
		mScene->registerRenderable(renderable);
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

	void RenderBeast::notifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture)
	{
		mScene->updateReflectionProbe(probe, texture);
	}

	void RenderBeast::notifyReflectionProbeRemoved(ReflectionProbe* probe)
	{
		mScene->unregisterReflectionProbe(probe);
	}

	void RenderBeast::notifyLightProbeVolumeAdded(LightProbeVolume* volume)
	{
		mScene->registerLightProbeVolume(volume);
	}

	void RenderBeast::notifyLightProbeVolumeUpdated(LightProbeVolume* volume)
	{
		mScene->updateLightProbeVolume(volume);
	}

	void RenderBeast::notifyLightProbeVolumeRemoved(LightProbeVolume* volume)
	{
		mScene->unregisterLightProbeVolume(volume);
	}

	void RenderBeast::notifySkyboxAdded(Skybox* skybox)
	{
		mScene->registerSkybox(skybox);
	}

	void RenderBeast::notifySkyboxRemoved(Skybox* skybox)
	{
		mScene->unregisterSkybox(skybox);
	}

	void RenderBeast::notifyParticleSystemAdded(ParticleSystem* particleSystem)
	{
		mScene->registerParticleSystem(particleSystem);
	}

	void RenderBeast::notifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly)
	{
		mScene->updateParticleSystem(particleSystem, tfrmOnly);
	}

	void RenderBeast::notifyParticleSystemRemoved(ParticleSystem* particleSystem)
	{
		mScene->unregisterParticleSystem(particleSystem);
	}

	void RenderBeast::notifyDecalAdded(Decal* decal)
	{
		mScene->registerDecal(decal);
	}

	void RenderBeast::notifyDecalRemoved(Decal* decal)
	{
		mScene->unregisterDecal(decal);
	}

	void RenderBeast::notifyDecalUpdated(Decal* decal)
	{
		mScene->updateDecal(decal);
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

	ShaderExtensionPointInfo RenderBeast::getShaderExtensionPointInfo(const String& name)
	{
		if(name == "DeferredDirectLighting")
		{
			ShaderExtensionPointInfo info;
			
			ExtensionShaderInfo tiledDeferredInfo;
			tiledDeferredInfo.name = "TiledDeferredDirectLighting";
			tiledDeferredInfo.path = TiledDeferredLightingMat::getShaderPath();
			tiledDeferredInfo.defines = TiledDeferredLightingMat::getShaderDefines();
			info.shaders.push_back(tiledDeferredInfo);

			ExtensionShaderInfo standardDeferredPointInfo;
			standardDeferredPointInfo.name = "StandardDeferredPointDirectLighting";
			standardDeferredPointInfo.path = DeferredPointLightMat::getShaderPath();
			standardDeferredPointInfo.defines = DeferredPointLightMat::getShaderDefines();
			info.shaders.push_back(standardDeferredPointInfo);

			ExtensionShaderInfo standardDeferredDirInfo;
			standardDeferredDirInfo.name = "StandardDeferredDirDirectLighting";
			standardDeferredDirInfo.path = DeferredDirectionalLightMat::getShaderPath();
			standardDeferredDirInfo.defines = DeferredDirectionalLightMat::getShaderDefines();
			info.shaders.push_back(standardDeferredPointInfo);

			return info;
		}

		return ShaderExtensionPointInfo();
	}

	void RenderBeast::setGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader)
	{
		SPtr<ct::Shader> shaderCore;
		if(shader)
			shaderCore = shader->getCore();

		auto setShaderOverride = [name, shaderCore]()
		{
			if (name == "TiledDeferredDirectLighting")
				TiledDeferredLightingMat::setOverride(shaderCore);
			else if(name == "StandardDeferredPointDirectLighting")
				DeferredPointLightMat::setOverride(shaderCore);
			else if(name == "StandardDeferredDirDirectLighting")
				DeferredDirectionalLightMat::setOverride(shaderCore);
		};
	
		gCoreThread().queueCommand(setShaderOverride);
	}

	void RenderBeast::renderAll(PerFrameData perFrameData)
	{
		// Sync all dirty sim thread CoreObject data to core thread
		PROFILE_CALL(CoreObjectManager::instance().syncToCore(), "Sync to core")

		if (mOptionsDirty)
		{
			gCoreThread().queueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		FrameTimings timings;
		timings.time = gTime().getTime();
		timings.timeDelta = gTime().getFrameDelta();
		timings.frameIdx = gTime().getFrameIdx();
		
		gCoreThread().queueCommand(std::bind(&RenderBeast::renderAllCore, this, timings, perFrameData));
	}

	void RenderBeast::renderAllCore(FrameTimings timings, PerFrameData perFrameData)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerGPU().beginFrame();
		gProfilerCPU().beginSample("Render");

		const SceneInfo& sceneInfo = mScene->getSceneInfo();

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in ct::Material which can only do it when sampler states
		// are actually modified after sync
		mScene->refreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mScene->setParamFrameParams(timings.time);

		// Update bounds for all particle systems
		if(perFrameData.particles)
			PROFILE_CALL(mScene->updateParticleSystemBounds(perFrameData.particles), "Particle bounds")

		sceneInfo.renderableReady.resize(sceneInfo.renderables.size(), false);
		sceneInfo.renderableReady.assign(sceneInfo.renderables.size(), false);
		
		FrameInfo frameInfo(timings, perFrameData);

		// Make sure any renderer tasks finish first, as rendering might depend on them
		processTasks(false, timings.frameIdx);

		// If any reflection probes were updated or added, we need to copy them over in the global reflection probe array
		updateReflProbeArray();

		// Update per-frame data for all renderable objects
		for (UINT32 i = 0; i < sceneInfo.renderables.size(); i++)
			mScene->prepareRenderable(i, frameInfo);

		for (UINT32 i = 0; i < sceneInfo.particleSystems.size(); i++)
			mScene->prepareParticleSystem(i, frameInfo);

		for (UINT32 i = 0; i < sceneInfo.decals.size(); i++)
			mScene->prepareDecal(i, frameInfo);

		// Gather all views
		for (auto& rtInfo : sceneInfo.renderTargets)
		{
			Vector<RendererView*> views;
			SPtr<RenderTarget> target = rtInfo.target;
			const Vector<Camera*>& cameras = rtInfo.cameras;

			UINT32 numCameras = (UINT32)cameras.size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				UINT32 viewIdx = sceneInfo.cameraToView.at(cameras[i]);
				RendererView* viewInfo = sceneInfo.views[viewIdx];
				views.push_back(viewInfo);
			}

			mMainViewGroup->setViews(views.data(), (UINT32)views.size());
			PROFILE_CALL(mMainViewGroup->determineVisibility(sceneInfo), "Determine visibility")

			// Render everything
			bool anythingDrawn = renderViews(*mMainViewGroup, frameInfo);

			if(rtInfo.target->getProperties().isWindow && anythingDrawn)
				PROFILE_CALL(RenderAPI::instance().swapBuffers(rtInfo.target), "Swap buffers");
		}

		// Tick pool frame
		GpuResourcePool::instance().update();

		gProfilerGPU().endFrame();
		gProfilerCPU().endSample("Render");
	}

	bool RenderBeast::renderViews(RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
	{
		bool needs3DRender = false;
		UINT32 numViews = viewGroup.getNumViews();
		for (UINT32 i = 0; i < numViews; i++)
		{
			RendererView* view = viewGroup.getView(i);

			if (view->shouldDraw3D())
			{
				needs3DRender = true;
				break;
			}
		}
		
		if (needs3DRender)
		{
			const SceneInfo& sceneInfo = mScene->getSceneInfo();
			const VisibilityInfo& visibility = viewGroup.getVisibilityInfo();

			// Render shadow maps
			ShadowRendering& shadowRenderer = viewGroup.getShadowRenderer();
			shadowRenderer.renderShadowMaps(*mScene, viewGroup, frameInfo);

			// Update various buffers required by each renderable
			UINT32 numRenderables = (UINT32)sceneInfo.renderables.size();
			for (UINT32 i = 0; i < numRenderables; i++)
			{
				if (!visibility.renderables[i])
					continue;

				mScene->prepareVisibleRenderable(i, frameInfo);
			}
		}

		bool anythingDrawn = false;
		for (UINT32 i = 0; i < numViews; i++)
		{
			RendererView* view = viewGroup.getView(i);
			view->updateAsyncOperations();
			
			auto viewId = (UINT64)view;
			const RendererViewTargetData& viewTarget = view->getProperties().target;
			String title = StringUtil::format("({0} x {1})", viewTarget.targetWidth, viewTarget.targetHeight);
			gProfilerGPU().beginView(viewId, ProfilerString(title.data(), title.size()));
			
			if (!view->shouldDraw())
			{
				gProfilerGPU().endView();
				continue;
			}
			
			const RenderSettings& settings = view->getRenderSettings();
			if (settings.overlayOnly)
			{
				if (renderOverlay(*view, frameInfo))
					anythingDrawn = true;
			}
			else
			{
				renderView(viewGroup, *view, frameInfo);
				anythingDrawn = true;
			}

			gProfilerGPU().endView();
		}

		return anythingDrawn;
	}

	void RenderBeast::renderView(const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
	{
		gProfilerCPU().beginSample("Render view");

		const SceneInfo& sceneInfo = mScene->getSceneInfo();
		auto& viewProps = view.getProperties();

		SPtr<GpuParamBlockBuffer> perCameraBuffer = view.getPerViewBuffer();
		perCameraBuffer->flushToGPU();

		// Make sure light probe data is up to date
		if(view.getRenderSettings().enableIndirectLighting)
			mScene->updateLightProbes();

		view.beginFrame(frameInfo);

		RenderCompositorNodeInputs inputs(viewGroup, view, sceneInfo, *mCoreOptions, frameInfo, mFeatureSet);

		// Register callbacks
		if (viewProps.triggerCallbacks)
		{
			const Camera* camera = view.getSceneCamera();
			for (auto& extension : mCallbacks)
			{
				RenderLocation location = extension->getLocation();
				RendererExtensionRequest request = extension->check(*camera);

				if (request == RendererExtensionRequest::DontRender)
					continue;

				switch(location)
				{
				case RenderLocation::Prepare:
					inputs.extPrepare.add(extension);
					break;
				case RenderLocation::PreBasePass:
					inputs.extPreBasePass.add(extension);
					break;
				case RenderLocation::PostBasePass:
					inputs.extPostBasePass.add(extension);
					break;
				case RenderLocation::PostLightPass:
					inputs.extPostLighting.add(extension);
					break;
				case RenderLocation::Overlay:
					inputs.extOverlay.add(extension);
					break;
				default:
					break;
				}
			}
		}

		const RenderCompositor& compositor = view.getCompositor();
		PROFILE_CALL(compositor.execute(inputs), "Compositor")

		view.endFrame();

		gProfilerCPU().endSample("Render view");
	}

	bool RenderBeast::renderOverlay(RendererView& view, const FrameInfo& frameInfo)
	{
		gProfilerCPU().beginSample("Render overlay");

		view.getPerViewBuffer()->flushToGPU();
		view.beginFrame(frameInfo);

		auto& viewProps = view.getProperties();
		const Camera* camera = view.getSceneCamera();
		SPtr<RenderTarget> target = viewProps.target.target;
		SPtr<Viewport> viewport = camera->getViewport();

		ClearFlags clearFlags = viewport->getClearFlags();
		UINT32 clearBuffers = 0;
		if (clearFlags.isSet(ClearFlagBits::Color))
			clearBuffers |= FBT_COLOR;

		if (clearFlags.isSet(ClearFlagBits::Depth))
			clearBuffers |= FBT_DEPTH;

		if (clearFlags.isSet(ClearFlagBits::Stencil))
			clearBuffers |= FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::instance();
		if (clearBuffers != 0)
		{
			rapi.setRenderTarget(target);
			rapi.clearViewport(clearBuffers, viewport->getClearColorValue(),
				viewport->getClearDepthValue(), viewport->getClearStencilValue());
		}
		else
			rapi.setRenderTarget(target, 0, RT_COLOR0);

		rapi.setViewport(viewport->getArea());

		// Trigger overlay callbacks
		bool needsRedraw = false;
		if(!mCallbacks.empty())
		{
			view._notifyCompositorTargetChanged(target);

			mOverlayExtensions.clear();

			for(auto& entry : mCallbacks)
			{
				if (entry->getLocation() != RenderLocation::Overlay)
					continue;

				RendererExtensionRequest request = entry->check(*camera);
				if (request == RendererExtensionRequest::DontRender)
					continue;

				if (request == RendererExtensionRequest::ForceRender)
					needsRedraw = true;

				mOverlayExtensions.push_back(entry);
			}

			if (!needsRedraw)
				mOverlayExtensions.clear();

			for (auto& entry : mOverlayExtensions)
				entry->render(*camera, view.getContext());
		}

		view.endFrame();

		gProfilerCPU().endSample("Render overlay");
		return needsRedraw;
	}
	
	void RenderBeast::updateReflProbeArray()
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
				cubeMapDesc.format = PF_RG11B10F;
				cubeMapDesc.width = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
				cubeMapDesc.numMips = PixelUtil::getMaxMipmaps(cubeMapDesc.width, cubeMapDesc.height, 1, cubeMapDesc.format);
				cubeMapDesc.numArraySlices = std::min(MaxReflectionCubemaps, numProbes + 4); // Keep a few empty entries

				sceneInfo.reflProbeCubemapsTex = Texture::create(cubeMapDesc);

				forceArrayUpdate = true;
			}

			auto& cubemapArrayProps = sceneInfo.reflProbeCubemapsTex->getProperties();

			FrameQueue<UINT32> emptySlots;
			for (UINT32 i = 0; i < numProbes; i++)
			{
				const RendererReflectionProbe& probeInfo = sceneInfo.reflProbes[i];

				if (probeInfo.arrayIdx > MaxReflectionCubemaps)
					continue;

				if(probeInfo.arrayDirty || forceArrayUpdate)
				{
					SPtr<Texture> texture = probeInfo.probe->getFilteredTexture();
					if (texture == nullptr)
						continue;

					auto& srcProps = texture->getProperties();
					bool isValid = srcProps.getWidth() == IBLUtility::REFLECTION_CUBEMAP_SIZE &&
						srcProps.getHeight() == IBLUtility::REFLECTION_CUBEMAP_SIZE &&
						srcProps.getNumMipmaps() == cubemapArrayProps.getNumMipmaps() &&
						srcProps.getTextureType() == TEX_TYPE_CUBE_MAP;

					if(!isValid)
					{
						if (!probeInfo.errorFlagged)
						{
							BS_LOG(Error, Renderer, "Cubemap texture invalid to use as a reflection cubemap. "
								"Check texture size (must be {0}x{0}) and mip-map count",
								IBLUtility::REFLECTION_CUBEMAP_SIZE);

							probeInfo.errorFlagged = true;
						}
					}
					else
					{
						for(UINT32 face = 0; face < 6; face++)
						{
							for(UINT32 mip = 0; mip <= srcProps.getNumMipmaps(); mip++)
							{
								TEXTURE_COPY_DESC copyDesc;
								copyDesc.srcFace = face;
								copyDesc.srcMip = mip;
								copyDesc.dstFace = probeInfo.arrayIdx * 6 + face;
								copyDesc.dstMip = mip;

								texture->copy(sceneInfo.reflProbeCubemapsTex, copyDesc);
							}
						}
					}

					mScene->setReflectionProbeArrayIndex(i, probeInfo.arrayIdx, true);
				}

				// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
			}
		}
		bs_frame_clear();
	}

	void RenderBeast::captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position,
		const CaptureSettings& settings)
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

		viewDesc.mainView = false;
		viewDesc.triggerCallbacks = false;
		viewDesc.runPostProcessing = false;
		viewDesc.capturingReflections = true;
		viewDesc.onDemand = false;
		viewDesc.encodeDepth = settings.encodeDepth;
		viewDesc.depthEncodeNear = settings.depthEncodeNear;
		viewDesc.depthEncodeFar = settings.depthEncodeFar;

		viewDesc.visibleLayers = 0xFFFFFFFFFFFFFFFF;
		viewDesc.nearPlane = 0.5f;
		viewDesc.farPlane = 1000.0f;
		viewDesc.flipView = gCaps().conventions.uvYAxis != Conventions::Axis::Up;

		viewDesc.viewOrigin = position;
		viewDesc.projTransform = projTransform;
		viewDesc.projType = PT_PERSPECTIVE;

		viewDesc.stateReduction = mCoreOptions->stateReductionMode;
		viewDesc.sceneCamera = nullptr;

		SPtr<RenderSettings> renderSettings = bs_shared_ptr_new<RenderSettings>();
		renderSettings->enableHDR = settings.hdr;
		renderSettings->enableShadows = true;
		renderSettings->enableIndirectLighting = false;
		renderSettings->screenSpaceReflections.enabled = false;
		renderSettings->ambientOcclusion.enabled = false;

		Matrix4 viewOffsetMat = Matrix4::translation(-position);

		// Note: We render upside down, then flip the image vertically, which results in a horizontal flip. The horizontal
		// flip is required due to the fact how cubemap faces are defined. Another option would be to change the view
		// orientation matrix, but that also requires a culling mode flip which is inconvenient to do globally.
		RendererView views[6];
		for(UINT32 i = 0; i < 6; i++)
		{
			// Calculate view matrix
			Vector3 forward;
			Vector3 up = Vector3::UNIT_Y;

			switch (i)
			{
			case CF_PositiveX:
				forward = -Vector3::UNIT_X;
				up = -Vector3::UNIT_Y;
				break;
			case CF_NegativeX:
				forward = Vector3::UNIT_X;
				up = -Vector3::UNIT_Y;
				break;
			case CF_PositiveY:
				forward = Vector3::UNIT_Y;
				up = -Vector3::UNIT_Z;
				break;
			case CF_NegativeY:
				forward = -Vector3::UNIT_Y;
				up = Vector3::UNIT_Z;
				break;
			case CF_PositiveZ:
				forward = -Vector3::UNIT_Z;
				up = -Vector3::UNIT_Y;
				break;
			case CF_NegativeZ:
				forward = Vector3::UNIT_Z;
				up = -Vector3::UNIT_Y;
				break;
			}

			Vector3 right = Vector3::cross(up, forward);
			Matrix3 viewRotationMat = Matrix3(right, up, forward);

			viewDesc.viewDirection = -forward;
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
			views[i].setRenderSettings(renderSettings);
			views[i].updatePerViewBuffer();
		}

		RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

		RendererViewGroup viewGroup(viewPtrs, 6, false, mCoreOptions->shadowMapSize);
		viewGroup.determineVisibility(sceneInfo);

		FrameInfo frameInfo({ 0.0f, 1.0f / 60.0f, 0 }, PerFrameData());
		renderViews(viewGroup, frameInfo);

		// Make sure the render texture is available for reads
		RenderAPI::instance().setRenderTarget(nullptr);
	}

	SPtr<RenderBeast> gRenderBeast()
	{
		return std::static_pointer_cast<RenderBeast>(RendererManager::instance().getActive());
	}
}}
