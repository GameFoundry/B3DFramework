//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeast.h"
#include "CoreObject/B3DRenderThread.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"
#include "RenderAPI/B3DViewport.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Utility/B3DTime.h"
#include "Animation/B3DAnimationScene.h"
#include "Animation/B3DSkeleton.h"
#include "Components/B3DLight.h"
#include "Renderer/B3DRendererExtension.h"
#include "Renderer/B3DRenderSettings.h"
#include "Renderer/B3DIBLUtility.h"
#include "Components/B3DSkybox.h"
#include "Components/B3DCamera.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DRendererTextures.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "Renderer/B3DRendererManager.h"
#include "Shading/B3DShadowRendering.h"
#include "Shading/B3DStandardDeferred.h"
#include "Shading/B3DTiledDeferred.h"
#include "B3DRenderBeastOptions.h"
#include "B3DRenderBeastIBLUtility.h"
#include "B3DRenderCompositor.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Resources/B3DBuiltinResources.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"
#include "B3DRendererParticles.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Math/B3DMath.h"

using namespace std::placeholders;

namespace b3d {
namespace render {

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
	textures.BokehFlare = B3DGetRenderProxy(bokehFlare);

	GetRenderThread().PostCommand([this, textures]() { InitializeOnRenderThread(textures); }, "RenderBeast::InitializeOnRenderThread");
}

void RenderBeast::Destroy()
{
	Renderer::Destroy();

	GetRenderThread().PostCommand([this]() { DestroyOnRenderThread(); }, "RenderBeast::DestroyOnRenderThread", true);
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

	// Create per-object parameter set layouts
	{
		// PerObject uniform buffer info (shared by both layouts)
		GpuUniformBufferInformation perObjectInfo;
		perObjectInfo.Name = "PerObject";
		perObjectInfo.Set = GpuPipelineSet::kPerObject;
		perObjectInfo.Slot = 0;
		perObjectInfo.Size = Math::CeilToMultiple(gPerObjectUniformDefinition.GetSize() / 4u, 4u);
		perObjectInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
		perObjectInfo.IsShareable = true;

		// Create renderable layout (PerObject only)
		{
			GpuProgramParameterDescription renderableDescription;
			renderableDescription.UniformBuffers["PerObject"] = perObjectInfo;
			mRenderableParameterSetInfo.Layout = mDevice->CreateGpuPipelineParameterSetLayout(renderableDescription);

			const u32 perObjectSlot = mRenderableParameterSetInfo.Layout->GetSlot("PerObject");
			if(perObjectSlot != ~0u)
				mRenderableParameterSetInfo.PerObjectDynamicOffsetIndex = mRenderableParameterSetInfo.Layout->GetDynamicOffsetIndex(perObjectSlot);
		}

		// Create decal layout (PerObject + DecalParams)
		{
			GpuProgramParameterDescription decalDescription;
			decalDescription.UniformBuffers["PerObject"] = perObjectInfo;

			GpuUniformBufferInformation decalInfo;
			decalInfo.Name = "DecalParams";
			decalInfo.Set = GpuPipelineSet::kPerObject;
			decalInfo.Slot = 1;
			decalInfo.Size = Math::CeilToMultiple(gDecalUniformDefinition.GetSize() / 4u, 4u);
			decalInfo.Stages = GpuProgramStageBit::Fragment;
			decalInfo.IsShareable = true;
			decalDescription.UniformBuffers["DecalParams"] = decalInfo;

			mDecalParameterSetInfo.Layout = mDevice->CreateGpuPipelineParameterSetLayout(decalDescription);

			const u32 perObjectSlot = mDecalParameterSetInfo.Layout->GetSlot("PerObject");
			if(perObjectSlot != ~0u)
				mDecalParameterSetInfo.PerObjectDynamicOffsetIndex = mDecalParameterSetInfo.Layout->GetDynamicOffsetIndex(perObjectSlot);

			const u32 decalSlot = mDecalParameterSetInfo.Layout->GetSlot("DecalParams");
			if(decalSlot != ~0u)
				mDecalParameterSetInfo.DecalDynamicOffsetIndex = mDecalParameterSetInfo.Layout->GetDynamicOffsetIndex(decalSlot);
		}

		// Create GPU particles layout (PerObject + GpuParticleParams)
		{
			GpuProgramParameterDescription gpuParticlesDescription;
			gpuParticlesDescription.UniformBuffers["PerObject"] = perObjectInfo;

			GpuUniformBufferInformation gpuParticlesInfo;
			gpuParticlesInfo.Name = "GpuParticleParams";
			gpuParticlesInfo.Set = GpuPipelineSet::kPerObject;
			gpuParticlesInfo.Slot = 1;
			gpuParticlesInfo.Size = Math::CeilToMultiple(gGpuParticlesUniformDefinition.GetSize() / 4u, 4u);
			gpuParticlesInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
			gpuParticlesInfo.IsShareable = true;
			gpuParticlesDescription.UniformBuffers["GpuParticleParams"] = gpuParticlesInfo;

			mGpuParticlesParameterSetInfo.Layout = mDevice->CreateGpuPipelineParameterSetLayout(gpuParticlesDescription);

			const u32 perObjectSlot = mGpuParticlesParameterSetInfo.Layout->GetSlot("PerObject");
			if(perObjectSlot != ~0u)
				mGpuParticlesParameterSetInfo.PerObjectDynamicOffsetIndex = mGpuParticlesParameterSetInfo.Layout->GetDynamicOffsetIndex(perObjectSlot);

			const u32 gpuParticlesSlot = mGpuParticlesParameterSetInfo.Layout->GetSlot("GpuParticleParams");
			if(gpuParticlesSlot != ~0u)
				mGpuParticlesParameterSetInfo.GpuParticlesDynamicOffsetIndex = mGpuParticlesParameterSetInfo.Layout->GetDynamicOffsetIndex(gpuParticlesSlot);
		}
	}

	// Build type configurations for UniformBufferPools
	{
		using PoolConfiguration = UniformBufferPools::PoolConfiguration;

		// Normal type - just PerObject buffer
		{
			PoolConfiguration config;
			config.Type = UniformBufferPools::RenderablePool;
			config.EntriesPerBuffer = 1024;
			config.Layout = mRenderableParameterSetInfo.Layout;
			config.Buffers.Add({UniformBufferPools::PerObjectBuffer, "PerObject", gPerObjectUniformDefinition.GetSize(), GpuBufferFlag::StoreOnGPU});
			mTypeConfigurations.Add(config);
		}

		// Decal type - PerObject + DecalParams buffers
		{
			PoolConfiguration config;
			config.Type = UniformBufferPools::DecalPool;
			config.EntriesPerBuffer = 256;
			config.Layout = mDecalParameterSetInfo.Layout;
			config.Buffers.Add({UniformBufferPools::PerObjectBuffer, "PerObject", gPerObjectUniformDefinition.GetSize(), GpuBufferFlag::StoreOnGPU});
			config.Buffers.Add({UniformBufferPools::DecalBuffer, "DecalParams", gDecalUniformDefinition.GetSize(), GpuBufferFlag::StoreOnGPU});
			mTypeConfigurations.Add(config);
		}

		// GPU particles type - PerObject + GpuParticleParams buffers
		{
			PoolConfiguration config;
			config.Type = UniformBufferPools::GpuParticlesPool;
			config.EntriesPerBuffer = 256;
			config.Layout = mGpuParticlesParameterSetInfo.Layout;
			config.Buffers.Add({UniformBufferPools::PerObjectBuffer, "PerObject", gPerObjectUniformDefinition.GetSize(), GpuBufferFlag::StoreOnGPU});
			config.Buffers.Add({UniformBufferPools::GpuParticlesBuffer, "GpuParticleParams", gGpuParticlesUniformDefinition.GetSize(), GpuBufferFlag::StoreOnGPU});
			mTypeConfigurations.Add(config);
		}
	}

	RendererUtility::StartUp();
	GpuSort::StartUp();
	GpuResourcePool::StartUp();
	IBLUtility::StartUp<RenderBeastIBLUtility>();
	RendererTextures::StartUp(rendererTextures);

	mRenderThreadOptions = B3DMakeShared<RenderBeastOptions>();
	mMainViewGroup = B3DNew<RendererViewGroup>(nullptr, 0, true);

	StandardDeferred::StartUp();
	ParticleRenderer::StartUp();
	GpuParticleSimulation::StartUp();

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

	while(!mScenes.empty())
	{
		RenderBeastScene* const scene = mScenes.back();
		scene->Destroy();
	}

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

void RenderBeast::NotifySceneCreated(const SPtr<RenderBeastScene>& scene)
{
	mScenes.push_back(scene.get());
}

void RenderBeast::NotifySceneDestroyed(const RenderBeastScene* scene)
{
	auto found = std::find_if(mScenes.begin(), mScenes.end(), [scene](const RenderBeastScene* otherScene)
	{
		return otherScene == scene;
	});
	
	if(B3D_ENSURE(found != mScenes.end()))
		mScenes.erase(found);
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
	bool filteringChanged = mRenderThreadOptions->Filtering != options.Filtering;
	if(options.Filtering == RenderBeastFiltering::Anisotropic)
		filteringChanged |= mRenderThreadOptions->AnisotropyMax != options.AnisotropyMax;

	if(filteringChanged)
	{
		for(const auto& entry : mScenes)
			entry->RefreshSamplerOverrides(true);
	}

	*mRenderThreadOptions = options;

	for(const auto& entry : mScenes)
		entry->SetOptions(mRenderThreadOptions);

	ShadowRendering& shadowRenderer = mMainViewGroup->GetShadowRenderer();
	shadowRenderer.SetShadowMapSize(mRenderThreadOptions->ShadowMapSize);
}

void RenderBeast::RenderAll(PerFrameData perFrameData)
{
	// Sync all dirty main thread CoreObject data to the render thread
	PROFILE_CALL(CoreObjectManager::Instance().SyncToRenderThread(true), "Sync to render thread")

	if(mOptionsDirty)
	{
		GetRenderThread().PostCommand([this, options = *mOptions]() { SyncOptions(options); }, "RenderBeast::SyncOptions");
		mOptionsDirty = false;
	}

	FrameTimings timings;
	timings.Time = GetTime().GetRealTimeInSeconds();
	timings.TimeDelta = GetTime().GetFrameDelta();
	timings.FrameIndex = GetTime().GetCurrentFrameIndex();

	GetRenderThread().PostCommand([this, timings, perFrameData]() { RenderAllOnRenderThread(timings, perFrameData); }, "RenderBeast::RenderAll");
}

void RenderBeast::RenderAllOnRenderThread(FrameTimings timings, PerFrameData perFrameData)
{
	ASSERT_IF_NOT_RENDER_THREAD

	GetProfilerCPU().BeginSample("Render");

	// Make sure any renderer tasks finish first, as rendering might depend on them
	ProcessTasks(false, timings.FrameIndex);

	for(auto& entry : mScenes)
	{
		RendererScene* const rendererScene = entry;

		if(auto foundSceneData = perFrameData.PerSceneData.find(rendererScene); foundSceneData != perFrameData.PerSceneData.end())
		{
			const bool asynchronousAnimationEvaluation = foundSceneData->second.Animation != nullptr ? foundSceneData->second.Animation->AsynchronousEvaluation : false;

			FrameInfo frameInfo(timings, asynchronousAnimationEvaluation, foundSceneData->second);
			RenderScene(*entry, frameInfo);
		}
		else
		{
			FrameInfo frameInfo(timings, false, PerSceneFrameData());
			RenderScene(*entry, frameInfo);
		}

		entry->GetUniformBufferPools().AdvanceFrame();
	}

	mDevice->EndFrame();

	// Tick pool frame
	GpuResourcePool::Instance().Update();

	GetProfilerCPU().EndSample("Render");

	if(mIsFrameCaptureRequested)
	{
		mDevice->WaitUntilIdle();
		GpuBackend::Instance().StopCapture();

		mIsFrameCaptureRequested = false;
	}

	GpuUniformBufferManager::Instance().AdvanceFrame();
	mCommandBufferPoolRing->AdvanceFrame();
	mRendererExtensionsDirty = false;
}

bool RenderBeast::RenderScene(RenderBeastScene& scene, const FrameInfo& frameInfo)
{
	SPtr<GpuCommandBuffer> commandBuffer = mCommandBufferPoolRing->GetCurrentPool().Create(GpuCommandBufferCreateInformation::Create("Main"));
#if B3D_PROFILING_ENABLED
	commandBuffer->BeginProfiling("RenderScene");
#endif

	const SceneInfo& sceneInfo = scene.GetSceneInfo();

	scene.UpdateCombinedRendererExtensionsIfNeeded(mRendererExtensions, mRendererExtensionsDirty);

	// Note: I'm iterating over all sampler states every frame. If this ends up being a performance
	// issue consider handling this internally in render::Material which can only do it when sampler states
	// are actually modified after sync
	scene.RefreshSamplerOverrides();

	// Update global per-frame hardware buffers
	scene.SetParamFrameParams(frameInfo.Timings.Time);

	// Update bounds for all particle systems
	if(frameInfo.PerSceneFrameData.Particles)
		PROFILE_CALL(scene.UpdateParticleSystemBounds(frameInfo.PerSceneFrameData.Particles), "Particle bounds")

	sceneInfo.RenderableReady.resize(sceneInfo.Renderables.size(), false);
	sceneInfo.RenderableReady.assign(sceneInfo.Renderables.size(), false);

	mDevice->BeginFrame();

	if (mIsFrameCaptureRequested)
		GpuBackend::Instance().StartCapture();

	// If any reflection probes were updated or added, we need to copy them over in the global reflection probe array
	scene.UpdateReflectionProbes(*commandBuffer);

	// Update per-frame data for all renderable objects
	for(u32 i = 0; i < sceneInfo.Renderables.size(); i++)
		scene.PrepareRenderable(i, frameInfo);

	for(u32 i = 0; i < sceneInfo.ParticleSystems.size(); i++)
		scene.PrepareParticleSystem(i, frameInfo);

	for(u32 i = 0; i < sceneInfo.Decals.size(); i++)
		scene.PrepareDecal(i, frameInfo);

	bool anythingDrawnForScene = false;
	for(auto& rtInfo : sceneInfo.RenderTargets)
	{
		Vector<RendererView*> views;
		SPtr<RenderTarget> target = rtInfo.Target;
		const Vector<Camera*>& cameras = rtInfo.Cameras;

		const bool isWindow = target->GetProperties().IsWindow;
		const SPtr<RenderWindow> window = std::static_pointer_cast<RenderWindow>(rtInfo.Target);
		const bool renderTargetNeedsRedraw = window != nullptr ? window->IsRedrawRequested() : false;

		const u32 cameraCount = (u32)cameras.size();
		for(u32 i = 0; i < cameraCount; i++)
		{
			const u32 viewIndex = sceneInfo.CameraToView.at(cameras[i]);
			RendererView* viewInfo = sceneInfo.Views[viewIndex];

			if (mIsFrameCaptureRequested || renderTargetNeedsRedraw)
				viewInfo->NotifyNeedsRedraw();

			viewInfo->UpdateAsyncOperations(); // Note: Needs to happen before any ShouldDraw*() calls, to be consistent
			views.push_back(viewInfo);
		}

		mMainViewGroup->SetViews(views.data(), (u32)views.size());
		PROFILE_CALL(mMainViewGroup->DetermineVisibility(*commandBuffer, sceneInfo), "Determine visibility")

		// Render everything
		const bool anythingDrawnForView = RenderViews(*commandBuffer, scene, *mMainViewGroup, frameInfo, renderTargetNeedsRedraw);
		if(anythingDrawnForView)
		{
#if B3D_PROFILING_ENABLED
			commandBuffer->EndProfiling();
#endif
			mDevice->SubmitCommandBuffer(commandBuffer);

			commandBuffer = mCommandBufferPoolRing->GetCurrentPool().Create(GpuCommandBufferCreateInformation::Create("Main"));

#if B3D_PROFILING_ENABLED
			commandBuffer->BeginProfiling("RenderScene");
#endif

			if(isWindow)
				mDevice->PresentRenderWindow(window);

			anythingDrawnForScene = true;
		}
	}

#if B3D_PROFILING_ENABLED
	commandBuffer->EndProfiling();
#endif
	mDevice->SubmitCommandBuffer(commandBuffer);

	return anythingDrawnForScene;
}

bool RenderBeast::RenderViews(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo, bool forceRender)
{
	bool needs3DRender = false;
	u32 viewCount = viewGroup.GetViewCount();
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		RendererView* view = viewGroup.GetView(viewIndex);

		if(view->ShouldDraw3D())
		{
			needs3DRender = true;
			break;
		}
	}

	if(needs3DRender)
	{
		const SceneInfo& sceneInfo = scene.GetSceneInfo();
		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		// Render shadow maps
		ShadowRendering& shadowRenderer = viewGroup.GetShadowRenderer();
		shadowRenderer.RenderShadowMaps(commandBuffer,scene, viewGroup, frameInfo);

		// Update various buffers required by each renderable
		u32 renderableCount = (u32)sceneInfo.Renderables.size();
		for(u32 i = 0; i < renderableCount; i++)
		{
			if(!visibility.Renderables[i])
				continue;

			scene.PrepareVisibleRenderable(i, frameInfo);
		}
	}

	bool anythingDrawn = false;
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		RendererView* view = viewGroup.GetView(viewIndex);

		const RendererViewTargetInformation& viewTarget = view->GetProperties().Target;

		if(!view->ShouldDraw())
			continue;

#if B3D_PROFILING_ENABLED
		const SPtr<GpuCommandBufferProfiler>& commandBufferProfiler = commandBuffer.GetProfiler();
		if(commandBufferProfiler != nullptr)
		{
			const String title = StringUtil::Format("View ({0} x {1})", viewTarget.TargetWidth, viewTarget.TargetHeight);
			commandBufferProfiler->BeginSample(commandBuffer, ProfilerString(title.data(), title.size()));
		}
#endif

		const RenderSettings& settings = view->GetRenderSettings();
		if(settings.OverlayOnly)
		{
			if(RenderOverlay(commandBuffer, scene, *view, frameInfo, forceRender))
				anythingDrawn = true;
		}
		else
		{
			RenderView(commandBuffer, scene, viewGroup, *view, frameInfo);
			anythingDrawn = true;
		}

#if B3D_PROFILING_ENABLED
		if(commandBufferProfiler != nullptr)
			commandBufferProfiler->EndSample(commandBuffer);
#endif
	}

	return anythingDrawn;
}

void RenderBeast::RenderView(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
{
	GetProfilerCPU().BeginSample("Render view");

	const SceneInfo& sceneInfo = scene.GetSceneInfo();
	auto& viewProps = view.GetProperties();

	const GpuBufferSuballocation& perCameraBuffer = view.GetPerViewBuffer();

	// Make sure light probe data is up to date
	if(view.GetRenderSettings().EnableIndirectLighting)
		scene.UpdateLightProbes(commandBuffer);

	view.BeginFrame(frameInfo);

	RenderCompositorNodeInputs inputs(viewGroup, view, sceneInfo, *mRenderThreadOptions, frameInfo, mFeatureSet);
	inputs.ActiveCommandBuffer = commandBuffer.GetShared();

#if B3D_PROFILING_ENABLED
	inputs.CommandBufferProfiler = commandBuffer.GetProfiler();
#endif

	// Register callbacks
	if(viewProps.TriggerCallbacks)
	{
		const Camera* camera = view.GetSceneCamera();
		for(auto& extension : scene.GetCombinedRendererExtensions())
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

bool RenderBeast::RenderOverlay(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererView& view, const FrameInfo& frameInfo, bool forceRender)
{
	GetProfilerCPU().BeginSample("Render overlay");

	view.BeginFrame(frameInfo);

	auto& viewProps = view.GetProperties();
	const Camera* camera = view.GetSceneCamera();
	SPtr<RenderTarget> target = viewProps.Target.Target;
	SPtr<Viewport> viewport = camera->GetViewport();

	ClearFlags clearFlags = viewport->GetClearFlags();
	RenderSurfaceMask clearMask = RT_NONE;
	if(clearFlags.IsSet(ClearFlagBits::Color))
		clearMask |= RT_COLOR_ALL;

	if(clearFlags.IsSet(ClearFlagBits::Depth))
		clearMask |= RT_DEPTH;

	if(clearFlags.IsSet(ClearFlagBits::Stencil))
		clearMask |= RT_STENCIL;

	if(clearMask != RT_NONE)
	{
		commandBuffer.BeginRenderPass(RenderPassCreateInformation(target));
		commandBuffer.ClearViewport(clearMask, viewport->GetClearColorValue(), viewport->GetClearDepthValue(), viewport->GetClearStencilValue());
		commandBuffer.EndRenderPass();
	}

	commandBuffer.SetViewport(viewport->GetArea());

	const Set<RendererExtension*, RendererExtension::SortFunction>& rendererExtensions = scene.GetCombinedRendererExtensions();

	// Trigger overlay callbacks
	bool needsRedraw = false;
	if(!rendererExtensions.empty())
	{
		view.NotifyCompositorTargetChangedInternal(target);

		mOverlayExtensions.clear();

		for(auto& entry : rendererExtensions)
		{
			if(entry->GetLocation() != RenderLocation::Overlay)
				continue;

			RendererExtensionRequest request = entry->Check(*camera);
			if(request == RendererExtensionRequest::DontRender)
				continue;

			if(request == RendererExtensionRequest::ForceRender || forceRender)
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

void RenderBeast::CaptureSceneCubeMap(RendererScene& scene, GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings)
{
	RenderBeastScene& renderBeastScene = static_cast<RenderBeastScene&>(scene);

	const SceneInfo& sceneInfo = renderBeastScene.GetSceneInfo();
	auto& texProps = cubemap->GetProperties();

	Matrix4 projTransform = Matrix4::ProjectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
	ConvexVolume localFrustum(projTransform);

	GpuDevice& gpuDevice = commandBuffer.GetGpuDevice();
	gpuDevice.ConvertProjectionMatrix(projTransform, projTransform);

	RendererViewCreateInformation viewDesc;
	viewDesc.Target.ClearFlags = FBT_COLOR | FBT_DEPTH;
	viewDesc.Target.ClearColor = Color::kBlack;
	viewDesc.Target.ClearDepthValue = 1.0f;
	viewDesc.Target.ClearStencilValue = 0;

	viewDesc.Target.NrmViewRect = Area2(0, 0, 1.0f, 1.0f);
	viewDesc.Target.ViewRect = Area2I(0, 0, texProps.Width, texProps.Height);
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

	viewDesc.StateReduction = mRenderThreadOptions->StateReductionMode;
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
		views[i].UpdateAsyncOperations(); // Note: Needs to happen before any ShouldDraw*() calls, to be consistent
	}

	RendererView* viewPtrs[] = { &views[0], &views[1], &views[2], &views[3], &views[4], &views[5] };

	RendererViewGroup viewGroup(viewPtrs, 6, false, mRenderThreadOptions->ShadowMapSize);
	viewGroup.DetermineVisibility(commandBuffer, sceneInfo);

	FrameInfo frameInfo({ 0.0f, 1.0f / 60.0f, 0 }, false, PerSceneFrameData());
	RenderViews(commandBuffer, renderBeastScene, viewGroup, frameInfo, false);

	// Make sure the render texture is available for reads
	commandBuffer.EndRenderPass();
}

SPtr<RendererScene> RenderBeast::CreateScene()
{
	SPtr<RenderBeastScene> scene = B3DMakeShared<RenderBeastScene>(mOptions);
	scene->SetShared(scene);

	return scene;
}

void RenderBeast::RequestScreenCapture(Camera* camera, TAsyncOp<SPtr<PixelData>> asyncOp)
{
	for (RenderBeastScene* scene : mScenes)
	{
		const SceneInfo& sceneInfo = scene->GetSceneInfo();
		auto it = sceneInfo.CameraToView.find(camera);
		if (it != sceneInfo.CameraToView.end())
		{
			RendererView* view = sceneInfo.Views[it->second];
			view->RequestScreenCapture(std::move(asyncOp));
			return;
		}
	}

	B3D_LOG(Warning, LogRenderer, "RequestCapture: No view found for camera");
	asyncOp.CompleteOperation(nullptr);
}

SPtr<RenderBeast> GetRenderBeast()
{
	return std::static_pointer_cast<RenderBeast>(RendererManager::Instance().GetActive());
}
}} // namespace b3d::render
