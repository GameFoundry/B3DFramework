//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCoreApplication.h"

#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsRenderAPIManager.h"

#include "Platform/BsPlatform.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Math/BsVector2.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "Scene/BsGameObjectManager.h"
#include "Utility/BsDynamicLibrary.h"
#include "Utility/BsDynamicLibraryManager.h"
#include "Scene/BsSceneManager.h"
#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Input/BsInput.h"
#include "Renderer/BsRendererManager.h"
#include "Managers/BsGpuProgramManager.h"
#include "Managers/BsMeshManager.h"
#include "Managers/BsRenderWindowManager.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsDeferredCallManager.h"
#include "CoreThread/BsCoreThread.h"
#include "Localization/BsStringTableManager.h"
#include "Profiling/BsProfilingManager.h"
#include "Profiling/BsProfilerCPU.h"
#include "Profiling/BsProfilerGPU.h"
#include "Managers/BsQueryManager.h"
#include "Threading/BsThreadPool.h"
#include "Threading/BsTaskScheduler.h"
#include "Profiling/BsRenderStats.h"
#include "Utility/BsMessageHandler.h"
#include "Managers/BsResourceListenerManager.h"
#include "Managers/BsRenderStateManager.h"
#include "Material/BsShaderManager.h"
#include "Physics/BsPhysicsManager.h"
#include "Physics/BsPhysics.h"
#include "Audio/BsAudioManager.h"
#include "Audio/BsAudio.h"
#include "Animation/BsAnimationManager.h"
#include "Material/BsShaderCompiler.h"
#include "Renderer/BsParamBlocks.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsVectorField.h"

using namespace bs;

B3D_LOG_CATEGORY_IMPL(CoreThread)
B3D_LOG_CATEGORY_IMPL(Renderer)
B3D_LOG_CATEGORY_IMPL(Scene)
B3D_LOG_CATEGORY_IMPL(Physics)
B3D_LOG_CATEGORY_IMPL(Audio)
B3D_LOG_CATEGORY_IMPL(RenderBackend)
B3D_LOG_CATEGORY_IMPL(BSLCompiler)
B3D_LOG_CATEGORY_IMPL(Particles)
B3D_LOG_CATEGORY_IMPL(Resources)
B3D_LOG_CATEGORY_IMPL(FBXImporter)
B3D_LOG_CATEGORY_IMPL(PixelUtility)
B3D_LOG_CATEGORY_IMPL(Texture)
B3D_LOG_CATEGORY_IMPL(Mesh)
B3D_LOG_CATEGORY_IMPL(GUI)
B3D_LOG_CATEGORY_IMPL(Profiler)
B3D_LOG_CATEGORY_IMPL(Material)
B3D_LOG_CATEGORY_IMPL(FreeImageImporter)
B3D_LOG_CATEGORY_IMPL(Script)
B3D_LOG_CATEGORY_IMPL(Importer)

CoreApplication::CoreApplication(START_UP_DESC desc)
	: mPrimaryWindow(nullptr), mStartUpDesc(desc), mRendererPlugin(nullptr), mIsFrameRenderingFinished(true), mSimThreadId(B3D_CURRENT_THREAD_ID), mRunMainLoop(false)
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

	Importer::ShutDown();
	MeshManager::ShutDown();
	ProfilerGPU::ShutDown();

	SceneManager::ShutDown();

	Input::ShutDown();

	ct::ParamBlockManager::ShutDown();
	StringTableManager::ShutDown();
	Resources::ShutDown();
	GameObjectManager::ShutDown();

	// Audio manager must be released before the ResourceListenerManager, as any one-shot audio sources need to be
	// destroyed since they implement the IResourceListener interface
	AudioManager::ShutDown();
	ResourceListenerManager::ShutDown();
	RenderStateManager::ShutDown();
	ParticleManager::ShutDown();
	AnimationManager::ShutDown();

	// This must be done after all resources are released since it will unload the physics plugin, and some resources
	// might be instances of types from that plugin.
	PhysicsManager::ShutDown();

	RendererManager::ShutDown();

	// All CoreObject related modules should be shut down now. They have likely queued CoreObjects for destruction, so
	// we need to wait for those objects to get destroyed before continuing.
	CoreObjectManager::Instance().SyncToCore();
	GetCoreThread().Update();
	GetCoreThread().SubmitAll(true);

	UnloadPlugin(mRendererPlugin);

	RenderAPIManager::ShutDown();
	ct::GpuProgramManager::ShutDown();
	GpuProgramManager::ShutDown();

	CoreObjectManager::ShutDown(); // Must shut down before DynLibManager to ensure all objects are destroyed before unloading their libraries
	DynamicLibraryManager::ShutDown();
	Time::ShutDown();
	DeferredCallManager::ShutDown();

	CoreThread::ShutDown();
	RenderStats::ShutDown();
	TaskScheduler::ShutDown();
	ThreadPool::ShutDown();
	ProfilingManager::ShutDown();
	ProfilerCPU::ShutDown();
	MessageHandler::ShutDown();
	ShaderManager::ShutDown();
	ShaderCompilers::ShutDown();

	MemStack::EndThread();
	Platform::ShutDownInternal();

	CrashHandler::ShutDown();
}

void CoreApplication::OnStartUp()
{
	u32 numWorkerThreads = B3D_THREAD_HARDWARE_CONCURRENCY - 1; // Number of cores while excluding current thread.

	Platform::StartUpInternal();
	MemStack::BeginThread();

	ShaderCompilers::StartUp();
	ShaderManager::StartUp(GetShaderIncludeHandler());
	MessageHandler::StartUp();
	ProfilerCPU::StartUp();
	ProfilingManager::StartUp();
	ThreadPool::StartUp<TThreadPool<ThreadDefaultPolicy>>((numWorkerThreads));
	TaskScheduler::StartUp();
	RenderStats::StartUp();
	CoreThread::StartUp();
	StringTableManager::StartUp();
	DeferredCallManager::StartUp();
	Time::StartUp();
	DynamicLibraryManager::StartUp();
	CoreObjectManager::StartUp();
	GameObjectManager::StartUp();
	Resources::StartUp();
	ResourceListenerManager::StartUp();
	GpuProgramManager::StartUp();
	RenderStateManager::StartUp();
	ct::GpuProgramManager::StartUp();
	RenderAPIManager::StartUp();

	mPrimaryWindow = RenderAPIManager::Instance().Initialize(mStartUpDesc.RenderApi, mStartUpDesc.PrimaryWindowDesc);

	ct::ParamBlockManager::StartUp();
	Input::StartUp();
	RendererManager::StartUp();

	LoadPlugin(mStartUpDesc.Renderer, &mRendererPlugin);

	// Must be initialized before the scene manager, as game scene creation triggers physics scene creation
	PhysicsManager::StartUp(mStartUpDesc.Physics, mStartUpDesc.PhysicsCooking);
	SceneManager::StartUp();
	RendererManager::Instance().SetActive(mStartUpDesc.Renderer);
	StartUpRenderer();

	ProfilerGPU::StartUp();
	MeshManager::StartUp();
	Importer::StartUp();
	AudioManager::StartUp(mStartUpDesc.Audio);
	AnimationManager::StartUp();
	ParticleManager::StartUp();

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
	GetProfilerCPU().BeginThread("Sim");

	Platform::UpdateInternal();
	DeferredCallManager::Instance().UpdateInternal();
	GetTime().UpdateInternal();
	GetInput().UpdateInternal();
	// RenderWindowManager::update needs to happen after Input::update and before Input::_triggerCallbacks,
	// so that all input is properly captured in case there is a focus change, and so that
	// focus change is registered before input events are sent out (mouse press can result in code
	// checking if a window is in focus, so it has to be up to date)
	RenderWindowManager::Instance().UpdateInternal();
	GetInput().TriggerCallbacksInternal();
	GetDebug().TriggerCallbacksInternal();

	PreUpdate();

	// Trigger fixed updates if required
	{
		u64 step;
		const u32 numIterations = GetTime().GetFixedUpdateStepInternal(step);

		const float stepSeconds = step / 1000000.0f;
		for(u32 i = 0; i < numIterations; i++)
		{
			FixedUpdate();
			PROFILE_CALL(GetSceneManager().FixedUpdateInternal(), "Scene fixed update");
			PROFILE_CALL(GetPhysics().FixedUpdate(stepSeconds), "Physics simulation");

			GetTime().AdvanceFixedUpdateInternal(step);
		}
	}

	PROFILE_CALL(GetSceneManager().UpdateInternal(), "Scene update");
	GetAudio().UpdateInternal();
	GetPhysics().Update();

	// Update plugins
	for(auto& pluginUpdateFunc : mPluginUpdateFunctions)
		pluginUpdateFunc.second();

	PostUpdate();

	PerFrameData perFrameData;

	// Evaluate animation after scene and plugin updates because the renderer will just now be displaying the
	// animation we sent on the previous frame, and we want the scene information to match to what is displayed.
	perFrameData.Animation = AnimationManager::Instance().Update(mStartUpDesc.AsyncAnimation);
	perFrameData.Particles = ParticleManager::Instance().Update(*perFrameData.Animation);

	// Send out resource events in case any were loaded/destroyed/modified
	ResourceListenerManager::Instance().Update();

	// Trigger any renderer task callbacks (should be done before scene object update, or core sync, so objects have
	// a chance to respond to the callback).
	RendererManager::Instance().GetActive()->Update();

	GetSceneManager().UpdateCoreObjectTransformsInternal();
	PROFILE_CALL(RendererManager::Instance().GetActive()->RenderAll(perFrameData), "Render");

	// Core and sim thread run in lockstep. This will result in a larger input latency than if I was
	// running just a single thread. Latency becomes worse if the core thread takes longer than sim
	// thread, in which case sim thread needs to wait. Optimal solution would be to get an average
	// difference between sim/core thread and start the sim thread a bit later so they finish at nearly the same time.
	{
		Lock lock(mFrameRenderingFinishedMutex);

		while(!mIsFrameRenderingFinished)
		{
			TaskScheduler::Instance().AddWorker();
			mFrameRenderingFinishedCondition.wait(lock);
			TaskScheduler::Instance().RemoveWorker();
		}

		mIsFrameRenderingFinished = false;
	}

	GetCoreThread().QueueCommand(std::bind(&::bs::CoreApplication::BeginCoreProfiling, this), CTQF_InternalQueue);
	GetCoreThread().QueueCommand(&Platform::CoreUpdateInternal, CTQF_InternalQueue);
	GetCoreThread().QueueCommand(std::bind(&ct::RenderWindowManager::UpdateInternal, ct::RenderWindowManager::InstancePtr()), CTQF_InternalQueue);

	GetCoreThread().Update();
	GetCoreThread().SubmitAll();

	GetCoreThread().QueueCommand(std::bind(&::bs::CoreApplication::FrameRenderingFinishedCallback, this), CTQF_InternalQueue);

	GetCoreThread().QueueCommand(std::bind(&ct::QueryManager::UpdateInternal, ct::QueryManager::InstancePtr()), CTQF_InternalQueue);
	GetCoreThread().QueueCommand(std::bind(&::bs::CoreApplication::EndCoreProfiling, this), CTQF_InternalQueue);

	GetProfilerCPU().EndThread();
	GetProfiler().UpdateInternal();
}

void CoreApplication::WaitUntilFrameFinished()
{
	Lock lock(mFrameRenderingFinishedMutex);

	while(!mIsFrameRenderingFinished)
	{
		TaskScheduler::Instance().AddWorker();
		mFrameRenderingFinishedCondition.wait(lock);
		TaskScheduler::Instance().RemoveWorker();
	}
}

void CoreApplication::PreUpdate()
{
	// Do nothing
}

void CoreApplication::PostUpdate()
{
	// Do nothing
}

void CoreApplication::FixedUpdate()
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
	Lock lock(mFrameRenderingFinishedMutex);

	mIsFrameRenderingFinished = true;
	mFrameRenderingFinishedCondition.notify_one();
}

void CoreApplication::StartUpRenderer()
{
	RendererManager::Instance().Initialize();
}

void CoreApplication::BeginCoreProfiling()
{
#if !BS_FORCE_SINGLETHREADED_RENDERING
	GetProfilerCPU().BeginThread("Core");
#endif
}

void CoreApplication::EndCoreProfiling()
{
	ProfilerGPU::Instance().UpdateInternal();

#if !BS_FORCE_SINGLETHREADED_RENDERING
	GetProfilerCPU().EndThread();
	GetProfiler().UpdateCoreInternal();
#endif
}

void* CoreApplication::LoadPlugin(const String& pluginName, DynamicLibrary** library, void* passThrough)
{
	DynamicLibrary* loadedLibrary = GetDynamicLibraryManager().Load(pluginName);
	if(library != nullptr)
		*library = loadedLibrary;

	void* retVal = nullptr;
	if(loadedLibrary != nullptr)
	{
		if(passThrough == nullptr)
		{
			typedef void* (*LoadPluginFunc)();

			LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->GetSymbol("LoadPlugin");

			if(loadPluginFunc != nullptr)
				retVal = loadPluginFunc();
		}
		else
		{
			typedef void* (*LoadPluginFunc)(void*);

			LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->GetSymbol("LoadPlugin");

			if(loadPluginFunc != nullptr)
				retVal = loadPluginFunc(passThrough);
		}

		UpdatePluginFunc loadPluginFunc = (UpdatePluginFunc)loadedLibrary->GetSymbol("UpdatePlugin");

		if(loadPluginFunc != nullptr)
			mPluginUpdateFunctions[loadedLibrary] = loadPluginFunc;
	}

	return retVal;
}

void CoreApplication::UnloadPlugin(DynamicLibrary* library)
{
	typedef void (*UnloadPluginFunc)();

	UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)library->GetSymbol("UnloadPlugin");

	if(unloadPluginFunc != nullptr)
		unloadPluginFunc();

	mPluginUpdateFunctions.erase(library);
	GetDynamicLibraryManager().Unload(library);
}

SPtr<IShaderIncludeHandler> CoreApplication::GetShaderIncludeHandler() const
{
	return B3DMakeShared<DefaultShaderIncludeHandler>();
}

namespace bs
{
CoreApplication& GetCoreApplication()
{
	return CoreApplication::Instance();
}
} // namespace
