//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObjectManager.h"
#include "BsScriptObject.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Scene/BsGameObjectManager.h"
#include "BsMonoAssembly.h"

namespace bs
{
	ScriptObjectManager::~ScriptObjectManager()
	{
		ProcessFinalizedObjects();
	}

	void ScriptObjectManager::RegisterScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.insert(instance);
	}

	void ScriptObjectManager::UnregisterScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.erase(instance);
	}

	void ScriptObjectManager::RefreshAssemblies(const Vector<AssemblyRefreshInfo>& assemblies)
	{
		Map<ScriptObjectBase*, ScriptObjectBackup> backupData;

		OnRefreshStarted();

		// Make sure any managed game objects are properly destroyed so their OnDestroy callbacks fire before unloading the domain
		GameObjectManager::Instance().DestroyQueuedObjects();

		// Make sure all objects that are finalized due to reasons other than assembly refreshed are destroyed
		ProcessFinalizedObjects(false);

		for(auto& scriptObject : mScriptObjects)
			backupData[scriptObject] = scriptObject->BeginRefresh();

		for(auto& scriptObject : mScriptObjects)
			scriptObject->ClearManagedInstanceInternal();

		MonoManager::Instance().UnloadScriptDomain();

		// Unload script domain should trigger finalizers on everything, but since we usually delay
		// their processing we need to manually trigger it here.
		ProcessFinalizedObjects(true);

		for(auto& scriptObject : mScriptObjects)
			assert(scriptObject->IsPersistent() && "Non-persistent ScriptObject alive after domain unload.");

		ScriptAssemblyManager::Instance().ClearAssemblyInfo();

		for(auto& entry : assemblies)
		{
			MonoManager::Instance().LoadAssembly(*entry.Path, entry.Name);
			ScriptAssemblyManager::Instance().LoadAssemblyInfo(entry.Name, *entry.TypeMapping);
		}

		Vector<ScriptObjectBase*> scriptObjCopy(mScriptObjects.size()); // Store originals as we could add new objects during the next iteration
		u32 idx = 0;
		for(auto& scriptObject : mScriptObjects)
			scriptObjCopy[idx++] = scriptObject;

		OnRefreshDomainLoaded();

		for(auto& scriptObject : scriptObjCopy)
			scriptObject->RestoreManagedInstanceInternal();

		for(auto& scriptObject : scriptObjCopy)
			scriptObject->EndRefresh(backupData[scriptObject]);

		OnRefreshComplete();
	}

	void ScriptObjectManager::NotifyObjectFinalized(ScriptObjectBase* instance)
	{
		assert(instance != nullptr);

		Lock lock(mMutex);
		mFinalizedObjects[mFinalizedQueueIdx].push_back(instance);
	}

	void ScriptObjectManager::Update()
	{
		ProcessFinalizedObjects();
	}

	void ScriptObjectManager::ProcessFinalizedObjects(bool assemblyRefresh)
	{
		u32 readQueueIdx = 0;
		{
			Lock lock(mMutex);
			readQueueIdx = mFinalizedQueueIdx;
			mFinalizedQueueIdx = (mFinalizedQueueIdx + 1) % 2;
		}

		for(auto& finalizedObj : mFinalizedObjects[readQueueIdx])
			finalizedObj->OnManagedInstanceDeletedInternal(assemblyRefresh);

		mFinalizedObjects[readQueueIdx].clear();
	}
} // namespace bs
