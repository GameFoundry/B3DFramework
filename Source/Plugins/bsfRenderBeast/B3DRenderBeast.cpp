//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeast.h"
#include "CoreObject/B3DRenderThread.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Renderer/B3DRendererSyncManager.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DViewport.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "GpuBackend/B3DGpuBuffer.h"
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
#include "Renderer/B3DRendererManager.h"
#include "Shading/B3DShadowRendering.h"
#include "Shading/B3DStandardDeferred.h"
#include "Shading/B3DTiledDeferred.h"
#include "B3DRenderBeastOptions.h"
#include "B3DRenderBeastIBLUtility.h"
#include "B3DRenderCompositor.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Resources/B3DBuiltinResources.h"
#include "RenderState/B3DRenderableRenderState.h"
#include "RenderState/B3DDecalRenderState.h"
#include "RenderState/B3DParticleRenderState.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Math/B3DMath.h"

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

void RenderBeast::Activate()
{
	LoadedRendererTextures textures;
	HTexture bokehFlare = GetBuiltinResources().GetTexture(BuiltinTexture::BokehFlare);
	textures.BokehFlare = B3DGetRenderProxy(bokehFlare);

	GetRenderThread().PostCommand([this, textures]() { ActivateOnRenderThread(textures); }, "RenderBeast::ActivateOnRenderThread");
}

void RenderBeast::Destroy()
{
	Renderer::Destroy();

	GetRenderThread().PostCommand([this]() { DestroyOnRenderThread(); }, "RenderBeast::DestroyOnRenderThread", true);
}

void RenderBeast::ActivateOnRenderThread(const LoadedRendererTextures& rendererTextures)
{
	Renderer::ActivateOnRenderThread();

	const GpuDeviceCapabilities& caps = mDevice->GetCapabilities();

	if(
		!caps.HasCapability(RSC_COMPUTE_PROGRAM) ||
		!caps.HasCapability(RSC_LOAD_STORE) ||
		!caps.HasCapability(RSC_TEXTURE_VIEWS))
	{
		B3D_LOG(Fatal, LogRenderer, "RenderBeast requires a device with compute programs, load-store textures and texture views.");
	}

	// Create per-object parameter set layouts
	{
		// PerObject uniform buffer info (shared by both layouts)
		GpuUniformBufferInformation perObjectInfo;
		perObjectInfo.Name = "PerObject";
		perObjectInfo.Set = GpuPipelineSet::kPerObjectSetIndex;
		perObjectInfo.Slot = mDevice->GetParameterSlot(ShaderRegisterClass::ConstantBuffer, 0);
		perObjectInfo.Size = Math::CeilToMultiple(gPerObjectUniformDefinition.GetSize() / 4u, 4u);
		perObjectInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
		perObjectInfo.IsShareable = true;
		perObjectInfo.UsesDynamicOffset = true;

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
			decalInfo.Set = GpuPipelineSet::kPerObjectSetIndex;
			decalInfo.Slot = mDevice->GetParameterSlot(ShaderRegisterClass::ConstantBuffer, 1);
			decalInfo.Size = Math::CeilToMultiple(gDecalUniformDefinition.GetSize() / 4u, 4u);
			decalInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
			decalInfo.IsShareable = true;
			decalInfo.UsesDynamicOffset = true;
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
			gpuParticlesInfo.Set = GpuPipelineSet::kPerObjectSetIndex;
			gpuParticlesInfo.Slot = mDevice->GetParameterSlot(ShaderRegisterClass::ConstantBuffer, 1);
			gpuParticlesInfo.Size = Math::CeilToMultiple(gGpuParticlesUniformDefinition.GetSize() / 4u, 4u);
			gpuParticlesInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
			gpuParticlesInfo.IsShareable = true;
			gpuParticlesInfo.UsesDynamicOffset = true;
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

	mSurfaceShaderParameterSetInfo.Initialize(*mDevice);

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
	GpuSort::ShutDown();
	RendererUtility::ShutDown();

	Renderer::DestroyOnRenderThread();
}

void RenderBeast::NotifySceneCreated(const TShared<RenderBeastScene>& scene)
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

void RenderBeast::SetOptions(const TShared<RendererOptions>& options)
{
	mOptions = std::static_pointer_cast<RenderBeastOptions>(options);
	mOptionsDirty = true;
}

TShared<RendererOptions> RenderBeast::GetOptions() const
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

void RenderBeast::ProcessTasks(bool forceAll, u64 upToFrame)
{
	// Tasks may draw objects whose uniform buffers were updated during sync
	for(RenderBeastScene* scene : mScenes)
		scene->GetUniformBufferPools().Flush();

	Renderer::ProcessTasks(forceAll, upToFrame);
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

	for(const TShared<RenderWindow>& window : mWindowsToPresent)
		mDevice->PresentRenderWindow(window);

	mWindowsToPresent.clear();

	EndFrame();

	GetProfilerCPU().EndSample("Render");

	if(mIsGPUCommandCaptureRequested)
	{
		mDevice->WaitUntilIdle();
		GpuBackend::Instance().StopGPUCommandCapture();

		mIsGPUCommandCaptureRequested = false;
	}

	GpuUniformBufferManager::Instance().AdvanceFrame();
	mRendererExtensionsDirty = false;
}

bool RenderBeast::RenderScene(RenderBeastScene& scene, const FrameInfo& frameInfo)
{
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

	scene.ResetRenderableReady();

	BeginFrame();

	if (mIsGPUCommandCaptureRequested)
		GpuBackend::Instance().StartGPUCommandCapture();

	// Update per-frame data for all renderable objects
	RenderableObjectStorage& renderableStorage = scene.GetRenderableStorage();
	for(u32 i = 0; i < renderableStorage.GetRenderableCount(); i++)
		renderableStorage.PrepareRenderable(i, frameInfo);

	scene.PrepareParticleSystems(frameInfo);

	for(u32 i = 0; i < scene.GetDecalCount(); i++)
		scene.PrepareDecal(i, frameInfo);

	// Record the uniform buffer updates of sync and preparation before anything draws
	scene.GetUniformBufferPools().Flush();

	scene.SortRenderTargets();

	bool anythingDrawnForScene = false;
	for(auto& rtInfo : scene.GetRenderTargets())
	{
		Vector<RendererView*> views;
		TShared<RenderTarget> target = rtInfo.Target;
		const Vector<Camera*>& cameras = rtInfo.Cameras;

		const bool isWindow = target->GetProperties().IsWindow;
		TShared<RenderWindow> window;
		if(isWindow)
			window = std::static_pointer_cast<RenderWindow>(rtInfo.Target);

		const bool renderTargetNeedsRedraw = window != nullptr && window->IsRedrawRequested();
		const bool screenCaptureRequested = window != nullptr && window->IsCaptureRequested();
		const bool forceRendering = mIsGPUCommandCaptureRequested || renderTargetNeedsRedraw || screenCaptureRequested;

		const u32 cameraCount = (u32)cameras.size();
		for(u32 i = 0; i < cameraCount; i++)
		{
			RendererView* viewInfo = scene.TryGetView(cameras[i]);

			if(forceRendering)
				viewInfo->NotifyNeedsRedraw();

			viewInfo->UpdateAsyncOperations(); // Note: Needs to happen before any ShouldDraw*() calls, to be consistent
			views.push_back(viewInfo);
		}

		mMainViewGroup->SetViews(views.data(), (u32)views.size());
		PROFILE_CALL(mMainViewGroup->DetermineVisibility(scene, frameInfo), "Determine visibility")
		scene.PrepareVisibleParticleSystems(mMainViewGroup->GetVisibilityInfo(), frameInfo);

		// Render everything. The graph of the group has executed once this returns.
		const bool anythingDrawnForView = RenderViews(scene, *mMainViewGroup, frameInfo, forceRendering);
		if(anythingDrawnForView)
		{
			// Cameras of other scenes may draw to the window too. Its redraw and capture requests stay pending until it is presented.
			if(isWindow && std::find(mWindowsToPresent.begin(), mWindowsToPresent.end(), window) == mWindowsToPresent.end())
				mWindowsToPresent.push_back(window);

			anythingDrawnForScene = true;
		}
	}

	return anythingDrawnForScene;
}

bool RenderBeast::RenderViews(RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo, bool forceRendering)
{
	// Covers the graph's execution, because pass functions may read data the nodes allocated from the frame allocator
	FrameAllocatorScope frameScope;

	// Named for the profiler overlay, which shows the results of this name
	FrameGraph graph(GetGpuContext(), "RenderScene");

	TInlineArray<RendererView*, 6> drawnViews;
	const bool anythingDrawn = BuildViewGroup(graph, scene, viewGroup, frameInfo, forceRendering, drawnViews);

	if(!drawnViews.Empty())
	{
		graph.Compile();
		graph.Execute();
	}

	// After the graph's execution, because the pass functions read the render queues this clears
	for(RendererView* view : drawnViews)
		view->EndFrame();

	return anythingDrawn;
}

bool RenderBeast::BuildViewGroup(FrameGraph& graph, RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo, bool forceRendering,
	TInlineArray<RendererView*, 6>& drawnViews)
{
	bool needs3DRender = false;
	bool needsReflectionProbes = false;
	bool needsLightProbes = false;
	u32 viewCount = viewGroup.GetViewCount();
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		RendererView* view = viewGroup.GetView(viewIndex);

		if(view->ShouldDraw3D())
		{
			needs3DRender = true;

			// Views that capture reflections do not sample the reflection probes
			if(!view->GetProperties().CapturingReflections)
				needsReflectionProbes = true;

			if(view->GetRenderSettings().EnableIndirectLighting)
				needsLightProbes = true;
		}
	}

	if(needs3DRender)
	{
		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		// Update various buffers required by each renderable
		RenderableObjectStorage& renderableStorage = scene.GetRenderableStorage();
		u32 renderableCount = renderableStorage.GetRenderableCount();
		for(u32 i = 0; i < renderableCount; i++)
		{
			if(!visibility.Renderables[i])
				continue;

			renderableStorage.PrepareVisibleRenderable(i, frameInfo);
		}

		// If any reflection probes were updated or added, we need to copy them over in the global reflection probe array. Not in
		// captures: the copy would read the cubemaps of probes that later captures of the same graph render into.
		if(needsReflectionProbes)
			scene.UpdateReflectionProbes(graph);

		// Make sure light probe data is up to date
		if(needsLightProbes)
			scene.UpdateLightProbes(graph);

		// The views of the group share the shadow maps, which are computed from the views as they were before any of them began its frame
		viewGroup.GetShadowRenderer().RenderShadowMaps(graph, scene, viewGroup, frameInfo);
	}

	bool anythingDrawn = false;
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		RendererView* view = viewGroup.GetView(viewIndex);

		if(!view->ShouldDraw())
			continue;

		const RenderSettings& settings = view->GetRenderSettings();
		if(settings.OverlayOnly)
		{
			if(RenderOverlay(graph, scene, *view, frameInfo, forceRendering))
				anythingDrawn = true;
		}
		else
		{
			RenderView(graph, scene, viewGroup, *view, frameInfo);
			anythingDrawn = true;
		}

		drawnViews.Add(view);
	}

	return anythingDrawn;
}

void RenderBeast::RenderView(FrameGraph& graph, RenderBeastScene& scene, RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
{
	GetProfilerCPU().BeginSample("Render view");

	auto& viewProps = view.GetProperties();

	view.BeginFrame(frameInfo);

	graph.ImportRenderTarget(viewProps.Target.Target);

	RenderCompositorNodeInputs inputs(graph, scene, viewGroup, view, *mRenderThreadOptions, frameInfo);

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

	GetProfilerCPU().EndSample("Render view");
}

bool RenderBeast::RenderOverlay(FrameGraph& graph, RenderBeastScene& scene, RendererView& view, const FrameInfo& frameInfo, bool forceRendering)
{
	GetProfilerCPU().BeginSample("Render overlay");

	view.BeginFrame(frameInfo);

	auto& viewProps = view.GetProperties();
	const Camera* camera = view.GetSceneCamera();
	TShared<RenderTarget> target = viewProps.Target.Target;
	TShared<Viewport> viewport = camera->GetViewport();

	graph.ImportRenderTarget(target);

	ClearFlags clearFlags = viewport->GetClearFlags();
	RenderSurfaceMask clearMask = RT_NONE;
	if(clearFlags.IsSet(ClearFlagBits::Depth))
		clearMask |= RT_DEPTH;

	if(clearFlags.IsSet(ClearFlagBits::Stencil))
		clearMask |= RT_STENCIL;

	const bool clearColor = clearFlags.IsSet(ClearFlagBits::Color);
	if(clearMask != RT_NONE || clearColor)
	{
		FrameGraphRasterPass clearPass;
		clearPass.SetRenderTarget(target, FrameGraphAttachmentLoad::Load);

		ClearMaterial* clearMaterial = nullptr;
		TShared<GpuParameterSet> clearSet;
		if(clearColor)
		{
			clearMaterial = ClearMaterial::GetVariation(true);
			clearSet = clearMaterial->Prepare(clearPass, viewport->GetBackgroundColor());
		}

		const Area2 viewArea = viewport->GetArea();
		graph.AddRasterPass("ClearViewport", std::move(clearPass), [clearMaterial, clearSet, clearMask, viewArea](FrameGraphPassContext& context)
		{
			GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
			commandBuffer.SetViewport(viewArea);

			if(clearMask != RT_NONE)
				commandBuffer.ClearViewport(clearMask);

			if(clearMaterial != nullptr)
				clearMaterial->Execute(context, clearSet);
		});
	}

	bool needsRedraw = view.ShouldRedrawOverlay();

	// Trigger overlay callbacks. Extensions that only render into a dirty target render if any other extension or the view redraws.
	TInlineArray<RendererExtension*, 4> overlayExtensions;
	for(RendererExtension* extension : scene.GetCombinedRendererExtensions())
	{
		if(extension->GetLocation() != RenderLocation::Overlay)
			continue;

		const RendererExtensionRequest request = extension->Check(*camera);
		if(request == RendererExtensionRequest::DontRender)
			continue;

		if(request == RendererExtensionRequest::ForceRender || forceRendering)
			needsRedraw = true;

		overlayExtensions.Add(extension);
	}

	if(needsRedraw)
	{
		RendererViewContext context(graph);
		context.Target = target;
		context.Viewport = viewport->GetArea();

		for(RendererExtension* extension : overlayExtensions)
			extension->Render(*camera, context);
	}

	view.ResolveCaptures(graph, target);

	GetProfilerCPU().EndSample("Render overlay");
	return needsRedraw;
}

namespace
{
	/** Views of a cube map capture. They live until the graph that renders them has executed, because its passes read them. */
	struct CubeMapCapture
	{
		RendererView Views[6];
		TUnique<RendererViewGroup> ViewGroup;
		TInlineArray<RendererView*, 6> DrawnViews;

		// Captures render without the frame's animation and particle data
		FrameInfo Frame = FrameInfo({ 0.0f, 1.0f / 60.0f, 0 }, false, PerSceneFrameData());
	};
}

void RenderBeast::CaptureSceneCubeMap(RendererScene& scene, FrameGraph& graph, const TShared<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings)
{
	RenderBeastScene& renderBeastScene = static_cast<RenderBeastScene&>(scene);

	auto& texProps = cubemap->GetProperties();

	Matrix4 projTransform = Matrix4::ProjectionPerspective(Degree(90.0f), 1.0f, 0.05f, 1000.0f);
	ConvexVolume localFrustum(projTransform);

	mDevice->ConvertProjectionMatrix(projTransform, projTransform);

	RendererViewCreateInformation viewDesc;
	viewDesc.Target.ClearFlags = FBT_COLOR | FBT_DEPTH;
	viewDesc.Target.BackgroundColor = Color::kBlack;

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

	TShared<RenderSettings> renderSettings = B3DMakeShared<RenderSettings>();
	renderSettings->EnableHdr = settings.Hdr;
	renderSettings->EnableShadows = true;
	renderSettings->EnableIndirectLighting = false;
	renderSettings->ScreenSpaceReflections.Enabled = false;
	renderSettings->AmbientOcclusion.Enabled = false;

	Matrix4 viewOffsetMat = Matrix4::Translation(-position);

	const TShared<CubeMapCapture> capture = B3DMakeShared<CubeMapCapture>();
	RendererView* const views = capture->Views;

	// Note: We render upside down, then flip the image vertically, which results in a horizontal flip. The horizontal
	// flip is required due to the fact how cubemap faces are defined. Another option would be to change the view
	// orientation matrix, but that also requires a culling mode flip which is inconvenient to do globally.
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

	capture->ViewGroup = B3DMakeUnique<RendererViewGroup>(viewPtrs, 6, false, mRenderThreadOptions->ShadowMapSize);
	capture->ViewGroup->DetermineVisibility(renderBeastScene, capture->Frame);

	// Tasks run before the scene's frame begins, so the scene's per-frame state is still that of the last frame: a per-frame uniform
	// buffer allocated for that frame, and ready flags sized for the renderables it had then. The capture sets up both for itself, and
	// prepares the renderables it sees anew. The scene's frame sets them up again.
	renderBeastScene.SetParamFrameParams(capture->Frame.Timings.Time);
	renderBeastScene.ResetRenderableReady();

	// The faces' render textures share the cubemap, so later passes of the graph that read the cubemap follow the capture
	BuildViewGroup(graph, renderBeastScene, *capture->ViewGroup, capture->Frame, false, capture->DrawnViews);

	// The passes read the views' render queues, so the views live, and their frames last, until the graph has executed
	graph.AddExecuteCallback([capture]()
	{
		for(RendererView* view : capture->DrawnViews)
			view->EndFrame();
	});
}

TShared<RendererScene> RenderBeast::CreateScene()
{
	TShared<RenderBeastScene> scene = B3DMakeShared<RenderBeastScene>(mOptions);
	scene->SetShared(scene);

	return scene;
}

void RenderBeast::RequestViewCapture(Camera* camera, TAsyncOp<TShared<PixelData>> asyncOp)
{
	for (RenderBeastScene* scene : mScenes)
	{
		RendererView* view = scene->TryGetView(camera);
		if(view)
		{
			view->RequestCapture(std::move(asyncOp));
			return;
		}
	}

	B3D_LOG(Warning, LogRenderer, "RequestCapture: No view found for camera");
	asyncOp.CompleteOperation(nullptr);
}

TShared<RenderBeast> GetRenderBeast()
{
	return std::static_pointer_cast<RenderBeast>(RendererManager::Instance().GetActive());
}
}} // namespace b3d::render
