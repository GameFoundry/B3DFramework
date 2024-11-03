//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObjectManager.h"
#include "BsScriptObject.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Scene/BsGameObjectManager.h"
#include "BsMonoAssembly.h"
#include "BsScriptObjectWrapper.h"

using namespace bs;
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

void ScriptObjectManager::RegisterScriptObjectWrapper(ScriptObjectWrapper* scriptObjectWrapper)
{
	mScriptObjectWrappers.insert(scriptObjectWrapper);
}

void ScriptObjectManager::UnregisterScriptObjectWrapper(ScriptObjectWrapper* scriptObjectWrapper)
{
	mScriptObjectWrappers.erase(scriptObjectWrapper);
}

void ScriptObjectManager::RefreshAssemblies(const Vector<AssemblyRefreshInfo>& assemblies)
{
	Map<ScriptObjectBase*, ScriptObjectBackup> backupData;
	UnorderedMap<IScriptObjectWrapper*, ScriptObjectReloadPersistentData> reloadPeristentDataMap;

	OnRefreshStarted();

	for(auto& scriptObject : mScriptObjectWrappers)
		scriptObject->NotifyScriptWillReload();

	// Make sure any managed game objects are properly destroyed so their OnDestroy callbacks fire before unloading the domain
	GameObjectManager::Instance().DestroyQueuedObjects();

	// Make sure all objects that are finalized due to reasons other than assembly refreshed are destroyed
	ProcessFinalizedObjects(false);

	for(auto& scriptObject : mScriptObjects)
		backupData[scriptObject] = scriptObject->BeginRefresh();

	for(auto& scriptObjectWrapper : mScriptObjectWrappers)
	{
		Optional<ScriptObjectReloadPersistentData> reloadPersistentData = scriptObjectWrapper->BackupDataBeforeScriptReload();
		if(reloadPersistentData.has_value())
			reloadPeristentDataMap[scriptObjectWrapper] = *reloadPersistentData;
	}

	for(auto& scriptObject : mScriptObjects)
		scriptObject->ClearManagedInstanceInternal();

	for(auto& scriptObjectWrapper : mScriptObjectWrappers)
		scriptObjectWrapper->ReleaseScriptObjectHandle();

	MonoManager::Instance().UnloadScriptDomain();

	// Unload script domain should trigger finalizers on everything, but since we usually delay
	// their processing we need to manually trigger it here.
	ProcessFinalizedObjects(true);

	for(auto& scriptObject : mScriptObjects)
		B3D_ASSERT(scriptObject->IsPersistent() && "Non-persistent ScriptObject alive after domain unload.");

	for(auto& scriptObjectWrapper : mScriptObjectWrappers)
		B3D_ENSURE(scriptObjectWrapper->ShouldPersistScriptReload());

	ScriptAssemblyManager::Instance().ClearAssemblyInfo();

	for(auto& entry : assemblies)
	{
		MonoManager::Instance().LoadAssembly(*entry.Path, entry.Name);
		ScriptAssemblyManager::Instance().LoadAssemblyInfo(entry.Name);
	}

	Vector<ScriptObjectBase*> scriptObjCopy(mScriptObjects.size()); // Store originals as we could add new objects during the next iteration
	u32 idx = 0;
	for(auto& scriptObject : mScriptObjects)
		scriptObjCopy[idx++] = scriptObject;

	TArray<ScriptObjectWrapper*> scriptObjectWrappersToRestore;
	scriptObjectWrappersToRestore.Reserve((u64)mScriptObjects.size());

	for(const auto& scriptObjectWrapper : mScriptObjectWrappers)
		scriptObjectWrappersToRestore.Add(scriptObjectWrapper);

	OnRefreshDomainLoaded();

	for(auto& scriptObject : scriptObjCopy)
		scriptObject->RestoreManagedInstanceInternal();

	for(auto& scriptObjectWrapper : scriptObjectWrappersToRestore)
		scriptObjectWrapper->RecreateScriptObjectAfterScriptReload();

	for(auto& scriptObject : scriptObjCopy)
		scriptObject->EndRefresh(backupData[scriptObject]);

	for(auto& scriptObjectWrapper : scriptObjectWrappersToRestore)
	{
		const auto found = reloadPeristentDataMap.find(scriptObjectWrapper);
		if(found == reloadPeristentDataMap.end())
			continue;

		scriptObjectWrapper->RestoreDataAfterScriptReload(found->second);
	}

	for(auto& scriptObjectWrapper : scriptObjectWrappersToRestore)
		scriptObjectWrapper->NotifyScriptReloadFinished();

	OnRefreshComplete();
}

void ScriptObjectManager::NotifyObjectFinalized(ScriptObjectBase* instance)
{
	B3D_ASSERT(instance != nullptr);

	Lock lock(mMutex);
	mFinalizedObjects[mFinalizedQueueIdx].push_back(instance);
}

void ScriptObjectManager::NotifyObjectFinalized(ScriptObjectWrapper* scriptObjectWrapper)
{
	if(!B3D_ENSURE(scriptObjectWrapper != nullptr))
		return;

	Lock lock(mMutex);
	mFinalizedScriptObjectWrappers[mFinalizedQueueIdx].push_back(scriptObjectWrapper);
}

void ScriptObjectManager::Update()
{
	ProcessFinalizedObjects();

	const float currentTimeMs = GetTime().GetRealTimeInSeconds();
	if((mLastGarbageCollectionTimeMs + mGarbageCollectionIntervalMs) <= currentTimeMs)
	{
		PerformGarbageCollection();

		mLastGarbageCollectionTimeMs = currentTimeMs;
	}
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

	for(auto& finalizedScriptObjectWrapper : mFinalizedScriptObjectWrappers[readQueueIdx])
		finalizedScriptObjectWrapper->NotifyScriptObjectDestroyed(assemblyRefresh);

	mFinalizedScriptObjectWrappers[readQueueIdx].clear();
}

void ScriptObjectManager::PerformGarbageCollection()
{
#if 0 // Disabled until I solve issues that are happening without GC
	// TODO - Should time-slice this over multiple frames

	for(const auto& entry : mScriptObjectWrappers)
	{
		const u32 nativeObjectReferenceCount = entry->GetNativeObjectReferenceCount();

		if(nativeObjectReferenceCount == 1)
			entry->TransitionToWeakScriptObjectHandle();
	}
#endif
}

