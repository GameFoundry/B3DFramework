//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCoreApplication.h"

#include "Platform/BsPlatform.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Math/BsVector2.h"
#include "CoreObject/BsCoreObjectManager.h"
#include "Scene/BsGameObjectManager.h"
#include "Utility/BsDynamicLibrary.h"
#include "Utility/BsDynamicLibraryManager.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneInstance.h"
#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Input/BsInput.h"
#include "Renderer/BsRendererManager.h"
#include "Managers/BsMeshManager.h"
#include "Managers/BsRenderWindowManager.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsDeferredCallManager.h"
#include "CoreObject/BsRenderThread.h"
#include "Localization/BsStringTableManager.h"
#include "Profiling/BsProfilingManager.h"
#include "Profiling/BsProfilerCPU.h"
#include "Profiling/BsProfilerGPU.h"
#include "Threading/BsThreadPool.h"
#include "Profiling/BsRenderStats.h"
#include "Utility/BsMessageHandler.h"
#include "Managers/BsResourceListenerManager.h"
#include "Material/BsShaderManager.h"
#include "Physics/BsPhysicsManager.h"
#include "Physics/BsPhysics.h"
#include "Audio/BsAudioManager.h"
#include "Audio/BsAudio.h"
#include "Animation/BsAnimationScene.h"
#include "FileSystem/BsFileSystem.h"
#include "Managers/BsGpuBackendManager.h"
#include "Material/BsShaderCompiler.h"
#include "Renderer/BsGpuDataParameterBlock.h"
#include "Particles/BsParticleScene.h"
#include "Particles/BsVectorField.h"
#include "Platform/BsFolderMonitor.h"
#include "RenderAPI/BsGpuBackend.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Renderer/BsRendererScene.h"
#include "Scene/BsPrefab.h"
#include "Text/BsFont.h"
#include "Threading/BsScheduler.h"

namespace b3d
{
	B3D_LOG_CATEGORY(RenderThread)
	B3D_LOG_CATEGORY(Renderer)
	B3D_LOG_CATEGORY(Scene)
	B3D_LOG_CATEGORY(Physics)
	B3D_LOG_CATEGORY(Audio)
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
} // namespace b3d

using namespace b3d;

CoreApplication::CoreApplication(START_UP_DESC desc)
	: mPrimaryWindow(nullptr), mStartUpDesc(desc), mFrameRenderingFinishedSignal(SignalEvent::Mode::AutomaticallyReset, true), mSimThreadId(B3D_CURRENT_THREAD_ID), mRunMainLoop(false), mMainThreadScheduler(SchedulerCreateInformation())
{
	// Ensure all errors are reported properly
	CrashHandler::StartUp(desc.CrashHandling);
	if(desc.LogCallback)
		GetDebug().SetLogCallback(desc.LogCallback);
}

CoreApplication::~CoreApplication()
{
	mPrimaryWindow->Destroy();
	mPrimaryWindow = nullptr;

	FolderMonitorManager::ShutDown();

	Importer::ShutDown();
	MeshManager::ShutDown();
	ProfilerGPU::ShutDown();

	PrefabManager::ShutDown();

	Input::ShutDown();

	FontAtlasRenderer::ShutDown();
	render::GpuDataParameterBlockManager::ShutDown();
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

void CoreApplication::OnStartUp()
{
	Platform::StartUpInternal();
	MemStack::BeginThread();
	ThreadPool::StartUp<TThreadPool<ThreadDefaultPolicy>>((Thread::GetLogicalCoreCount()));

	mMainThreadScheduler.BindToCurrentThread();

	SchedulerCreateInformation schedulerCreateInformation;
	schedulerCreateInformation.WorkerThreadCount = (u32)Math::Max(1, (i32)Thread::GetLogicalCoreCount() - 2); // Reserve two threads for main + render thread
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

	mPrimaryWindow = RenderWindow::Create(mStartUpDesc.PrimaryWindowDesc, nullptr);

	render::GpuDataParameterBlockManager::StartUp();
	FontAtlasRenderer::StartUp();
	Input::StartUp();
	RendererManager::StartUp();

	LoadPlugin(mStartUpDesc.Renderer);

	// Must be initialized before the scene manager, as game scene creation triggers physics scene creation
	PhysicsManager::StartUp(mStartUpDesc.Physics, mStartUpDesc.PhysicsCooking);
	PrefabManager::StartUp();
	RendererManager::Instance().SetActive(mStartUpDesc.Renderer);
	StartUpRenderer();

	ProfilerGPU::StartUp();
	MeshManager::StartUp();
	Importer::StartUp();
	AudioManager::StartUp(mStartUpDesc.Audio);
	FolderMonitorManager::StartUp();

	for(auto& importerName : mStartUpDesc.Importers)
		LoadPlugin(importerName);

	// Built-in importers
	FGAImporter* fgaImporter = B3DNew<FGAImporter>();
	Importer::Instance().RegisterAssetImporterInternal(fgaImporter);
}

void CoreApplication::RunMainLoop()
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

void CoreApplication::BeginMainLoop()
{
	mRunMainLoop = true;
}

void CoreApplication::EndMainLoop()
{
	WaitUntilFrameFinished();
}

void CoreApplication::RunMainLoopFrame()
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

	GetSceneManager().UpdateLinkedSceneActorTransforms();

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
}

void CoreApplication::WaitUntilFrameFinished()
{
	mFrameRenderingFinishedSignal.Wait();
}

void CoreApplication::PreUpdate()
{
	// Do nothing
}

void CoreApplication::PostUpdate()
{
	// Do nothing
}

void CoreApplication::StopMainLoop()
{
	mRunMainLoop = false; // No sync primitives needed, in that rare case of
	// a race condition we might run the loop one extra iteration which is acceptable
}

void CoreApplication::QuitRequested()
{
	StopMainLoop();
}

void CoreApplication::SetFpsLimit(u32 limit)
{
	if(limit > 0)
		mFrameStep = (u64)1000000 / limit;
	else
		mFrameStep = 0;
}

void CoreApplication::FrameRenderingFinishedCallback()
{
	mFrameRenderingFinishedSignal.Signal();
}

void CoreApplication::BeginRenderThreadProfiling()
{
	GetProfilerCPU().BeginThread("Core");
}

void CoreApplication::EndRenderThreadProfiling()
{
	ProfilerGPU::Instance().UpdateInternal();

	GetProfilerCPU().EndThread();
	GetProfiler().UpdateRenderThreadInternal();
}

void* CoreApplication::LoadPlugin(const String& pluginName, DynamicLibrary** outLibrary, void* passThrough)
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

void CoreApplication::UnloadPlugin(const String& pluginName)
{
	auto found = mLoadedPlugins.find(pluginName);
	if(found == mLoadedPlugins.end())
		return;

	if(found->second.UnloadCallback != nullptr)
		found->second.UnloadCallback();

	GetDynamicLibraryManager().Unload(found->second.Library);
	mLoadedPlugins.erase(found);
}

void CoreApplication::UnloadAllPlugins()
{
	for(auto& entryPair : mLoadedPlugins)
	{
		if(entryPair.second.UnloadCallback != nullptr)
			entryPair.second.UnloadCallback();

		GetDynamicLibraryManager().Unload(entryPair.second.Library);
	}

	mLoadedPlugins.clear();
}

SPtr<IShaderIncludeHandler> CoreApplication::GetShaderIncludeHandler() const
{
	return B3DMakeShared<DefaultShaderIncludeHandler>();
}

namespace b3d
{
CoreApplication& GetCoreApplication()
{
	return CoreApplication::Instance();
}
} // namespace
