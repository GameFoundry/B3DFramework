//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptObjectManager.h"
#include "BsScriptObject.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsGameObjectManager.h"
#include "BsMonoAssembly.h"

namespace bs
{
	ScriptObjectManager::ScriptObjectManager()
		:mFinalizedQueueIdx(0)
	{

	}

	ScriptObjectManager::~ScriptObjectManager()
	{
		processFinalizedObjects();
	}

	void ScriptObjectManager::registerScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.insert(instance);
	}

	void ScriptObjectManager::unregisterScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.erase(instance);
	}

	void ScriptObjectManager::refreshAssemblies(const Vector<std::pair<String, Path>>& assemblies)
	{
		Map<ScriptObjectBase*, ScriptObjectBackup> backupData;

		onRefreshStarted();

		// Make sure any managed game objects are properly destroyed so their OnDestroy callbacks fire before unloading the domain
		GameObjectManager::instance().destroyQueuedObjects();

		for (auto& scriptObject : mScriptObjects)
			backupData[scriptObject] = scriptObject->beginRefresh();

		MonoManager::instance().unloadScriptDomain();
		// Unload script domain should trigger finalizers on everything, but since we usually delay
		// their processing we need to manually trigger it here.
		processFinalizedObjects();

		for (auto& scriptObject : mScriptObjects)
			assert(scriptObject->isPersistent() && "Non-persistent ScriptObject alive after domain unload.");

		for (auto& scriptObject : mScriptObjects)
			scriptObject->_clearManagedInstance();

		ScriptAssemblyManager::instance().clearAssemblyInfo();

		for (auto& assemblyPair : assemblies)
		{
			MonoManager::instance().loadAssembly(assemblyPair.second.toWString(), assemblyPair.first);
			ScriptAssemblyManager::instance().loadAssemblyInfo(assemblyPair.first);
		}

		Vector<ScriptObjectBase*> scriptObjCopy(mScriptObjects.size()); // Store originals as we could add new objects during the next iteration
		UINT32 idx = 0;
		for (auto& scriptObject : mScriptObjects)
			scriptObjCopy[idx++] = scriptObject;

		onRefreshDomainLoaded();

		for (auto& scriptObject : scriptObjCopy)
			scriptObject->_restoreManagedInstance();

		for (auto& scriptObject : scriptObjCopy)
			scriptObject->endRefresh(backupData[scriptObject]);

		onRefreshComplete();
	}

	void ScriptObjectManager::notifyObjectFinalized(ScriptObjectBase* instance)
	{
		assert(instance != nullptr);

		Lock lock(mMutex);
		mFinalizedObjects[mFinalizedQueueIdx].push_back(instance);
	}

	void ScriptObjectManager::update()
	{
		processFinalizedObjects();
	}

	void ScriptObjectManager::processFinalizedObjects()
	{
		UINT32 readQueueIdx = 0;
		{
			Lock lock(mMutex);
			readQueueIdx = mFinalizedQueueIdx;
			mFinalizedQueueIdx = (mFinalizedQueueIdx + 1) % 2;
		}
		
		for (auto& finalizedObj : mFinalizedObjects[readQueueIdx])
			finalizedObj->_onManagedInstanceDeleted();

		mFinalizedObjects[readQueueIdx].clear();
	}
}