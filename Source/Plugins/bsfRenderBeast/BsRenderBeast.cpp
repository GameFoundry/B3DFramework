//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
#include "Animation/BsAnimationScene.h"
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

	// Ensure profiler methods can be called from start-up methods
	GetProfilerGPU().BeginFrame();

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

	GetRenderThread().PostCommand([this, timings, perFrameData]() { RenderAllScenes(timings, perFrameData); }, "RenderBeast::RenderAll");
}

void RenderBeast::RenderAllScenes(FrameTimings timings, PerFrameData perFrameData)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	GetProfilerGPU().BeginFrame();
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
	}

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

	mRendererExtensionsDirty = false;
}

bool RenderBeast::RenderScene(RenderBeastScene& scene, const FrameInfo& frameInfo)
{
	SPtr<GpuCommandBuffer> commandBuffer = mCommandBufferPool->Create(GpuCommandBufferCreateInformation::Create("Main"));
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
			mDevice->SubmitCommandBuffer(commandBuffer);
			commandBuffer = mCommandBufferPool->Create(GpuCommandBufferCreateInformation::Create("Main"));

			if(isWindow)
				mDevice->PresentRenderWindow(window);

			anythingDrawnForScene = true;
		}
	}

	mDevice->SubmitCommandBuffer(commandBuffer);

	return anythingDrawnForScene;
}

bool RenderBeast::RenderViews(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo, bool forceRender)
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
		const SceneInfo& sceneInfo = scene.GetSceneInfo();
		const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

		// Render shadow maps
		ShadowRendering& shadowRenderer = viewGroup.GetShadowRenderer();
		shadowRenderer.RenderShadowMaps(commandBuffer,scene, viewGroup, frameInfo);

		// Update various buffers required by each renderable
		u32 numRenderables = (u32)sceneInfo.Renderables.size();
		for(u32 i = 0; i < numRenderables; i++)
		{
			if(!visibility.Renderables[i])
				continue;

			scene.PrepareVisibleRenderable(i, frameInfo);
		}
	}

	bool anythingDrawn = false;
	for(u32 i = 0; i < numViews; i++)
	{
		RendererView* view = viewGroup.GetView(i);

		auto viewId = (u64)view;
		const RendererViewTargetInformation& viewTarget = view->GetProperties().Target;
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
			if(RenderOverlay(commandBuffer, scene, *view, frameInfo, forceRender))
				anythingDrawn = true;
		}
		else
		{
			RenderView(commandBuffer, scene, viewGroup, *view, frameInfo);
			anythingDrawn = true;
		}

		GetProfilerGPU().EndView(commandBuffer);
	}

	return anythingDrawn;
}

void RenderBeast::RenderView(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo)
{
	GetProfilerCPU().BeginSample("Render view");

	const SceneInfo& sceneInfo = scene.GetSceneInfo();
	auto& viewProps = view.GetProperties();

	SPtr<GpuBuffer> perCameraBuffer = view.GetPerViewBuffer();
	perCameraBuffer->FlushCache();

	// Make sure light probe data is up to date
	if(view.GetRenderSettings().EnableIndirectLighting)
		scene.UpdateLightProbes(commandBuffer);

	view.BeginFrame(frameInfo);

	RenderCompositorNodeInputs inputs(viewGroup, view, sceneInfo, *mRenderThreadOptions, frameInfo, mFeatureSet);
	inputs.ActiveCommandBuffer = commandBuffer.GetShared();

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
	commandBuffer.SetRenderTarget(nullptr);
}

SPtr<RendererScene> RenderBeast::CreateScene()
{
	SPtr<RenderBeastScene> scene = B3DMakeShared<RenderBeastScene>(mOptions);
	scene->SetShared(scene);

	return scene;
}

SPtr<RenderBeast> GetRenderBeast()
{
	return std::static_pointer_cast<RenderBeast>(RendererManager::Instance().GetActive());
}
}} // namespace b3d::render
