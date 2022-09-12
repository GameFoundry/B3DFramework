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

	const StringID& RenderBeast::GetName() const
	{
		static StringID name = "RenderBeast";
		return name;
	}

	void RenderBeast::Initialize()
	{
		Renderer::initialize();

		LoadedRendererTextures textures;
		HTexture bokehFlare = gBuiltinResources().GetTexture(BuiltinTexture::BokehFlare);
		if(bokehFlare.IsLoaded(false))
			textures.bokehFlare = bokehFlare->GetCore();

		gCoreThread().QueueCommand([this, textures]() { initializeCore(textures); }, CTQF_InternalQueue);
	}

	void RenderBeast::Destroy()
	{
		Renderer::destroy();

		gCoreThread().QueueCommand(std::bind(&RenderBeast::destroyCore, this));
		gCoreThread().Submit(true);
	}

	void RenderBeast::InitializeCore(const LoadedRendererTextures& rendererTextures)
	{
		const RenderAPICapabilities& caps = gCaps();

		if(
			!caps.HasCapability(RSC_COMPUTE_PROGRAM) ||
			!caps.HasCapability(RSC_LOAD_STORE) ||
			!caps.HasCapability(RSC_TEXTURE_VIEWS))
		{
			mFeatureSet = RenderBeastFeatureSet::DesktopMacOS;
		}

		// Ensure profiler methods can be called from start-up methods
		gProfilerGPU().BeginFrame();

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

		gProfilerGPU().EndFrame(true);

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

	void RenderBeast::DestroyCore()
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

	void RenderBeast::NotifyRenderableAdded(Renderable* renderable)
	{
		mScene->RegisterRenderable(renderable);
	}

	void RenderBeast::NotifyRenderableRemoved(Renderable* renderable)
	{
		mScene->UnregisterRenderable(renderable);
	}

	void RenderBeast::NotifyRenderableUpdated(Renderable* renderable)
	{
		mScene->UpdateRenderable(renderable);
	}

	void RenderBeast::NotifyLightAdded(Light* light)
	{
		mScene->RegisterLight(light);
	}

	void RenderBeast::NotifyLightUpdated(Light* light)
	{
		mScene->UpdateLight(light);
	}

	void RenderBeast::NotifyLightRemoved(Light* light)
	{
		mScene->UnregisterLight(light);
	}

	void RenderBeast::NotifyCameraAdded(Camera* camera)
	{
		mScene->RegisterCamera(camera);
	}

	void RenderBeast::NotifyCameraUpdated(Camera* camera, UINT32 updateFlag)
	{
		mScene->UpdateCamera(camera, updateFlag);
	}

	void RenderBeast::NotifyCameraRemoved(Camera* camera)
	{
		mScene->UnregisterCamera(camera);
	}

	void RenderBeast::NotifyReflectionProbeAdded(ReflectionProbe* probe)
	{
		mScene->RegisterReflectionProbe(probe);
	}

	void RenderBeast::NotifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture)
	{
		mScene->UpdateReflectionProbe(probe, texture);
	}

	void RenderBeast::NotifyReflectionProbeRemoved(ReflectionProbe* probe)
	{
		mScene->UnregisterReflectionProbe(probe);
	}

	void RenderBeast::NotifyLightProbeVolumeAdded(LightProbeVolume* volume)
	{
		mScene->RegisterLightProbeVolume(volume);
	}

	void RenderBeast::NotifyLightProbeVolumeUpdated(LightProbeVolume* volume)
	{
		mScene->UpdateLightProbeVolume(volume);
	}

	void RenderBeast::NotifyLightProbeVolumeRemoved(LightProbeVolume* volume)
	{
		mScene->UnregisterLightProbeVolume(volume);
	}

	void RenderBeast::NotifySkyboxAdded(Skybox* skybox)
	{
		mScene->RegisterSkybox(skybox);
	}

	void RenderBeast::NotifySkyboxRemoved(Skybox* skybox)
	{
		mScene->UnregisterSkybox(skybox);
	}

	void RenderBeast::NotifyParticleSystemAdded(ParticleSystem* particleSystem)
	{
		mScene->RegisterParticleSystem(particleSystem);
	}

	void RenderBeast::NotifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly)
	{
		mScene->UpdateParticleSystem(particleSystem, tfrmOnly);
	}

	void RenderBeast::NotifyParticleSystemRemoved(ParticleSystem* particleSystem)
	{
		mScene->UnregisterParticleSystem(particleSystem);
	}

	void RenderBeast::NotifyDecalAdded(Decal* decal)
	{
		mScene->RegisterDecal(decal);
	}

	void RenderBeast::NotifyDecalRemoved(Decal* decal)
	{
		mScene->UnregisterDecal(decal);
	}

	void RenderBeast::NotifyDecalUpdated(Decal* decal)
	{
		mScene->UpdateDecal(decal);
	}
	void RenderBeast::SetOptions(const SPtr<RendererOptions>& options)
	{
		mOptions = std::static_pointer_cast<RenderBeastOptions>(options);
		mOptionsDirty = true;
	}

	SPtr<RendererOptions> RenderBeast::GetOptions() const
	{
		return mOptions;
	}

	void RenderBeast::SyncOptions(const RenderBeastOptions& options)
	{
		bool filteringChanged = mCoreOptions->filtering != options.filtering;
		if (options.filtering == RenderBeastFiltering::Anisotropic)
			filteringChanged |= mCoreOptions->anisotropyMax != options.anisotropyMax;

		if (filteringChanged)
			mScene->RefreshSamplerOverrides(true);

		*mCoreOptions = options;

		mScene->SetOptions(mCoreOptions);

		ShadowRendering& shadowRenderer = mMainViewGroup->GetShadowRenderer();
		shadowRenderer.SetShadowMapSize(mCoreOptions->shadowMapSize);
	}

	ShaderExtensionPointInfo RenderBeast::GetShaderExtensionPointInfo(const String& name)
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

	void RenderBeast::SetGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader)
	{
		SPtr<ct::Shader> shaderCore;
		if(shader)
			shaderCore = shader->GetCore();

		auto setShaderOverride = [name, shaderCore]()
		{
			if (name == "TiledDeferredDirectLighting")
				TiledDeferredLightingMat::setOverride(shaderCore);
			else If(name == "StandardDeferredPointDirectLighting")
				DeferredPointLightMat::setOverride(shaderCore);
			else If(name == "StandardDeferredDirDirectLighting")
				DeferredDirectionalLightMat::setOverride(shaderCore);
		};
	
		gCoreThread().QueueCommand(setShaderOverride);
	}

	void RenderBeast::RenderAll(PerFrameData perFrameData)
	{
		// Sync all dirty sim thread CoreObject data to core thread
		PROFILE_CALL(CoreObjectManager::instance().SyncToCore(), "Sync to core")

		if (mOptionsDirty)
		{
			gCoreThread().QueueCommand(std::bind(&RenderBeast::syncOptions, this, *mOptions));
			mOptionsDirty = false;
		}

		FrameTimings timings;
		timings.time = gTime().GetTime();
		timings.timeDelta = gTime().GetFrameDelta();
		timings.frameIdx = gTime().GetFrameIdx();
		
		gCoreThread().QueueCommand(std::bind(&RenderBeast::renderAllCore, this, timings, perFrameData));
	}

	void RenderBeast::RenderAllCore(FrameTimings timings, PerFrameData perFrameData)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerGPU().BeginFrame();
		gProfilerCPU().BeginSample("Render");

		const SceneInfo& sceneInfo = mScene->GetSceneInfo();

		// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
		// issue consider handling this internally in ct::Material which can only do it when sampler states
		// are actually modified after sync
		mScene->RefreshSamplerOverrides();

		// Update global per-frame hardware buffers
		mScene->SetParamFrameParams(timings.time);

		// Update bounds for all particle systems
		if(perFrameData.particles)
			PROFILE_CALL(mScene->UpdateParticleSystemBounds(perFrameData.particles), "Particle bounds")

		sceneInfo.renderableReady.Resize(sceneInfo.renderables.size(), false);
		sceneInfo.renderableReady.Assign(sceneInfo.renderables.size(), false);
		
		FrameInfo FrameInfo(timings, perFrameData);

		// Make sure any renderer tasks finish first, as rendering might depend on them
		processTasks(false, timings.frameIdx);

		// If any reflection probes were updated or added, we need to copy them over in the global reflection probe array
		updateReflProbeArray();

		// Update per-frame data for all renderable objects
		for (UINT32 i = 0; i < sceneInfo.renderables.Size(); i++)
			mScene->PrepareRenderable(i, frameInfo);

		for (UINT32 i = 0; i < sceneInfo.particleSystems.Size(); i++)
			mScene->PrepareParticleSystem(i, frameInfo);

		for (UINT32 i = 0; i < sceneInfo.decals.Size(); i++)
			mScene->PrepareDecal(i, frameInfo);

		// Gather all views
		for (auto& rtInfo : sceneInfo.renderTargets)
		{
			Vector<RendererView*> views;
			SPtr<RenderTarget> target = rtInfo.target;
			const Vector<Camera*>& cameras = rtInfo.cameras;

			UINT32 numCameras = (UINT32)cameras.Size();
			for (UINT32 i = 0; i < numCameras; i++)
			{
				UINT32 viewIdx = sceneInfo.cameraToView.At(cameras[i]);
				RendererView* viewInfo = sceneInfo.views[viewIdx];
				views.push_back(viewInfo);
			}

			mMainViewGroup->SetViews(views.Data(), (UINT32)views.size());
			PROFILE_CALL(mMainViewGroup->DetermineVisibility(sceneInfo), "Determine visibility")

			// Render everything
			bool anythingDrawn = renderViews(*mMainViewGroup, frameInfo);

			if(rtInfo.target->GetProperties().isWindow && anythingDrawn)
				PROFILE_CALL(RenderAPI::instance().SwapBuffers(rtInfo.target), "Swap buffers");
		}

		// Tick pool frame
		GpuResourcePool::instance().Update();

		gProfilerGPU().EndFrame();
		gProfilerCPU().EndSample("Render");
	}

	bool RenderBeast::RenderViews(RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
	{
		bool needs3DRender = false;
		UINT32 numViews = viewGroup.GetNumViews();
		for (UINT32 i = 0; i < numViews; i++)
		{
			RendererView* view = viewGroup.GetView(i);

			if (view->ShouldDraw3D())
			{
				needs3DRender = true;
				break;
			}
		}
		
		if (needs3DRender)
		{
			const SceneInfo& sceneInfo = mScene->GetSceneInfo();
			const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

			// Render shadow maps
			ShadowRendering& shadowRenderer = viewGroup.GetShadowRenderer();
			shadowRenderer.RenderShadowMaps(*mScene, viewGroup, frameInfo);

			// Update various buffers required by each renderable
			UINT32 numRenderables = (UINT32)sceneInfo.renderables.Size();
			for (UINT32 i = 0; i < numRenderables; i++)
			{
				if (!visibility.renderables[i])
					continue;

				mScene->PrepareVisibleRenderable(i, frameInfo);
			}
		}

		bool anythingDrawn = false;
		for (UINT32 i = 0; i < numViews; i++)
		{
			RendererView* view = viewGroup.GetView(i);
			view->UpdateAsyncOperations();
			
			auto viewId = (UINT64)view;
			const RendererViewTargetData& viewTarget = view->GetProperties().target;
			String title = StringUtil::format("({0} x {1})", viewTarget.targetWidth, viewTarget.targetHeight);
			gProfilerGPU().BeginView(viewId, ProfilerString(title.data(), title.size()));
			
			if (!view->ShouldDraw())
			{
				gProfilerGPU().EndView();
				continue;
			}
			
			const RenderSettings& settings = view->GetRenderSettings();
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

			gProfilerGPU().EndView();
		}

		return anythingDrawn;
	}

	void RenderBeast::RenderView(const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
	{
		gProfilerCPU().BeginSample("Render view");

		const SceneInfo& sceneInfo = mScene->GetSceneInfo();
		auto& viewProps = view.GetProperties();

		SPtr<GpuParamBlockBuffer> perCameraBuffer = view.GetPerViewBuffer();
		perCameraBuffer->FlushToGPU();

		// Make sure light probe data is up to date
		if(view.GetRenderSettings().enableIndirectLighting)
			mScene->UpdateLightProbes();

		view.BeginFrame(frameInfo);

		RenderCompositorNodeInputs Inputs(viewGroup, view, sceneInfo, *mCoreOptions, frameInfo, mFeatureSet);

		// Register callbacks
		if (viewProps.triggerCallbacks)
		{
			const Camera* camera = view.GetSceneCamera();
			for (auto& extension : mCallbacks)
			{
				RenderLocation location = extension->GetLocation();
				RendererExtensionRequest request = extension->Check(*camera);

				if (request == RendererExtensionRequest::DontRender)
					continue;

				switch(location)
				{
				case RenderLocation::Prepare:
					inputs.extPrepare.Add(extension);
					break;
				case RenderLocation::PreBasePass:
					inputs.extPreBasePass.Add(extension);
					break;
				case RenderLocation::PostBasePass:
					inputs.extPostBasePass.Add(extension);
					break;
				case RenderLocation::PostLightPass:
					inputs.extPostLighting.Add(extension);
					break;
				case RenderLocation::Overlay:
					inputs.extOverlay.Add(extension);
					break;
				default:
					break;
				}
			}
		}

		const RenderCompositor& compositor = view.GetCompositor();
		PROFILE_CALL(compositor.Execute(inputs), "Compositor")

		view.EndFrame();

		gProfilerCPU().EndSample("Render view");
	}

	bool RenderBeast::RenderOverlay(RendererView& view, const FrameInfo& frameInfo)
	{
		gProfilerCPU().BeginSample("Render overlay");

		view.GetPerViewBuffer()->FlushToGPU();
		view.BeginFrame(frameInfo);

		auto& viewProps = view.GetProperties();
		const Camera* camera = view.GetSceneCamera();
		SPtr<RenderTarget> target = viewProps.target.target;
		SPtr<Viewport> viewport = camera->GetViewport();

		ClearFlags clearFlags = viewport->GetClearFlags();
		UINT32 clearBuffers = 0;
		if (clearFlags.IsSet(ClearFlagBits::Color))
			clearBuffers |= FBT_COLOR;

		if (clearFlags.IsSet(ClearFlagBits::Depth))
			clearBuffers |= FBT_DEPTH;

		if (clearFlags.IsSet(ClearFlagBits::Stencil))
			clearBuffers |= FBT_STENCIL;

		RenderAPI& rapi = RenderAPI::instance();
		if (clearBuffers != 0)
		{
			rapi.SetRenderTarget(target);
			rapi.ClearViewport(clearBuffers, viewport->GetClearColorValue(),
				viewport->GetClearDepthValue(), viewport->getClearStencilValue());
		}
		else
			rapi.SetRenderTarget(target, 0, RT_COLOR0);

		rapi.SetViewport(viewport->GetArea());

		// Trigger overlay callbacks
		bool needsRedraw = false;
		if(!mCallbacks.Empty())
		{
			view._notifyCompositorTargetChanged(target);

			mOverlayExtensions.Clear();

			for(auto& entry : mCallbacks)
			{
				if (entry->GetLocation() != RenderLocation::Overlay)
					continue;

				RendererExtensionRequest request = entry->Check(*camera);
				if (request == RendererExtensionRequest::DontRender)
					continue;

				if (request == RendererExtensionRequest::ForceRender)
					needsRedraw = true;

				mOverlayExtensions.push_back(entry);
			}

			if (!needsRedraw)
				mOverlayExtensions.Clear();

			for (auto& entry : mOverlayExtensions)
				entry->Render(*camera, view.GetContext());
		}

		view.EndFrame();

		gProfilerCPU().EndSample("Render overlay");
		return needsRedraw;
	}
	
	void RenderBeast::UpdateReflProbeArray()
	{
		SceneInfo& sceneInfo = mScene->_getSceneInfo();
		UINT32 numProbes = (UINT32)sceneInfo.reflProbes.Size();

		bs_frame_mark();
		{		
			UINT32 currentCubeArraySize = 0;

			if(sceneInfo.reflProbeCubemapsTex != nullptr)
				currentCubeArraySize = sceneInfo.reflProbeCubemapsTex->GetProperties().GetNumArraySlices();

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

			auto& cubemapArrayProps = sceneInfo.reflProbeCubemapsTex->GetProperties();

			FrameQueue<UINT32> emptySlots;
			for (UINT32 i = 0; i < numProbes; i++)
			{
				const RendererReflectionProbe& probeInfo = sceneInfo.reflProbes[i];

				if (probeInfo.arrayIdx > MaxReflectionCubemaps)
					continue;

				if(probeInfo.arrayDirty || forceArrayUpdate)
				{
					SPtr<Texture> texture = probeInfo.probe->GetFilteredTexture();
					if (texture == nullptr)
						continue;

					auto& srcProps = texture->GetProperties();
					bool isValid = srcProps.GetWidth() == IBLUtility::REFLECTION_CUBEMAP_SIZE &&
						srcProps.GetHeight() == IBLUtility::REFLECTION_CUBEMAP_SIZE &&
						srcProps.GetNumMipmaps() == cubemapArrayProps.getNumMipmaps() &&
						srcProps.GetTextureType() == TEX_TYPE_CUBE_MAP;

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
							for(UINT32 mip = 0; mip <= srcProps.GetNumMipmaps(); mip++)
							{
								TEXTURE_COPY_DESC copyDesc;
								copyDesc.srcFace = face;
								copyDesc.srcMip = mip;
								copyDesc.dstFace = probeInfo.arrayIdx * 6 + face;
								copyDesc.dstMip = mip;

								texture->Copy(sceneInfo.reflProbeCubemapsTex, copyDesc);
							}
						}
					}

					mScene->SetReflectionProbeArrayIndex(i, probeInfo.arrayIdx, true);
				}

				// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
			}
		}
		bs_frame_clear();
	}

	void RenderBeast::captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position,
		const CaptureSettings& settings)
	{
		const SceneInfo& sceneInfo = mScene->GetSceneInfo();
		auto& texProps = cubemap->GetProperties();

		Matrix4 projTransform = Matrix4::projectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
		ConvexVolume LocalFrustum(projTransform);
		RenderAPI::instance().ConvertProjectionMatrix(projTransform, projTransform);

		RENDERER_VIEW_DESC viewDesc;
		viewDesc.target.clearFlags = FBT_COLOR | FBT_DEPTH;
		viewDesc.target.clearColor = Color::Black;
		viewDesc.target.clearDepthValue = 1.0f;
		viewDesc.target.clearStencilValue = 0;

		viewDesc.target.nrmViewRect = Rect2(0, 0, 1.0f, 1.0f);
		viewDesc.target.viewRect = Rect2I(0, 0, texProps.GetWidth(), texProps.getHeight());
		viewDesc.target.targetWidth = texProps.GetWidth();
		viewDesc.target.targetHeight = texProps.GetHeight();
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
			const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();
			Matrix4 worldMatrix = viewDesc.viewTransform.Transpose();

			Vector<Plane> WorldPlanes(frustumPlanes.Size());
			UINT32 j = 0;
			for (auto& plane : frustumPlanes)
			{
				worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
				j++;
			}

			viewDesc.cullFrustum = ConvexVolume(worldPlanes);

			// Set up face render target
			RENDER_TEXTURE_DESC cubeFaceRTDesc;
			cubeFaceRTDesc.colorSurfaces[0].texture = cubemap;
			cubeFaceRTDesc.colorSurfaces[0].face = i;
			cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
			
			viewDesc.target.target = RenderTexture::create(cubeFaceRTDesc);

			views[i].SetView(viewDesc);
			views[i].SetRenderSettings(renderSettings);
			views[i].UpdatePerViewBuffer();
		}

		RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

		RendererViewGroup ViewGroup(viewPtrs, 6, false, mCoreOptions->shadowMapSize);
		viewGroup.DetermineVisibility(sceneInfo);

		FrameInfo FrameInfo({ 0.0f, 1.0f / 60.0f, 0 }, PerFrameData());
		renderViews(viewGroup, frameInfo);

		// Make sure the render texture is available for reads
		RenderAPI::instance().SetRenderTarget(nullptr);
	}

	SPtr<RenderBeast> GRenderBeast()
	{
		return std::static_pointer_cast<RenderBeast>(RendererManager::instance().GetActive());
	}
}}
