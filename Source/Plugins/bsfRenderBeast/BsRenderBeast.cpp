//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeast.h"
#include "BsCoreApplication.h"
#include "CoreObject/BsRenderThread.h"
#include "CoreObject/BsCoreObjectManager.h"
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsGpuBuffer.h"
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
#include "RenderAPI/BsGpuBackend.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Resources/BsBuiltinResources.h"

using namespace std::placeholders;

namespace bs {
namespace ct {

RenderBeast::RenderBeast()
{
	mOptions = B3DMakeShared<RenderBeastOptions>();
}

const StringID& RenderBeast::GetName() const
{
	static StringID name = "RenderBeast";
	return name;
}

void RenderBeast::Initialize(const SPtr<GpuDevice>& gpuDevice)
{
	Renderer::Initialize(gpuDevice);

	LoadedRendererTextures textures;
	HTexture bokehFlare = GetBuiltinResources().GetTexture(BuiltinTexture::BokehFlare);
	if(bokehFlare.IsLoaded(false))
		textures.BokehFlare = bokehFlare->GetCore();

	GetCoreThread().PostCommand([this, textures]() { InitializeOnRenderThread(textures); });
}

void RenderBeast::Destroy()
{
	Renderer::Destroy();

	GetCoreThread().PostCommand([this]() { DestroyOnRenderThread(); }, true);
}

void RenderBeast::InitializeOnRenderThread(const LoadedRendererTextures& rendererTextures)
{
	Renderer::InitializeOnRenderThread();

	const GpuDeviceCapabilities& caps = mDevice->GetCapabilities();

	if(
		!caps.HasCapability(RSC_COMPUTE_PROGRAM) ||
		!caps.HasCapability(RSC_LOAD_STORE) ||
		!caps.HasCapability(RSC_TEXTURE_VIEWS))
	{
		mFeatureSet = RenderBeastFeatureSet::DesktopMacOS;
	}

	// Ensure profiler methods can be called from start-up methods
	GetProfilerGPU().BeginFrame();

	RendererUtility::StartUp();
	GpuSort::StartUp();
	GpuResourcePool::StartUp();
	IBLUtility::StartUp<RenderBeastIBLUtility>();
	RendererTextures::StartUp(rendererTextures);

	mCoreOptions = B3DMakeShared<RenderBeastOptions>();
	mScene = B3DMakeShared<RendererScene>(*mDevice, mCoreOptions);

	mMainViewGroup = B3DNew<RendererViewGroup>(nullptr, 0, true);

	StandardDeferred::StartUp();
	ParticleRenderer::StartUp();
	GpuParticleSimulation::StartUp();

	GetProfilerGPU().EndFrame(true);

	RenderCompositor::RegisterNodeType<RCNodeSceneDepth>();
	RenderCompositor::RegisterNodeType<RCNodeBasePass>();
	RenderCompositor::RegisterNodeType<RCNodeLightAccumulation>();
	RenderCompositor::RegisterNodeType<RCNodeSceneColor>();
	RenderCompositor::RegisterNodeType<RCNodeDeferredDirectLighting>();
	RenderCompositor::RegisterNodeType<RCNodeIndirectDiffuseLighting>();
	RenderCompositor::RegisterNodeType<RCNodeDeferredIndirectSpecularLighting>();
	RenderCompositor::RegisterNodeType<RCNodeFinalResolve>();
	RenderCompositor::RegisterNodeType<RCNodeSkybox>();
	RenderCompositor::RegisterNodeType<RCNodePostProcess>();
	RenderCompositor::RegisterNodeType<RCNodeTonemapping>();
	RenderCompositor::RegisterNodeType<RCNodeGaussianDOF>();
	RenderCompositor::RegisterNodeType<RCNodeBokehDOF>();
	RenderCompositor::RegisterNodeType<RCNodeFXAA>();
	RenderCompositor::RegisterNodeType<RCNodeResolvedSceneDepth>();
	RenderCompositor::RegisterNodeType<RCNodeHiZ>();
	RenderCompositor::RegisterNodeType<RCNodeSSAO>();
	RenderCompositor::RegisterNodeType<RCNodeClusteredForward>();
	RenderCompositor::RegisterNodeType<RCNodeSSR>();
	RenderCompositor::RegisterNodeType<RCNodeMSAACoverage>();
	RenderCompositor::RegisterNodeType<RCNodeParticleSimulate>();
	RenderCompositor::RegisterNodeType<RCNodeParticleSort>();
	RenderCompositor::RegisterNodeType<RCNodeHalfSceneColor>();
	RenderCompositor::RegisterNodeType<RCNodeBloom>();
	RenderCompositor::RegisterNodeType<RCNodeEyeAdaptation>();
	RenderCompositor::RegisterNodeType<RCNodeScreenSpaceLensFlare>();
	RenderCompositor::RegisterNodeType<RCNodeSceneColorDownsamples>();
	RenderCompositor::RegisterNodeType<RCNodeMotionBlur>();
	RenderCompositor::RegisterNodeType<RCNodeChromaticAberration>();
	RenderCompositor::RegisterNodeType<RCNodeFilmGrain>();
	RenderCompositor::RegisterNodeType<RCNodeTemporalAA>();
}

void RenderBeast::DestroyOnRenderThread()
{
	// Make sure all tasks finish first
	ProcessTasks(true);

	mScene = nullptr;

	RenderCompositor::CleanUp();

	GpuParticleSimulation::ShutDown();
	ParticleRenderer::ShutDown();
	StandardDeferred::ShutDown();

	B3DDelete(mMainViewGroup);

	RendererTextures::ShutDown();
	IBLUtility::ShutDown();
	GpuResourcePool::ShutDown();
	GpuSort::ShutDown();
	RendererUtility::ShutDown();

	Renderer::DestroyOnRenderThread();
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

void RenderBeast::NotifyCameraUpdated(Camera* camera, u32 updateFlag)
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
	bool filteringChanged = mCoreOptions->Filtering != options.Filtering;
	if(options.Filtering == RenderBeastFiltering::Anisotropic)
		filteringChanged |= mCoreOptions->AnisotropyMax != options.AnisotropyMax;

	if(filteringChanged)
		mScene->RefreshSamplerOverrides(true);

	*mCoreOptions = options;

	mScene->SetOptions(mCoreOptions);

	ShadowRendering& shadowRenderer = mMainViewGroup->GetShadowRenderer();
	shadowRenderer.SetShadowMapSize(mCoreOptions->ShadowMapSize);
}

void RenderBeast::RenderAll(PerFrameData perFrameData)
{
	// Sync all dirty sim thread CoreObject data to core thread
	PROFILE_CALL(CoreObjectManager::Instance().SyncToCore(true), "Sync to core")

	if(mOptionsDirty)
	{
		GetCoreThread().PostCommand(std::bind(&::bs::ct::RenderBeast::SyncOptions, this, *mOptions));
		mOptionsDirty = false;
	}

	FrameTimings timings;
	timings.Time = GetTime().GetTime();
	timings.TimeDelta = GetTime().GetFrameDelta();
	timings.FrameIdx = GetTime().GetCurrentFrameIndex();

	GetCoreThread().PostCommand(std::bind(&::bs::ct::RenderBeast::RenderAllCore, this, timings, perFrameData));
}

void RenderBeast::RenderAllCore(FrameTimings timings, PerFrameData perFrameData)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	GetProfilerGPU().BeginFrame();
	GetProfilerCPU().BeginSample("Render");

	SPtr<GpuCommandBuffer> commandBuffer = mCommandBufferPool->Create(GpuCommandBufferCreateInformation::Create("Main"));
	const SceneInfo& sceneInfo = mScene->GetSceneInfo();

	// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
	// issue consider handling this internally in ct::Material which can only do it when sampler states
	// are actually modified after sync
	mScene->RefreshSamplerOverrides();

	// Update global per-frame hardware buffers
	mScene->SetParamFrameParams(timings.Time);

	// Update bounds for all particle systems
	if(perFrameData.Particles)
		PROFILE_CALL(mScene->UpdateParticleSystemBounds(perFrameData.Particles), "Particle bounds")

	sceneInfo.RenderableReady.resize(sceneInfo.Renderables.size(), false);
	sceneInfo.RenderableReady.assign(sceneInfo.Renderables.size(), false);

	FrameInfo frameInfo(timings, perFrameData);

	// Make sure any renderer tasks finish first, as rendering might depend on them
	ProcessTasks(false, timings.FrameIdx);

	mDevice->BeginFrame();

	if (mIsFrameCaptureRequested)
		GpuBackend::Instance().StartCapture();

	// If any reflection probes were updated or added, we need to copy them over in the global reflection probe array
	UpdateReflProbeArray(*commandBuffer);

	// Update per-frame data for all renderable objects
	for(u32 i = 0; i < sceneInfo.Renderables.size(); i++)
		mScene->PrepareRenderable(i, frameInfo);

	for(u32 i = 0; i < sceneInfo.ParticleSystems.size(); i++)
		mScene->PrepareParticleSystem(i, frameInfo);

	for(u32 i = 0; i < sceneInfo.Decals.size(); i++)
		mScene->PrepareDecal(i, frameInfo);

	// Gather all views
	for(auto& rtInfo : sceneInfo.RenderTargets)
	{
		Vector<RendererView*> views;
		SPtr<RenderTarget> target = rtInfo.Target;
		const Vector<Camera*>& cameras = rtInfo.Cameras;

		u32 numCameras = (u32)cameras.size();
		for(u32 i = 0; i < numCameras; i++)
		{
			u32 viewIdx = sceneInfo.CameraToView.at(cameras[i]);
			RendererView* viewInfo = sceneInfo.Views[viewIdx];

			if (mIsFrameCaptureRequested)
				viewInfo->NotifyNeedsRedraw();

			views.push_back(viewInfo);
		}

		mMainViewGroup->SetViews(views.data(), (u32)views.size());
		PROFILE_CALL(mMainViewGroup->DetermineVisibility(*commandBuffer, sceneInfo), "Determine visibility")

		// Render everything
		const bool anythingDrawn = RenderViews(*commandBuffer, *mMainViewGroup, frameInfo);
		if(anythingDrawn)
		{
			mDevice->SubmitCommandBuffer(commandBuffer);
			commandBuffer = mCommandBufferPool->Create(GpuCommandBufferCreateInformation::Create("Main"));

			if(rtInfo.Target->GetProperties().IsWindow)
			{
				mDevice->PresentRenderWindow(std::static_pointer_cast<RenderWindow>(rtInfo.Target));
			}
		}
	}

	mDevice->SubmitCommandBuffer(commandBuffer);
	mDevice->EndFrame();

	// Tick pool frame
	GpuResourcePool::Instance().Update();

	GetProfilerGPU().EndFrame();
	GetProfilerCPU().EndSample("Render");

	if(mIsFrameCaptureRequested)
	{
		mDevice->WaitUntilIdle();
		GpuBackend::Instance().StopCapture();

		mIsFrameCaptureRequested = false;
	}
}

bool RenderBeast::RenderViews(GpuCommandBuffer& commandBuffer, RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
{
	bool needs3DRender = false;
	u32 numViews = viewGroup.GetNumViews();
	for(u32 i = 0; i < numViews; i++)
	{
		RendererView* view = viewGroup.GetView(i);

		if(view->ShouldDraw3D())
		{
			needs3DRender = true;
			break;
		}
	}

	if(needs3DRender)
	{
		const SceneInfo& sceneInfo = mScene->GetSceneInfo();
		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		// Render shadow maps
		ShadowRendering& shadowRenderer = viewGroup.GetShadowRenderer();
		shadowRenderer.RenderShadowMaps(commandBuffer,*mScene, viewGroup, frameInfo);

		// Update various buffers required by each renderable
		u32 numRenderables = (u32)sceneInfo.Renderables.size();
		for(u32 i = 0; i < numRenderables; i++)
		{
			if(!visibility.Renderables[i])
				continue;

			mScene->PrepareVisibleRenderable(i, frameInfo);
		}
	}

	bool anythingDrawn = false;
	for(u32 i = 0; i < numViews; i++)
	{
		RendererView* view = viewGroup.GetView(i);
		view->UpdateAsyncOperations();

		auto viewId = (u64)view;
		const RendererViewTargetData& viewTarget = view->GetProperties().Target;
		String title = StringUtil::Format("({0} x {1})", viewTarget.TargetWidth, viewTarget.TargetHeight);
		GetProfilerGPU().BeginView(commandBuffer, viewId, ProfilerString(title.data(), title.size()));

		if(!view->ShouldDraw())
		{
			GetProfilerGPU().EndView(commandBuffer);
			continue;
		}

		const RenderSettings& settings = view->GetRenderSettings();
		if(settings.OverlayOnly)
		{
			if(RenderOverlay(commandBuffer, *view, frameInfo))
				anythingDrawn = true;
		}
		else
		{
			RenderView(commandBuffer, viewGroup, *view, frameInfo);
			anythingDrawn = true;
		}

		GetProfilerGPU().EndView(commandBuffer);
	}

	return anythingDrawn;
}

void RenderBeast::RenderView(GpuCommandBuffer& commandBuffer, const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
{
	GetProfilerCPU().BeginSample("Render view");

	const SceneInfo& sceneInfo = mScene->GetSceneInfo();
	auto& viewProps = view.GetProperties();

	SPtr<GpuBuffer> perCameraBuffer = view.GetPerViewBuffer();
	perCameraBuffer->FlushCache();

	// Make sure light probe data is up to date
	if(view.GetRenderSettings().EnableIndirectLighting)
		mScene->UpdateLightProbes(commandBuffer);

	view.BeginFrame(frameInfo);

	RenderCompositorNodeInputs inputs(viewGroup, view, sceneInfo, *mCoreOptions, frameInfo, mFeatureSet);
	inputs.ActiveCommandBuffer = commandBuffer.GetShared();

	// Register callbacks
	if(viewProps.TriggerCallbacks)
	{
		const Camera* camera = view.GetSceneCamera();
		for(auto& extension : mCallbacks)
		{
			RenderLocation location = extension->GetLocation();
			RendererExtensionRequest request = extension->Check(*camera);

			if(request == RendererExtensionRequest::DontRender)
				continue;

			switch(location)
			{
			case RenderLocation::Prepare:
				inputs.ExtPrepare.Add(extension);
				break;
			case RenderLocation::PreBasePass:
				inputs.ExtPreBasePass.Add(extension);
				break;
			case RenderLocation::PostBasePass:
				inputs.ExtPostBasePass.Add(extension);
				break;
			case RenderLocation::PostLightPass:
				inputs.ExtPostLighting.Add(extension);
				break;
			case RenderLocation::Overlay:
				inputs.ExtOverlay.Add(extension);
				break;
			default:
				break;
			}
		}
	}

	const RenderCompositor& compositor = view.GetCompositor();
	PROFILE_CALL(compositor.Execute(inputs), "Compositor")

	view.EndFrame();

	GetProfilerCPU().EndSample("Render view");
}

bool RenderBeast::RenderOverlay(GpuCommandBuffer& commandBuffer, RendererView& view, const FrameInfo& frameInfo)
{
	GetProfilerCPU().BeginSample("Render overlay");

	view.GetPerViewBuffer()->FlushCache();
	view.BeginFrame(frameInfo);

	auto& viewProps = view.GetProperties();
	const Camera* camera = view.GetSceneCamera();
	SPtr<RenderTarget> target = viewProps.Target.Target;
	SPtr<Viewport> viewport = camera->GetViewport();

	ClearFlags clearFlags = viewport->GetClearFlags();
	u32 clearBuffers = 0;
	if(clearFlags.IsSet(ClearFlagBits::Color))
		clearBuffers |= FBT_COLOR;

	if(clearFlags.IsSet(ClearFlagBits::Depth))
		clearBuffers |= FBT_DEPTH;

	if(clearFlags.IsSet(ClearFlagBits::Stencil))
		clearBuffers |= FBT_STENCIL;

	if(clearBuffers != 0)
	{
		commandBuffer.SetRenderTarget(target);
		commandBuffer.ClearViewport(clearBuffers, viewport->GetClearColorValue(), viewport->GetClearDepthValue(), viewport->GetClearStencilValue());
	}
	else
		commandBuffer.SetRenderTarget(target, 0, RT_COLOR0);

	commandBuffer.SetViewport(viewport->GetArea());

	// Trigger overlay callbacks
	bool needsRedraw = false;
	if(!mCallbacks.empty())
	{
		view.NotifyCompositorTargetChangedInternal(target);

		mOverlayExtensions.clear();

		for(auto& entry : mCallbacks)
		{
			if(entry->GetLocation() != RenderLocation::Overlay)
				continue;

			RendererExtensionRequest request = entry->Check(*camera);
			if(request == RendererExtensionRequest::DontRender)
				continue;

			if(request == RendererExtensionRequest::ForceRender)
				needsRedraw = true;

			mOverlayExtensions.push_back(entry);
		}

		if(!needsRedraw)
			mOverlayExtensions.clear();

		for(auto& entry : mOverlayExtensions)
		{
			RendererViewContext context;
			context.CurrentTarget = view.GetCompositorRenderTarget();
			context.CommandBuffer = commandBuffer.GetShared();

			entry->Render(*camera, context);
		}
	}

	view.EndFrame();

	GetProfilerCPU().EndSample("Render overlay");
	return needsRedraw;
}

void RenderBeast::UpdateReflProbeArray(GpuCommandBuffer& commandBuffer)
{
	SceneInfo& sceneInfo = mScene->GetSceneInfoInternal();
	u32 numProbes = (u32)sceneInfo.ReflProbes.size();

	B3DMarkAllocatorFrame();
	{
		u32 currentCubeArraySize = 0;

		if(sceneInfo.ReflProbeCubemapsTex != nullptr)
			currentCubeArraySize = sceneInfo.ReflProbeCubemapsTex->GetProperties().ArraySliceCount;

		bool forceArrayUpdate = false;
		if(sceneInfo.ReflProbeCubemapsTex == nullptr || (currentCubeArraySize < numProbes && currentCubeArraySize != kMaxReflectionCubemaps))
		{
			TextureCreateInformation cubeMapDesc;
			cubeMapDesc.Name = "Reflection Probe Cubemap Array";
			cubeMapDesc.Type = TEX_TYPE_CUBE_MAP;
			cubeMapDesc.Format = PF_RG11B10F;
			cubeMapDesc.Width = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.Height = IBLUtility::kReflectionCubemapSize;
			cubeMapDesc.MipMapCount = PixelUtil::GetMaxMipmaps(cubeMapDesc.Width, cubeMapDesc.Height, 1, cubeMapDesc.Format);
			cubeMapDesc.ArraySliceCount = std::min(kMaxReflectionCubemaps, numProbes + 4); // Keep a few empty entries

			sceneInfo.ReflProbeCubemapsTex = mDevice->CreateTexture(cubeMapDesc);

			forceArrayUpdate = true;
		}

		auto& cubemapArrayProps = sceneInfo.ReflProbeCubemapsTex->GetProperties();

		FrameQueue<u32> emptySlots;
		for(u32 i = 0; i < numProbes; i++)
		{
			const RendererReflectionProbe& probeInfo = sceneInfo.ReflProbes[i];

			if(probeInfo.ArrayIdx > kMaxReflectionCubemaps)
				continue;

			if(probeInfo.ArrayDirty || forceArrayUpdate)
			{
				SPtr<Texture> texture = probeInfo.Probe->GetFilteredTexture();
				if(texture == nullptr)
					continue;

				auto& srcProps = texture->GetProperties();
				bool isValid = srcProps.Width == IBLUtility::kReflectionCubemapSize &&
					srcProps.Height == IBLUtility::kReflectionCubemapSize &&
					srcProps.MipMapCount == cubemapArrayProps.MipMapCount &&
					srcProps.Type == TEX_TYPE_CUBE_MAP;

				if(!isValid)
				{
					if(!probeInfo.ErrorFlagged)
					{
						B3D_LOG(Error, Renderer, "Cubemap texture invalid to use as a reflection cubemap. "
												"Check texture size (must be {0}x{0}) and mip-map count",
							   IBLUtility::kReflectionCubemapSize);

						probeInfo.ErrorFlagged = true;
					}
				}
				else
				{
					for(u32 face = 0; face < 6; face++)
					{
						for(u32 mip = 0; mip <= srcProps.MipMapCount; mip++)
						{
							TextureCopyInformation copyDesc;
							copyDesc.SourceFace = face;
							copyDesc.SourceMip = mip;
							copyDesc.DestinationFace = probeInfo.ArrayIdx * 6 + face;
							copyDesc.DestinationMip = mip;

							texture->Copy(commandBuffer, sceneInfo.ReflProbeCubemapsTex, copyDesc);
						}
					}
				}

				mScene->SetReflectionProbeArrayIndex(i, probeInfo.ArrayIdx, true);
			}

			// Note: Consider pruning the reflection cubemap array if empty slot count becomes too high
		}
	}
	B3DClearAllocatorFrame();
}

void RenderBeast::CaptureSceneCubeMap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings)
{
	const SceneInfo& sceneInfo = mScene->GetSceneInfo();
	auto& texProps = cubemap->GetProperties();

	Matrix4 projTransform = Matrix4::ProjectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
	ConvexVolume localFrustum(projTransform);

	GpuDevice& gpuDevice = commandBuffer.GetGpuDevice();
	gpuDevice.ConvertProjectionMatrix(projTransform, projTransform);

	RENDERER_VIEW_DESC viewDesc;
	viewDesc.Target.ClearFlags = FBT_COLOR | FBT_DEPTH;
	viewDesc.Target.ClearColor = Color::kBlack;
	viewDesc.Target.ClearDepthValue = 1.0f;
	viewDesc.Target.ClearStencilValue = 0;

	viewDesc.Target.NrmViewRect = Rect2(0, 0, 1.0f, 1.0f);
	viewDesc.Target.ViewRect = Rect2I(0, 0, texProps.Width, texProps.Height);
	viewDesc.Target.TargetWidth = texProps.Width;
	viewDesc.Target.TargetHeight = texProps.Height;
	viewDesc.Target.NumSamples = 1;

	viewDesc.MainView = false;
	viewDesc.TriggerCallbacks = false;
	viewDesc.RunPostProcessing = false;
	viewDesc.CapturingReflections = true;
	viewDesc.OnDemand = false;
	viewDesc.EncodeDepth = settings.EncodeDepth;
	viewDesc.DepthEncodeNear = settings.DepthEncodeNear;
	viewDesc.DepthEncodeFar = settings.DepthEncodeFar;

	viewDesc.VisibleLayers = 0xFFFFFFFFFFFFFFFF;
	viewDesc.NearPlane = 0.5f;
	viewDesc.FarPlane = 1000.0f;
	viewDesc.FlipView = mDevice->GetCapabilities().Conventions.UvYAxis != GpuBackendConventions::Axis::Up;

	viewDesc.ViewOrigin = position;
	viewDesc.ProjTransform = projTransform;
	viewDesc.ProjType = PT_PERSPECTIVE;

	viewDesc.StateReduction = mCoreOptions->StateReductionMode;
	viewDesc.SceneCamera = nullptr;

	SPtr<RenderSettings> renderSettings = B3DMakeShared<RenderSettings>();
	renderSettings->EnableHdr = settings.Hdr;
	renderSettings->EnableShadows = true;
	renderSettings->EnableIndirectLighting = false;
	renderSettings->ScreenSpaceReflections.Enabled = false;
	renderSettings->AmbientOcclusion.Enabled = false;

	Matrix4 viewOffsetMat = Matrix4::Translation(-position);

	// Note: We render upside down, then flip the image vertically, which results in a horizontal flip. The horizontal
	// flip is required due to the fact how cubemap faces are defined. Another option would be to change the view
	// orientation matrix, but that also requires a culling mode flip which is inconvenient to do globally.
	RendererView views[6];
	for(u32 i = 0; i < 6; i++)
	{
		// Calculate view matrix
		Vector3 forward;
		Vector3 up = Vector3::kUnitY;

		switch(i)
		{
		case CF_PositiveX:
			forward = -Vector3::kUnitX;
			up = -Vector3::kUnitY;
			break;
		case CF_NegativeX:
			forward = Vector3::kUnitX;
			up = -Vector3::kUnitY;
			break;
		case CF_PositiveY:
			forward = Vector3::kUnitY;
			up = -Vector3::kUnitZ;
			break;
		case CF_NegativeY:
			forward = -Vector3::kUnitY;
			up = Vector3::kUnitZ;
			break;
		case CF_PositiveZ:
			forward = -Vector3::kUnitZ;
			up = -Vector3::kUnitY;
			break;
		case CF_NegativeZ:
			forward = Vector3::kUnitZ;
			up = -Vector3::kUnitY;
			break;
		}

		Vector3 right = Vector3::Cross(up, forward);
		Matrix3 viewRotationMat = Matrix3(right, up, forward);

		viewDesc.ViewDirection = -forward;
		viewDesc.ViewTransform = Matrix4(viewRotationMat) * viewOffsetMat;

		// Calculate world frustum for culling
		const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();
		Matrix4 worldMatrix = viewDesc.ViewTransform.Transpose();

		Vector<Plane> worldPlanes(frustumPlanes.size());
		u32 j = 0;
		for(auto& plane : frustumPlanes)
		{
			worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
			j++;
		}

		viewDesc.CullFrustum = ConvexVolume(worldPlanes);

		// Set up face render target
		RenderTextureCreateInformation cubeFaceRTDesc;
		cubeFaceRTDesc.ColorSurfaces[0].Texture = cubemap;
		cubeFaceRTDesc.ColorSurfaces[0].Face = i;
		cubeFaceRTDesc.ColorSurfaces[0].FaceCount = 1;

		viewDesc.Target.Target = RenderTexture::Create(cubeFaceRTDesc);

		views[i].SetView(viewDesc);
		views[i].SetRenderSettings(renderSettings);
		views[i].UpdatePerViewBuffer();
	}

	RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

	RendererViewGroup viewGroup(viewPtrs, 6, false, mCoreOptions->ShadowMapSize);
	viewGroup.DetermineVisibility(commandBuffer, sceneInfo);

	FrameInfo frameInfo({ 0.0f, 1.0f / 60.0f, 0 }, PerFrameData());
	RenderViews(commandBuffer, viewGroup, frameInfo);

	// Make sure the render texture is available for reads
	commandBuffer.SetRenderTarget(nullptr);
}

SPtr<RenderBeast> GetRenderBeast()
{
	return std::static_pointer_cast<RenderBeast>(RendererManager::Instance().GetActive());
}
}} // namespace bs::ct
