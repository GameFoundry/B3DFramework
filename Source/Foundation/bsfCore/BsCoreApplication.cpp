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
#include "Utility/BsDynLib.h"
#include "Utility/BsDynLibManager.h"
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
#include "Renderer/BsParamBlocks.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsVectorField.h"

namespace bs
{
	BS_LOG_CATEGORY_IMPL(CoreThread)
	BS_LOG_CATEGORY_IMPL(Renderer)
	BS_LOG_CATEGORY_IMPL(Scene)
	BS_LOG_CATEGORY_IMPL(Physics)
	BS_LOG_CATEGORY_IMPL(Audio)
	BS_LOG_CATEGORY_IMPL(RenderBackend)
	BS_LOG_CATEGORY_IMPL(BSLCompiler)
	BS_LOG_CATEGORY_IMPL(Particles)
	BS_LOG_CATEGORY_IMPL(Resources)
	BS_LOG_CATEGORY_IMPL(FBXImporter)
	BS_LOG_CATEGORY_IMPL(PixelUtility)
	BS_LOG_CATEGORY_IMPL(Texture)
	BS_LOG_CATEGORY_IMPL(Mesh)
	BS_LOG_CATEGORY_IMPL(GUI)
	BS_LOG_CATEGORY_IMPL(Profiler)
	BS_LOG_CATEGORY_IMPL(Material)
	BS_LOG_CATEGORY_IMPL(FreeImageImporter)
	BS_LOG_CATEGORY_IMPL(Script)
	BS_LOG_CATEGORY_IMPL(Importer)

	CoreApplication::CoreApplication(START_UP_DESC desc)
		: MPrimaryWindow(nullptr), mStartUpDesc(desc), mRendererPlugin(nullptr), mIsFrameRenderingFinished(true)
		, MSimThreadId(BS_THREAD_CURRENT_ID), mRunMainLoop(false)
	{
		// Ensure all errors are reported properly
		CrashHandler::startUp(desc.crashHandling);
		if(desc.logCallback)
			gDebug().SetLogCallback(desc.logCallback);
	}

	CoreApplication::~CoreApplication()
	{
		mPrimaryWindow->Destroy();
		mPrimaryWindow = nullptr;

		Importer::shutDown();
		MeshManager::shutDown();
		ProfilerGPU::shutDown();

		SceneManager::shutDown();
		
		Input::shutDown();

		ct::ParamBlockManager::shutDown();
		StringTableManager::shutDown();
		Resources::shutDown();
		GameObjectManager::shutDown();

		// Audio manager must be released before the ResourceListenerManager, as any one-shot audio sources need to be
		// destroyed since they implement the IResourceListener interface
		AudioManager::shutDown();
		ResourceListenerManager::shutDown();
		RenderStateManager::shutDown();
		ParticleManager::shutDown();
		AnimationManager::shutDown();

		// This must be done after all resources are released since it will unload the physics plugin, and some resources
		// might be instances of types from that plugin.
		PhysicsManager::shutDown();

		RendererManager::shutDown();

		// All CoreObject related modules should be shut down now. They have likely queued CoreObjects for destruction, so
		// we need to wait for those objects to get destroyed before continuing.
		CoreObjectManager::instance().SyncToCore();
		gCoreThread().Update();
		gCoreThread().SubmitAll(true);

		unloadPlugin(mRendererPlugin);

		RenderAPIManager::shutDown();
		ct::GpuProgramManager::shutDown();
		GpuProgramManager::shutDown();

		CoreObjectManager::shutDown(); // Must shut down before DynLibManager to ensure all objects are destroyed before unloading their libraries
		DynLibManager::shutDown();
		Time::shutDown();
		DeferredCallManager::shutDown();

		CoreThread::shutDown();
		RenderStats::shutDown();
		TaskScheduler::shutDown();
		ThreadPool::shutDown();
		ProfilingManager::shutDown();
		ProfilerCPU::shutDown();
		MessageHandler::shutDown();
		ShaderManager::shutDown();

		MemStack::endThread();
		Platform::_shutDown();

		CrashHandler::shutDown();
	}

	void CoreApplication::OnStartUp()
	{
		UINT32 numWorkerThreads = BS_THREAD_HARDWARE_CONCURRENCY - 1; // Number of cores while excluding current thread.

		Platform::_startUp();
		MemStack::beginThread();

		ShaderManager::startUp(getShaderIncludeHandler());
		MessageHandler::startUp();
		ProfilerCPU::startUp();
		ProfilingManager::startUp();
		ThreadPool::startUp<TThreadPool<ThreadDefaultPolicy>>((numWorkerThreads));
		TaskScheduler::startUp();
		RenderStats::startUp();
		CoreThread::startUp();
		StringTableManager::startUp();
		DeferredCallManager::startUp();
		Time::startUp();
		DynLibManager::startUp();
		CoreObjectManager::startUp();
		GameObjectManager::startUp();
		Resources::startUp();
		ResourceListenerManager::startUp();
		GpuProgramManager::startUp();
		RenderStateManager::startUp();
		ct::GpuProgramManager::startUp();
		RenderAPIManager::startUp();

		mPrimaryWindow = RenderAPIManager::instance().Initialize(mStartUpDesc.renderAPI, mStartUpDesc.primaryWindowDesc);

		ct::ParamBlockManager::startUp();
		Input::startUp();
		RendererManager::startUp();

		loadPlugin(mStartUpDesc.renderer, &mRendererPlugin);

		// Must be initialized before the scene manager, as game scene creation triggers physics scene creation
		PhysicsManager::startUp(mStartUpDesc.physics, mStartUpDesc.physicsCooking);
		SceneManager::startUp();
		RendererManager::instance().SetActive(mStartUpDesc.renderer);
		startUpRenderer();

		ProfilerGPU::startUp();
		MeshManager::startUp();
		Importer::startUp();
		AudioManager::startUp(mStartUpDesc.audio);
		AnimationManager::startUp();
		ParticleManager::startUp();

		for (auto& importerName : mStartUpDesc.importers)
			loadPlugin(importerName);

		// Built-in importers
		FGAImporter* fgaImporter = bs_new<FGAImporter>();
		Importer::instance()._registerAssetImporter(fgaImporter);
	}

	void CoreApplication::RunMainLoop()
	{
		beginMainLoop();

		while(isMainLoopRunning())
		{
			// Limit FPS if needed
			if (mFrameStep > 0)
			{
				UINT64 currentTime = gTime().GetTimePrecise();
				UINT64 nextFrameTime = mLastFrameTime + mFrameStep;
				while (nextFrameTime > currentTime)
				{
					UINT32 waitTime = (UINT32)(nextFrameTime - currentTime);

					// If waiting for longer, sleep
					if (waitTime >= 2000)
					{
						Platform::sleep(waitTime / 1000);
						currentTime = gTime().GetTimePrecise();
					}
					else
					{
						// Otherwise we just spin, sleep timer granularity is too low and we might end up wasting a
						// millisecond otherwise.
						// Note: For mobiles where power might be more important than input latency, consider using sleep.
						while(nextFrameTime > currentTime)
							currentTime = gTime().GetTimePrecise();
					}
				}

				mLastFrameTime = currentTime;
			}

			runMainLoopFrame();
		}

		endMainLoop();
	}

	void CoreApplication::BeginMainLoop()
	{
		mRunMainLoop = true;
	}

	void CoreApplication::EndMainLoop()
	{
		waitUntilFrameFinished();
	}

	void CoreApplication::RunMainLoopFrame()
	{
		gProfilerCPU().BeginThread("Sim");

		Platform::_update();
		DeferredCallManager::instance()._update();
		gTime()._update();
		gInput()._update();
		// RenderWindowManager::update needs to happen after Input::update and before Input::_triggerCallbacks,
		// so that all input is properly captured in case there is a focus change, and so that
		// focus change is registered before input events are sent out (mouse press can result in code
		// checking if a window is in focus, so it has to be up to date)
		RenderWindowManager::instance()._update();
		gInput()._triggerCallbacks();
		gDebug()._triggerCallbacks();

		preUpdate();

		// Trigger fixed updates if required
		{
			UINT64 step;
			const UINT32 numIterations = gTime()._getFixedUpdateStep(step);

			const float stepSeconds = step / 1000000.0f;
			for (UINT32 i = 0; i < numIterations; i++)
			{
				fixedUpdate();
				PROFILE_CALL(gSceneManager()._fixedUpdate(), "Scene fixed update");
				PROFILE_CALL(gPhysics().FixedUpdate(stepSeconds), "Physics simulation");

				gTime()._advanceFixedUpdate(step);
			}
		}

		PROFILE_CALL(gSceneManager()._update(), "Scene update");
		gAudio()._update();
		gPhysics().Update();

		// Update plugins
		for (auto& pluginUpdateFunc : mPluginUpdateFunctions)
			pluginUpdateFunc.Second();

		postUpdate();

		PerFrameData perFrameData;

		// Evaluate animation after scene and plugin updates because the renderer will just now be displaying the
		// animation we sent on the previous frame, and we want the scene information to match to what is displayed.
		perFrameData.animation = AnimationManager::instance().Update(mStartUpDesc.asyncAnimation);
		perFrameData.particles = ParticleManager::instance().Update(*perFrameData.animation);

		// Send out resource events in case any were loaded/destroyed/modified
		ResourceListenerManager::instance().Update();

		// Trigger any renderer task callbacks (should be done before scene object update, or core sync, so objects have
		// a chance to respond to the callback).
		RendererManager::instance().GetActive()->Update();

		gSceneManager()._updateCoreObjectTransforms();
		PROFILE_CALL(RendererManager::instance().GetActive()->RenderAll(perFrameData), "Render");

		// Core and sim thread run in lockstep. This will result in a larger input latency than if I was
		// running just a single thread. Latency becomes worse if the core thread takes longer than sim
		// thread, in which case sim thread needs to wait. Optimal solution would be to get an average
		// difference between sim/core thread and start the sim thread a bit later so they finish at nearly the same time.
		{
			Lock Lock(mFrameRenderingFinishedMutex);

			while(!mIsFrameRenderingFinished)
			{
				TaskScheduler::instance().AddWorker();
				mFrameRenderingFinishedCondition.Wait(lock);
				TaskScheduler::instance().RemoveWorker();
			}

			mIsFrameRenderingFinished = false;
		}

		gCoreThread().QueueCommand(std::bind(&CoreApplication::beginCoreProfiling, this), CTQF_InternalQueue);
		gCoreThread().QueueCommand(&Platform::_coreUpdate, CTQF_InternalQueue);
		gCoreThread().QueueCommand(std::bind(&ct::RenderWindowManager::_update, ct::RenderWindowManager::instancePtr()), CTQF_InternalQueue);

		gCoreThread().Update();
		gCoreThread().SubmitAll();

		gCoreThread().QueueCommand(std::bind(&CoreApplication::frameRenderingFinishedCallback, this), CTQF_InternalQueue);

		gCoreThread().QueueCommand(std::bind(&ct::QueryManager::_update, ct::QueryManager::instancePtr()), CTQF_InternalQueue);
		gCoreThread().QueueCommand(std::bind(&CoreApplication::endCoreProfiling, this), CTQF_InternalQueue);

		gProfilerCPU().EndThread();
		gProfiler()._update();
	}

	void CoreApplication::WaitUntilFrameFinished()
	{
		Lock Lock(mFrameRenderingFinishedMutex);

		while (!mIsFrameRenderingFinished)
		{
			TaskScheduler::instance().AddWorker();
			mFrameRenderingFinishedCondition.Wait(lock);
			TaskScheduler::instance().RemoveWorker();
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
		stopMainLoop();
	}

	void CoreApplication::SetFPSLimit(UINT32 limit)
	{
		if(limit > 0)
			mFrameStep = (UINT64)1000000 / limit;
		else
			mFrameStep = 0;
	}

	void CoreApplication::FrameRenderingFinishedCallback()
	{
		Lock Lock(mFrameRenderingFinishedMutex);

		mIsFrameRenderingFinished = true;
		mFrameRenderingFinishedCondition.notify_one();
	}

	void CoreApplication::StartUpRenderer()
	{
		RendererManager::instance().Initialize();
	}

	void CoreApplication::BeginCoreProfiling()
	{
#if !BS_FORCE_SINGLETHREADED_RENDERING
		gProfilerCPU().BeginThread("Core");
#endif
	}

	void CoreApplication::EndCoreProfiling()
	{
		ProfilerGPU::instance()._update();

#if !BS_FORCE_SINGLETHREADED_RENDERING
		gProfilerCPU().EndThread();
		gProfiler()._updateCore();
#endif
	}

	void* CoreApplication::loadPlugin(const String& pluginName, DynLib** library, void* passThrough)
	{
		DynLib* loadedLibrary = gDynLibManager().Load(pluginName);
		if(library != nullptr)
			*library = loadedLibrary;

		void* retVal = nullptr;
		if(loadedLibrary != nullptr)
		{
			if (passThrough == nullptr)
			{
				typedef void* (*LoadPluginFunc)();

				LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->GetSymbol("loadPlugin");

				if (loadPluginFunc != nullptr)
					retVal = loadPluginFunc();
			}
			else
			{
				typedef void* (*LoadPluginFunc)(void*);

				LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->GetSymbol("loadPlugin");

				if (loadPluginFunc != nullptr)
					retVal = loadPluginFunc(passThrough);
			}

			UpdatePluginFunc loadPluginFunc = (UpdatePluginFunc)loadedLibrary->GetSymbol("updatePlugin");

			if (loadPluginFunc != nullptr)
				mPluginUpdateFunctions[loadedLibrary] = loadPluginFunc;
		}

		return retVal;
	}

	void CoreApplication::UnloadPlugin(DynLib* library)
	{
		typedef void (*UnloadPluginFunc)();

		UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)library->GetSymbol("unloadPlugin");

		if(unloadPluginFunc != nullptr)
			unloadPluginFunc();

		mPluginUpdateFunctions.erase(library);
		gDynLibManager().Unload(library);
	}

	SPtr<IShaderIncludeHandler> CoreApplication::GetShaderIncludeHandler() const
	{
		return bs_shared_ptr_new<DefaultShaderIncludeHandler>();
	}

	CoreApplication& GCoreApplication()
	{
		return CoreApplication::Instance();
	}
}
