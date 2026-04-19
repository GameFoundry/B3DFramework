//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptObjectManager.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Scene/B3DGameObjectManager.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoUtil.h"
#include "B3DScriptObjectWrapper.h"
#include "Utility/B3DTime.h"

using namespace b3d;
ScriptObjectManager::~ScriptObjectManager()
{
	ProcessFinalizedObjects();
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
	UnorderedMap<IScriptObjectWrapper*, ScriptObjectReloadPersistentData> reloadPeristentDataMap;

	OnRefreshStarted();

	for(auto& scriptObject : mScriptObjectWrappers)
		scriptObject->NotifyScriptWillReload();

	// Make sure any managed game objects are properly destroyed so their OnDestroy callbacks fire before unloading the domain
	GameObjectManager::Instance().DestroyQueuedObjects();

	// Make sure all objects that are finalized due to reasons other than assembly refreshed are destroyed
	ProcessFinalizedObjects(false);

	for(auto& scriptObjectWrapper : mScriptObjectWrappers)
	{
		TOptional<ScriptObjectReloadPersistentData> reloadPersistentData = scriptObjectWrapper->BackupDataBeforeScriptReload();
		if(reloadPersistentData.has_value())
			reloadPeristentDataMap[scriptObjectWrapper] = *reloadPersistentData;
	}

	for(auto& scriptObjectWrapper : mScriptObjectWrappers)
		scriptObjectWrapper->ReleaseScriptObjectHandle();

	OnWillUnloadAssemblies();

	// Unload the domain which GCs all the script objects and triggers their finalizers
	MonoManager::Instance().UnloadScriptDomain();

	// Unload script domain should trigger finalizers on everything, but since we usually delay
	// their processing we need to manually trigger it here.
	ProcessFinalizedObjects(true);

	for(auto it = mScriptObjectWrappers.begin(); it != mScriptObjectWrappers.end();)
	{
		ScriptObjectWrapper* const scriptObjectWrapper = *it;
#if !B3D_USE_DOTNETCORE
		B3D_ENSURE(scriptObjectWrapper->ShouldPersistScriptReload());
		++it;
#else
		if(!B3D_ENSURE(scriptObjectWrapper->ShouldPersistScriptReload()))
		{
			it = mScriptObjectWrappers.erase(it);
			scriptObjectWrapper->NotifyScriptObjectDestroyed(true);
		}
		else
			++it;
#endif
	}

#if B3D_USE_DOTNETCORE
	MonoManager::Instance().UnloadMonoLibrary();
#endif

	ScriptAssemblyManager::Instance().ClearAssemblyInfo();

#if B3D_USE_DOTNETCORE
	MonoManager::Instance().LoadMonoLibrary();
#endif

	for(auto& entry : assemblies)
	{
		MonoManager::Instance().LoadAssembly(*entry.Path, entry.Name);
		ScriptAssemblyManager::Instance().LoadAssemblyInfo(entry.Name);
	}

	TArray<ScriptObjectWrapper*> scriptObjectWrappersToRestore;
	scriptObjectWrappersToRestore.Reserve((u64)mScriptObjectWrappers.size());

	for(const auto& scriptObjectWrapper : mScriptObjectWrappers)
		scriptObjectWrappersToRestore.Add(scriptObjectWrapper);

	OnRefreshAssembliesLoaded();

	for(auto& scriptObjectWrapper : scriptObjectWrappersToRestore)
		scriptObjectWrapper->RecreateScriptObjectAfterScriptReload();

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

	for(auto& finalizedScriptObjectWrapper : mFinalizedScriptObjectWrappers[readQueueIdx])
		finalizedScriptObjectWrapper->NotifyScriptObjectDestroyed(assemblyRefresh);

	mFinalizedScriptObjectWrappers[readQueueIdx].clear();
}

void ScriptObjectManager::PerformGarbageCollection()
{
	// TODO - Should time-slice this over multiple frames

	for(const auto& entry : mScriptObjectWrappers)
	{
		// TODO - Keep these entries in a separate list so we don't have to check here
		if(entry->GetLifetimeTrackingMode() != ScriptObjectLifetimeTrackingMode::StrongHandleWithGarbageCollection)
			continue;

		const u32 nativeObjectReferenceCount = entry->GetNativeObjectReferenceCount();

		if(nativeObjectReferenceCount == 1)
			entry->TransitionToWeakScriptObjectHandle();
	}
}

