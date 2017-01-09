//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreApplication.h"

#include "BsRenderAPI.h"
#include "BsRenderAPIManager.h"

#include "BsPlatform.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderWindow.h"
#include "BsViewport.h"
#include "BsVector2.h"
#include "BsGpuProgram.h"
#include "BsCoreObjectManager.h"
#include "BsGameObjectManager.h"
#include "BsDynLib.h"
#include "BsDynLibManager.h"
#include "BsCoreSceneManager.h"
#include "BsImporter.h"
#include "BsResources.h"
#include "BsMesh.h"
#include "BsSceneObject.h"
#include "BsTime.h"
#include "BsInput.h"
#include "BsRendererManager.h"
#include "BsGpuProgramManager.h"
#include "BsMeshManager.h"
#include "BsMaterialManager.h"
#include "BsFontManager.h"
#include "BsRenderWindowManager.h"
#include "BsCoreRenderer.h"
#include "BsDeferredCallManager.h"
#include "BsCoreThread.h"
#include "BsStringTableManager.h"
#include "BsProfilingManager.h"
#include "BsProfilerCPU.h"
#include "BsProfilerGPU.h"
#include "BsQueryManager.h"
#include "BsThreadPool.h"
#include "BsTaskScheduler.h"
#include "BsRenderStats.h"
#include "BsMessageHandler.h"
#include "BsResourceListenerManager.h"
#include "BsRenderStateManager.h"
#include "BsShaderManager.h"
#include "BsPhysicsManager.h"
#include "BsPhysics.h"
#include "BsAudioManager.h"
#include "BsAudio.h"
#include "BsAnimationManager.h"
#include "BsParamBlocks.h"

namespace bs
{
	CoreApplication::CoreApplication(START_UP_DESC desc)
		: mPrimaryWindow(nullptr), mStartUpDesc(desc), mFrameStep(16666), mLastFrameTime(0), mRendererPlugin(nullptr)
		, mIsFrameRenderingFinished(true), mSimThreadId(BS_THREAD_CURRENT_ID), mRunMainLoop(false)
	{ }

	CoreApplication::~CoreApplication()
	{
		mPrimaryWindow->destroy();
		mPrimaryWindow = nullptr;

		Importer::shutDown();
		FontManager::shutDown();
		MaterialManager::shutDown();
		MeshManager::shutDown();
		ProfilerGPU::shutDown();

		CoreSceneManager::shutDown();
		
		Input::shutDown();

		ct::ParamBlockManager::shutDown();
		StringTableManager::shutDown();
		Resources::shutDown();
		ResourceListenerManager::shutDown();
		GameObjectManager::shutDown();
		RenderStateManager::shutDown();

		// This must be done after all resources are released since it will unload the physics plugin, and some resources
		// might be instances of types from that plugin.
		AnimationManager::shutDown();
		PhysicsManager::shutDown();
		AudioManager::shutDown();

		RendererManager::shutDown();

		// All CoreObject related modules should be shut down now. They have likely queued CoreObjects for destruction, so
		// we need to wait for those objects to get destroyed before continuing.
		CoreObjectManager::instance().syncToCore();
		gCoreThread().update();
		gCoreThread().submitAll(true);

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
	}

	void CoreApplication::onStartUp()
	{
		UINT32 numWorkerThreads = BS_THREAD_HARDWARE_CONCURRENCY - 1; // Number of cores while excluding current thread.

		Platform::_startUp();
		MemStack::beginThread();

		ShaderManager::startUp(getShaderIncludeHandler());
		MessageHandler::startUp();
		ProfilerCPU::startUp();
		ProfilingManager::startUp();
		ThreadPool::startUp<TThreadPool<ThreadBansheePolicy>>((numWorkerThreads));
		TaskScheduler::startUp();
		TaskScheduler::instance().removeWorker();
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

		mPrimaryWindow = RenderAPIManager::instance().initialize(mStartUpDesc.renderAPI, mStartUpDesc.primaryWindowDesc);

		ct::ParamBlockManager::startUp();
		Input::startUp();
		RendererManager::startUp();

		loadPlugin(mStartUpDesc.renderer, &mRendererPlugin);

		SceneManagerFactory::create();
		RendererManager::instance().setActive(mStartUpDesc.renderer);
		startUpRenderer();

		ProfilerGPU::startUp();
		MeshManager::startUp();
		MaterialManager::startUp();
		FontManager::startUp();
		Importer::startUp();
		AudioManager::startUp(mStartUpDesc.audio);
		PhysicsManager::startUp(mStartUpDesc.physics, isEditor());
		AnimationManager::startUp();

		for (auto& importerName : mStartUpDesc.importers)
			loadPlugin(importerName);

		loadPlugin(mStartUpDesc.input, nullptr, mPrimaryWindow.get());
	}

	void CoreApplication::runMainLoop()
	{
		mRunMainLoop = true;

		while(mRunMainLoop)
		{
			// Limit FPS if needed
			if (mFrameStep > 0)
			{
				UINT64 currentTime = gTime().getTimePrecise();
				UINT64 nextFrameTime = mLastFrameTime + mFrameStep;
				while (nextFrameTime > currentTime)
				{
					UINT32 waitTime = (UINT32)(nextFrameTime - currentTime);

					// If waiting for longer, sleep
					if (waitTime >= 2000)
					{
						Platform::sleep(waitTime / 1000);
						currentTime = gTime().getTimePrecise();
					}
					else
					{
						// Otherwise we just spin, sleep timer granularity is too low and we might end up wasting a 
						// millisecond otherwise. 
						// Note: For mobiles where power might be more important than input latency, consider using sleep.
						while(nextFrameTime > currentTime)
							currentTime = gTime().getTimePrecise();
					}
				}

				mLastFrameTime = currentTime;
			}

			gProfilerCPU().beginThread("Sim");

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
			AnimationManager::instance().preUpdate();

			preUpdate();

			if (mStartUpDesc.updateCallback != nullptr)
				mStartUpDesc.updateCallback();

			PROFILE_CALL(gCoreSceneManager()._update(), "SceneManager");
			gAudio()._update();
			gPhysics().update();
			AnimationManager::instance().postUpdate();

			// Update plugins
			for (auto& pluginUpdateFunc : mPluginUpdateFunctions)
				pluginUpdateFunc.second();

			postUpdate();

			// Send out resource events in case any were loaded/destroyed/modified
			ResourceListenerManager::instance().update();

			gCoreSceneManager()._updateCoreObjectTransforms();
			PROFILE_CALL(RendererManager::instance().getActive()->renderAll(), "Render");

			// Core and sim thread run in lockstep. This will result in a larger input latency than if I was 
			// running just a single thread. Latency becomes worse if the core thread takes longer than sim 
			// thread, in which case sim thread needs to wait. Optimal solution would be to get an average 
			// difference between sim/core thread and start the sim thread a bit later so they finish at nearly the same time.
			{
				Lock lock(mFrameRenderingFinishedMutex);

				while(!mIsFrameRenderingFinished)
				{
					TaskScheduler::instance().addWorker();
					mFrameRenderingFinishedCondition.wait(lock);
					TaskScheduler::instance().removeWorker();
				}

				mIsFrameRenderingFinished = false;
			}

			gCoreThread().queueCommand(std::bind(&CoreApplication::beginCoreProfiling, this), CTQF_InternalQueue);
			gCoreThread().queueCommand(&Platform::_coreUpdate, CTQF_InternalQueue);

			gCoreThread().update(); 
			gCoreThread().submitAll(); 

			gCoreThread().queueCommand(std::bind(&CoreApplication::frameRenderingFinishedCallback, this), CTQF_InternalQueue);

			gCoreThread().queueCommand(std::bind(&ct::RenderWindowManager::_update, ct::RenderWindowManager::instancePtr()), CTQF_InternalQueue);
			gCoreThread().queueCommand(std::bind(&ct::QueryManager::_update, ct::QueryManager::instancePtr()), CTQF_InternalQueue);
			gCoreThread().queueCommand(std::bind(&CoreApplication::endCoreProfiling, this), CTQF_InternalQueue);

			gProfilerCPU().endThread();
			gProfiler()._update();
		}

		// Wait until last core frame is finished before exiting
		{
			Lock lock(mFrameRenderingFinishedMutex);

			while (!mIsFrameRenderingFinished)
			{
				TaskScheduler::instance().addWorker();
				mFrameRenderingFinishedCondition.wait(lock);
				TaskScheduler::instance().removeWorker();
			}
		}
	}

	void CoreApplication::preUpdate()
	{
		// Do nothing
	}

	void CoreApplication::postUpdate()
	{
		// Do nothing
	}

	void CoreApplication::stopMainLoop()
	{
		mRunMainLoop = false; // No sync primitives needed, in that rare case of 
		// a race condition we might run the loop one extra iteration which is acceptable
	}

	void CoreApplication::quitRequested()
	{
		stopMainLoop();
	}

	void CoreApplication::setFPSLimit(UINT32 limit)
	{
		mFrameStep = (UINT64)1000000 / limit;
	}

	void CoreApplication::frameRenderingFinishedCallback()
	{
		Lock lock(mFrameRenderingFinishedMutex);

		mIsFrameRenderingFinished = true;
		mFrameRenderingFinishedCondition.notify_one();
	}

	void CoreApplication::startUpRenderer()
	{
		RendererManager::instance().initialize();
	}

	void CoreApplication::beginCoreProfiling()
	{
		gProfilerCPU().beginThread("Core");
	}

	void CoreApplication::endCoreProfiling()
	{
		ProfilerGPU::instance()._update();

		gProfilerCPU().endThread();
		gProfiler()._updateCore();
	}

	void* CoreApplication::loadPlugin(const String& pluginName, DynLib** library, void* passThrough)
	{
		DynLib* loadedLibrary = gDynLibManager().load(pluginName);
		if(library != nullptr)
			*library = loadedLibrary;

		void* retVal = nullptr;
		if(loadedLibrary != nullptr)
		{
			if (passThrough == nullptr)
			{
				typedef void* (*LoadPluginFunc)();

				LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->getSymbol("loadPlugin");

				if (loadPluginFunc != nullptr)
					retVal = loadPluginFunc();
			}
			else
			{
				typedef void* (*LoadPluginFunc)(void*);

				LoadPluginFunc loadPluginFunc = (LoadPluginFunc)loadedLibrary->getSymbol("loadPlugin");

				if (loadPluginFunc != nullptr)
					retVal = loadPluginFunc(passThrough);
			}

			UpdatePluginFunc loadPluginFunc = (UpdatePluginFunc)loadedLibrary->getSymbol("updatePlugin");

			if (loadPluginFunc != nullptr)
				mPluginUpdateFunctions[loadedLibrary] = loadPluginFunc;
		}

		return retVal;
	}

	void CoreApplication::unloadPlugin(DynLib* library)
	{
		typedef void (*UnloadPluginFunc)();

		UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)library->getSymbol("unloadPlugin");

		if(unloadPluginFunc != nullptr)
			unloadPluginFunc();

		mPluginUpdateFunctions.erase(library);
		gDynLibManager().unload(library);
	}

	SPtr<IShaderIncludeHandler> CoreApplication::getShaderIncludeHandler() const
	{
		return bs_shared_ptr_new<DefaultShaderIncludeHandler>();
	}

	CoreApplication& gCoreApplication()
	{
		return CoreApplication::instance();
	}
}