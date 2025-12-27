//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DApplication.h"
#include "GUI/B3DGUIManager.h"
#include "2D/B3DSpriteManager.h"
#include "Resources/B3DBuiltinResources.h"
#include "Script/B3DScriptManager.h"
#include "Profiling/B3DProfilingManager.h"
#include "Input/B3DVirtualInput.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneObject.h"
#include "Platform/B3DCursor.h"
#include "CoreObject/B3DRenderThread.h"
#include "FileSystem/B3DFileSystem.h"
#include "Resources/B3DPlainTextImporter.h"
#include "Importer/B3DImporter.h"
#include "GUI/B3DShortcutManager.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Debug/B3DDebugDraw.h"
#include "Platform/B3DPlatform.h"
#include "Resources/B3DEngineShaderIncludeHandler.h"
#include "Resources/B3DResources.h"
#include "B3DEngineConfig.h"
#include "Audio/B3DAudio.h"
#include "GUI/B3DProfilerOverlay.h"
#include "Components/B3DCamera.h"
#include "Scene/B3DSceneInstance.h"
#include "Text/B3DStockIcons.h"
#include "RenderAPI/B3DRenderWindow.h"
#include "Scene/B3DGameObjectManager.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DTime.h"
#include "Input/B3DInput.h"
#include "Managers/B3DMeshManager.h"
#include "Utility/B3DDeferredCallManager.h"
#include "Localization/B3DStringTableManager.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Threading/B3DThreadPool.h"
#include "Profiling/B3DRenderStats.h"
#include "Utility/B3DMessageHandler.h"
#include "Managers/B3DResourceListenerManager.h"
#include "Material/B3DShaderManager.h"
#include "Physics/B3DPhysicsManager.h"
#include "Audio/B3DAudioManager.h"
#include "Managers/B3DGpuBackendManager.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Material/B3DShaderCompiler.h"
#include "Particles/B3DParticleScene.h"
#include "Particles/B3DVectorField.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Platform/B3DFolderMonitor.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DPrefab.h"
#include "Text/B3DFont.h"
#include "Threading/B3DScheduler.h"
#include "Utility/B3DCommandLine.h"
#include "Utility/B3DDynamicLibrary.h"
#include "Utility/B3DPersistentCache.h"

namespace b3d
{
	B3D_LOG_CATEGORY(RenderThread)
	B3D_LOG_CATEGORY(Renderer)
	B3D_LOG_CATEGORY(Scene)
	B3D_LOG_CATEGORY(Physics)
	B3D_LOG_CATEGORY(Audio)
	B3D_LOG_CATEGORY(Network)
	B3D_LOG_CATEGORY(RenderBackend)
	B3D_LOG_CATEGORY(BSLCompiler)
	B3D_LOG_CATEGORY(Particles)
	B3D_LOG_CATEGORY(Resources)
	B3D_LOG_CATEGORY(FBXImporter)
	B3D_LOG_CATEGORY(PixelUtility)
	B3D_LOG_CATEGORY(Texture)
	B3D_LOG_CATEGORY(Mesh)
	B3D_LOG_CATEGORY(GUI)
	B3D_LOG_CATEGORY(Profiler)
	B3D_LOG_CATEGORY(Material)
	B3D_LOG_CATEGORY(FreeImageImporter)
	B3D_LOG_CATEGORY(Script)
	B3D_LOG_CATEGORY(Importer)
	B3D_LOG_CATEGORY(LogInput)
} // namespace b3d

using namespace b3d;

Application::Application(const ApplicationCreateInformation& createInformation)
	: mPrimaryWindow(nullptr), mStartUpDesc(createInformation), mFrameRenderingFinishedSignal(SignalEvent::Mode::AutomaticallyReset, true), mSimThreadId(B3D_CURRENT_THREAD_ID), mRunMainLoop(false), mMainThreadScheduler(SchedulerCreateInformation())
{
	// Ensure all errors are reported properly
	CrashHandler::StartUp(createInformation.CrashHandling);
	if(createInformation.LogCallback)
		GetDebug().SetLogCallback(createInformation.LogCallback);
}

Application::Application(VideoMode videoMode, const String& title, bool fullscreen)
	: Application(BuildCreateInformation(videoMode, title, fullscreen))
{ }

Application::~Application()
{
	// Cleanup any new objects queued for destruction by unloaded scripts
	CoreObjectManager::Instance().SyncToRenderThread(true);
	GetRenderThread().PostCommand([]{}, "SyncToRenderThread() before shutdown", true);

	Cursor::ShutDown();

	GUIManager::ShutDown();
	SpriteManager::ShutDown();
	StockIcons::ShutDown();
	BuiltinResources::ShutDown();
	RendererMaterialManager::ShutDown();
	VirtualInput::ShutDown();

	mPrimaryWindow->Destroy();
	mPrimaryWindow = nullptr;

	FolderMonitorManager::ShutDown();

	Importer::ShutDown();
	MeshManager::ShutDown();
	GpuProfiler::ShutDown();

	PrefabManager::ShutDown();

	Input::ShutDown();

	FontAtlasRenderer::ShutDown();
	render::GpuUniformBufferManager::ShutDown();
	StringTableManager::ShutDown();
	Resources::ShutDown();
	PackageManager::ShutDown();
	GameObjectManager::ShutDown();

	// Audio manager must be released before the ResourceListenerManager, as any one-shot audio sources need to be
	// destroyed since they implement the IResourceListener interface
	AudioManager::ShutDown();
	ResourceListenerManager::ShutDown();

	// This must be done after all resources are released since it will unload the physics plugin, and some resources
	// might be instances of types from that plugin.
	PhysicsManager::ShutDown();

	RendererManager::ShutDown();

	// All CoreObject related modules should be shut down now. They have likely queued CoreObjects for destruction, so
	// we need to wait for those objects to get destroyed before continuing.
	CoreObjectManager::Instance().SyncToRenderThread(true);
	GetRenderThread().PostCommand([] {}, "SyncToRenderThread before shutdown", true);

	UnloadPlugin(mStartUpDesc.Renderer);

	mPrimaryGpu = nullptr;
	GpuBackendManager::ShutDown();

	CoreObjectManager::ShutDown(); // Must shut down before DynLibManager to ensure all objects are destroyed before unloading their libraries

	UnloadAllPlugins();
	DynamicLibraryManager::ShutDown();

	Time::ShutDown();
	DeferredCallManager::ShutDown();
	RenderThread::ShutDown();
	RenderStats::ShutDown();
	ProfilingManager::ShutDown();
	ProfilerCPU::ShutDown();
	MessageHandler::ShutDown();
	ShaderManager::ShutDown();
	ShaderCompilers::ShutDown();

	mTaskScheduler = nullptr;
	Scheduler::UnbindFromCurrentThread();

	ThreadPool::ShutDown();
	MemStack::EndThread();
	Platform::ShutDownInternal();

	CrashHandler::ShutDown();
}

void Application::OnStartUp()
{
	Platform::StartUpInternal();
	MemStack::BeginThread();
	ThreadPool::StartUp<TThreadPool<ThreadDefaultPolicy>>((Thread::GetLogicalCoreCount()));

	mMainThreadScheduler.BindToCurrentThread();

	SchedulerCreateInformation schedulerCreateInformation;
	schedulerCreateInformation.InternalWorkerThreadCount = (u32)Math::Max(1, (i32)Thread::GetLogicalCoreCount() - 2); // Reserve two threads for main + render thread
	schedulerCreateInformation.AffinityPolicy = B3DMakeShared<AnyOfThreadAffinityPolicy>(ThreadCoreMask::CreateAnyThreadMask()); // TODO - Mask out main + render threads

	mTaskScheduler = B3DMakeShared<Scheduler>(schedulerCreateInformation);

	mApplicationCache = PersistentCache::Create();
	mApplicationCache->Initialize(FileSystem::GetApplicationDataFolder());

	ShaderCompilers::StartUp();
	ShaderManager::StartUp(GetShaderIncludeHandler());
	MessageHandler::StartUp();
	ProfilerCPU::StartUp();
	ProfilingManager::StartUp();
	RenderStats::StartUp();
	RenderThread::StartUp();
	StringTableManager::StartUp();
	DeferredCallManager::StartUp();
	Time::StartUp();

	// Parse command-line parameters for fixed timestep and exit after N frames
	const i32 fixedTimestepFPS = CommandLine::GetParameterValueAsInt("FixedTimestep", 0);
	if(fixedTimestepFPS > 0)
		GetTime().SetFixedDeltaTime(1.0f / (float)fixedTimestepFPS);

	mExitAfterNFrames = (u32)CommandLine::GetParameterValueAsInt("ExitAfterNFrames", 0);

#if B3D_ENABLE_TESTS
	const SnapshotTestConfiguration snapshotTestConfiguration = SnapshotTestConfiguration::ParseFromCommandLine();

	if(snapshotTestConfiguration.Enabled)
		mSnapshotTestRunner = B3DMakeUnique<SnapshotTestRunner>(snapshotTestConfiguration);
#endif

	DynamicLibraryManager::StartUp();
	CoreObjectManager::StartUp();
	GameObjectManager::StartUp();
	PackageManager::StartUp();
	Resources::StartUp();
	ResourceListenerManager::StartUp();

	GpuBackendManager::StartUp();
	GpuBackendManager::Instance().Initialize(mStartUpDesc.RenderApi);

	mPrimaryGpu = GpuBackend::Instance().GetDevice(0);
	mPrimaryGpu->Initialize();

	mPrimaryWindow = RenderWindow::Create(mStartUpDesc.PrimaryWindow, nullptr);

	render::GpuUniformBufferManager::StartUp();
	FontAtlasRenderer::StartUp();
	Input::StartUp();
	RendererManager::StartUp();

	LoadPlugin(mStartUpDesc.Renderer);

	// Must be initialized before the scene manager, as game scene creation triggers physics scene creation
	PhysicsManager::StartUp(mStartUpDesc.Physics, mStartUpDesc.PhysicsCooking);
	PrefabManager::StartUp();
	RendererManager::Instance().SetActive(mStartUpDesc.Renderer);
	StartUpRenderer();

	GpuProfiler::StartUp();
	MeshManager::StartUp();
	Importer::StartUp();
	AudioManager::StartUp(mStartUpDesc.Audio);
	FolderMonitorManager::StartUp();

	for(auto& importerName : mStartUpDesc.Importers)
		LoadPlugin(importerName);

	// Built-in importers
	FGAImporter* fgaImporter = B3DNew<FGAImporter>();
	Importer::Instance().RegisterAssetImporterInternal(fgaImporter);

	PlainTextImporter* importer = B3DNew<PlainTextImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	VirtualInput::StartUp();
	StockIcons::StartUp();
	BuiltinResources::StartUp();
	RendererMaterialManager::StartUp();
	RendererManager::Instance().Initialize(GetPrimaryGpuDevice());
	SceneManager::StartUp(); // Must be initialized after the renderer
	SpriteManager::StartUp();
	GUIManager::StartUp();
	ShortcutManager::StartUp();

	Cursor::StartUp();
	Cursor::Instance().SetCursor(CursorType::Arrow);
	Platform::SetIcon(BuiltinResources::Instance().GetFrameworkIcon());

	SceneManager::Instance().SetMainCameraRenderTarget(GetPrimaryWindow());
	DebugDraw::StartUp();

	StartUpScriptManager();
}

void Application::OnShutDown()
{
	// Need to clear all objects before I unload any plugins, as they
	// could have allocated parts or all of those objects.
	const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>> allScenes = GetSceneManager().GetAllScenes();
	for(const auto& entry : allScenes)
	{
		const SPtr<SceneInstance>& scene = entry.second.lock();
		if(scene == nullptr)
			continue;

		scene->Clear(true);
	}

	// Resources too (Prefabs especially, since they hold the same data as a scene)
	Resources::Instance().UnloadAll();

	// Shut down before script manager as scripts could have registered shortcut callbacks
	ShortcutManager::ShutDown();

	ScriptManager::ShutDown();
	DebugDraw::ShutDown();

	SceneManager::ShutDown(); // Needs to trigger after ScriptManager, as script objects may still be referencing scene objects
}

void Application::RunMainLoop()
{
	BeginMainLoop();

	while(IsMainLoopRunning())
	{
		// Limit FPS if needed
		if(mFrameStep > 0)
		{
			u64 currentTime = GetTime().GetTimePrecise();
			u64 nextFrameTime = mLastFrameTime + mFrameStep;
			while(nextFrameTime > currentTime)
			{
				u32 waitTime = (u32)(nextFrameTime - currentTime);

				// If waiting for longer, sleep
				if(waitTime >= 2000)
				{
					Platform::Sleep(waitTime / 1000);
					currentTime = GetTime().GetTimePrecise();
				}
				else
				{
					// Otherwise we just spin, sleep timer granularity is too low and we might end up wasting a
					// millisecond otherwise.
					// Note: For mobiles where power might be more important than input latency, consider using sleep.
					while(nextFrameTime > currentTime)
						currentTime = GetTime().GetTimePrecise();
				}
			}

			mLastFrameTime = currentTime;
		}

		RunMainLoopFrame();
	}

	EndMainLoop();
}

void Application::StopMainLoop()
{
	mRunMainLoop = false; // No sync primitives needed, in that rare case of
	// a race condition we might run the loop one extra iteration which is acceptable
}

void Application::BeginMainLoop()
{
	mRunMainLoop = true;
}

void Application::EndMainLoop()
{
#if B3D_ENABLE_TESTS
	// Finalize test runner before waiting for frame completion
	if(mSnapshotTestRunner != nullptr)
		mSnapshotTestRunner->Finalize();
#endif

	WaitUntilFrameFinished();
}

void Application::RunMainLoopFrame()
{
	GetProfilerCPU().BeginThread("Main");

	Platform::UpdateInternal();
	DeferredCallManager::Instance().UpdateInternal();
	GetTime().Update();
	{
		const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>>& allScenes = GetSceneManager().GetAllScenes();

		// Note: Can we do this as part of SceneInstance::Update? Would clean up this bit of code
		for(const auto& entry : allScenes)
		{
			const SPtr<SceneInstance>& scene = entry.second.lock();
			scene->GetTime().Update();
		}
	}
	GetInput().UpdateInternal();
	// RenderWindowManager::update needs to happen after Input::update and before Input::_triggerCallbacks,
	// so that all input is properly captured in case there is a focus change, and so that
	// focus change is registered before input events are sent out (mouse press can result in code
	// checking if a window is in focus, so it has to be up to date)
	RenderWindowManager::Instance().Update();
	GetInput().TriggerCallbacksInternal();
	GetDebug().TriggerCallbacksInternal();
	FolderMonitorManager::Instance().Update();

	PreUpdate();

	{
		// Purposefully make a copy, assume components Updates can modify the active scene list
		const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>> allScenes = GetSceneManager().GetAllScenes();
		for(const auto& entry : allScenes)
		{
			const SPtr<SceneInstance>& scene = entry.second.lock();
			if(scene == nullptr)
				continue;

			scene->FixedUpdate();
			PROFILE_CALL(scene->Update(), "Scene update");
		}
	}

	GetAudio().Update();

	// Update plugins
	for(const auto& pair : mLoadedPlugins)
	{
		if(pair.second.UpdateCallback != nullptr)
			pair.second.UpdateCallback();
	}

	PostUpdate();

	mApplicationCache->Update();

	PerFrameData perFrameData;

	// Update particles and animation
	{
		const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>>& allScenes = GetSceneManager().GetAllScenes();

		// Note: Can we do this as part of SceneInstance::Update? Would clean up this bit of code
		for(const auto& entry : allScenes)
		{
			const SPtr<SceneInstance>& scene = entry.second.lock();
			render::RendererScene* const rendererSceneProxy = B3DGetRenderProxy(scene->GetRendererScene()).get();

			// Evaluate animation after scene and plugin updates because the renderer will just now be displaying the
			// animation we sent on the previous frame, and we want the scene information to match to what is displayed.

			PerSceneFrameData perSceneFrameData;
			perSceneFrameData.Animation = scene->GetAnimationScene()->Update(mStartUpDesc.AsyncAnimation);
			perSceneFrameData.Particles = scene->GetParticleScene()->Update(*perSceneFrameData.Animation);

			perFrameData.PerSceneData[rendererSceneProxy] = std::move(perSceneFrameData);
		}
	}

	// Send out resource events in case any were loaded/destroyed/modified
	ResourceListenerManager::Instance().Update();

	// Trigger any renderer task callbacks (should be done before scene object update, or core sync, so objects have
	// a chance to respond to the callback).
	RendererManager::Instance().GetActive()->Update();

	// Render and main thread run in lockstep. This will result in a larger input latency than if I was
	// running just a single thread. Latency becomes worse if the render thread takes longer than main
	// thread, in which case main thread needs to wait. Optimal solution would be to get an average
	// difference between main/render thread and start the main thread a bit later so they finish at nearly the same time.
	WaitUntilFrameFinished();

	GetRenderThread().PostCommand([this] { BeginRenderThreadProfiling(); }, "BeginRenderThreadProfiling");

	PROFILE_CALL(RendererManager::Instance().GetActive()->RenderAll(perFrameData), "Render");

	GetRenderThread().PostCommand([this] { FrameRenderingFinishedCallback(); }, "FrameRenderingFinishedCallback");
	GetRenderThread().PostCommand([this] { EndRenderThreadProfiling(); }, "EndRenderThreadProfiling");

	GetProfilerCPU().EndThread();
	GetProfiler().UpdateInternal();

	// Check if we should exit after N frames
	if(mExitAfterNFrames > 0 && GetTime().GetCurrentFrameIndex() >= mExitAfterNFrames)
		StopMainLoop();
}

void Application::WaitUntilFrameFinished()
{
	mFrameRenderingFinishedSignal.Wait();
}

void Application::SetFpsLimit(u32 limit)
{
	if(limit > 0)
		mFrameStep = (u64)1000000 / limit;
	else
		mFrameStep = 0;
}

void Application::PreUpdate()
{
	VirtualInput::Instance().UpdateInternal();

	if(mProfilerOverlay)
		mProfilerOverlay->Update();
}

void Application::PostUpdate()
{
	UpdateScriptManager();

	PROFILE_CALL(GUIManager::Instance().Update(), "GUI");
	DebugDraw::Instance().UpdateInternal();

#if B3D_ENABLE_TESTS
	if(mSnapshotTestRunner != nullptr)
		mSnapshotTestRunner->Update();
#endif
}

void Application::ShowProfilerOverlay(ProfilerOverlayType type, const HCamera& camera)
{
	const SPtr<SceneInstance>& scene = camera ? camera->SceneObject()->GetScene() : GetSceneManager().GetMainScene();
	const HCamera& overlayCamera = camera ? camera : scene->GetMainCamera();
	if(!overlayCamera.IsValid())
		return;

	if(!mProfilerOverlay)
		mProfilerOverlay = B3DMakeShared<ProfilerOverlay>(overlayCamera);
	else
		mProfilerOverlay->SetTarget(overlayCamera);

	mProfilerOverlay->Show(type);
}

void Application::HideProfilerOverlay()
{
	if(mProfilerOverlay)
		mProfilerOverlay->Hide();

	mProfilerOverlay = nullptr;
}

void Application::StartUpRenderer()
{
	// Do nothing, we activate the renderer at a later stage
}

void Application::StartUpScriptManager()
{
	ScriptManager::StartUp();
}

void Application::UpdateScriptManager()
{
	ScriptManager::Instance().Update();
}

void Application::NotifyQuitRequested()
{
	StopMainLoop();
}

void Application::FrameRenderingFinishedCallback()
{
	mFrameRenderingFinishedSignal.Signal();
}

void Application::BeginRenderThreadProfiling()
{
	GetProfilerCPU().BeginThread("Core");
}

void Application::EndRenderThreadProfiling()
{
	GpuProfiler::Instance().Update();

	GetProfilerCPU().EndThread();
	GetProfiler().UpdateRenderThreadInternal();
}

void* Application::LoadPlugin(const String& pluginName, DynamicLibrary** outLibrary, void* passThrough)
{
	B3D_ASSERT(mLoadedPlugins.find(pluginName) == mLoadedPlugins.end());

	DynamicLibrary* loadedLibrary = GetDynamicLibraryManager().Load(pluginName);
	if(outLibrary != nullptr)
		*outLibrary = loadedLibrary;

	void* returnValue = nullptr;
	if(loadedLibrary != nullptr)
	{
		if(passThrough == nullptr)
		{
			typedef void* (*LoadPluginFunctionPointer)();

			LoadPluginFunctionPointer loadFunction = (LoadPluginFunctionPointer)loadedLibrary->GetSymbol("LoadPlugin");

			if(loadFunction != nullptr)
				returnValue = loadFunction();
		}
		else
		{
			typedef void* (*LoadPluginFunctionPointer)(void*);

			LoadPluginFunctionPointer loadFunction = (LoadPluginFunctionPointer)loadedLibrary->GetSymbol("LoadPlugin");

			if(loadFunction != nullptr)
				returnValue = loadFunction(passThrough);
		}

		LoadedPlugin loadedPlugin;
		loadedPlugin.Library = loadedLibrary;
		loadedPlugin.UpdateCallback = (UpdatePluginFunctionPointer)loadedLibrary->GetSymbol("UpdatePlugin");
		loadedPlugin.UnloadCallback = (UnloadPluginFunctionPointer)loadedLibrary->GetSymbol("UnloadPlugin");

		mLoadedPlugins[pluginName] = loadedPlugin;
	}

	return returnValue;
}

void Application::UnloadPlugin(const String& pluginName)
{
	auto found = mLoadedPlugins.find(pluginName);
	if(found == mLoadedPlugins.end())
		return;

	if(found->second.UnloadCallback != nullptr)
		found->second.UnloadCallback();

	GetDynamicLibraryManager().Unload(found->second.Library);
	mLoadedPlugins.erase(found);
}

void Application::UnloadAllPlugins()
{
	for(auto& entryPair : mLoadedPlugins)
	{
		if(entryPair.second.UnloadCallback != nullptr)
			entryPair.second.UnloadCallback();

		GetDynamicLibraryManager().Unload(entryPair.second.Library);
	}

	mLoadedPlugins.clear();
}

ApplicationCreateInformation Application::BuildCreateInformation(VideoMode videoMode, const String& title, bool fullscreen)
{
	ApplicationCreateInformation desc;

	// Set up default plugins
	desc.RenderApi = B3D_RENDER_BACKEND;
	desc.Renderer = B3D_RENDERER;
	desc.Audio = B3D_AUDIO_BACKEND;
	desc.Physics = B3D_PHYSICS_BACKEND;

	desc.Importers.push_back("bsfFreeImgImporter");
	desc.Importers.push_back("bsfFBXImporter");
	desc.Importers.push_back("bsfFontImporter");
	desc.Importers.push_back("bsfSL");

	desc.PrimaryWindow.VideoMode = videoMode;
	desc.PrimaryWindow.Fullscreen = fullscreen;
	desc.PrimaryWindow.Title = title;

	return desc;
}

SPtr<IShaderIncludeHandler> Application::GetShaderIncludeHandler() const
{
	return B3DMakeShared<EngineShaderIncludeHandler>();
}

namespace b3d
{
Application& GetApplication()
{
	return static_cast<Application&>(Application::Instance());
}
} // namespace b3d
