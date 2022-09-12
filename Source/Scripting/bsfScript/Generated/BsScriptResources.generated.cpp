//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResources.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManifest.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptResources::onResourceLoadedThunkDef ScriptResources::onResourceLoadedThunk; 
	ScriptResources::onResourceDestroyedThunkDef ScriptResources::onResourceDestroyedThunk; 
	ScriptResources::onResourceModifiedThunkDef ScriptResources::onResourceModifiedThunk; 

	HEvent ScriptResources::onResourceLoadedConn;
	HEvent ScriptResources::onResourceDestroyedConn;
	HEvent ScriptResources::onResourceModifiedConn;

	ScriptResources::ScriptResources(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
		mGCHandle = MonoUtil::newWeakGCHandle(managedInstance);
	}

	void ScriptResources::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_load", (void*)&ScriptResources::Internal_load);
		metaData.scriptClass->AddInternalCall("Internal_loadAsync", (void*)&ScriptResources::Internal_loadAsync);
		metaData.scriptClass->AddInternalCall("Internal_loadFromUUID", (void*)&ScriptResources::Internal_loadFromUUID);
		metaData.scriptClass->AddInternalCall("Internal_release", (void*)&ScriptResources::Internal_release);
		metaData.scriptClass->AddInternalCall("Internal_unloadAllUnused", (void*)&ScriptResources::Internal_unloadAllUnused);
		metaData.scriptClass->AddInternalCall("Internal_unloadAll", (void*)&ScriptResources::Internal_unloadAll);
		metaData.scriptClass->AddInternalCall("Internal_save", (void*)&ScriptResources::Internal_save);
		metaData.scriptClass->AddInternalCall("Internal_save0", (void*)&ScriptResources::Internal_save0);
		metaData.scriptClass->AddInternalCall("Internal_getDependencies", (void*)&ScriptResources::Internal_getDependencies);
		metaData.scriptClass->AddInternalCall("Internal_isLoaded", (void*)&ScriptResources::Internal_isLoaded);
		metaData.scriptClass->AddInternalCall("Internal_getLoadProgress", (void*)&ScriptResources::Internal_getLoadProgress);
		metaData.scriptClass->AddInternalCall("Internal_registerResourceManifest", (void*)&ScriptResources::Internal_registerResourceManifest);
		metaData.scriptClass->AddInternalCall("Internal_unregisterResourceManifest", (void*)&ScriptResources::Internal_unregisterResourceManifest);
		metaData.scriptClass->AddInternalCall("Internal_getResourceManifest", (void*)&ScriptResources::Internal_getResourceManifest);
		metaData.scriptClass->AddInternalCall("Internal_getFilePathFromUUID", (void*)&ScriptResources::Internal_getFilePathFromUUID);
		metaData.scriptClass->AddInternalCall("Internal_getUUIDFromFilePath", (void*)&ScriptResources::Internal_getUUIDFromFilePath);

		onResourceLoadedThunk = (onResourceLoadedThunkDef)metaData.scriptClass->GetMethodExact("Internal_onResourceLoaded", "RRefBase")->getThunk();
		onResourceDestroyedThunk = (onResourceDestroyedThunkDef)metaData.scriptClass->GetMethodExact("Internal_onResourceDestroyed", "UUID&")->getThunk();
		onResourceModifiedThunk = (onResourceModifiedThunkDef)metaData.scriptClass->GetMethodExact("Internal_onResourceModified", "RRefBase")->getThunk();
	}

	void ScriptResources::StartUp()
	{
		onResourceLoadedConn = Resources::instance().onResourceLoaded.Connect(&ScriptResources::onResourceLoaded);
		onResourceDestroyedConn = Resources::instance().onResourceDestroyed.Connect(&ScriptResources::onResourceDestroyed);
		onResourceModifiedConn = Resources::instance().onResourceModified.Connect(&ScriptResources::onResourceModified);
	}
	void ScriptResources::ShutDown()
	{
		onResourceLoadedConn.Disconnect();
		onResourceDestroyedConn.Disconnect();
		onResourceModifiedConn.Disconnect();
	}

	void ScriptResources::OnResourceLoaded(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::invokeThunk(onResourceLoadedThunk, tmpp0);
	}

	void ScriptResources::OnResourceDestroyed(const UUID& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptUUID::box(p0);
		MonoUtil::invokeThunk(onResourceDestroyedThunk, tmpp0);
	}

	void ScriptResources::OnResourceModified(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::invokeThunk(onResourceModifiedThunk, tmpp0);
	}
	MonoObject* ScriptResources::Internal_load(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = Resources::instance().Load(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::Internal_loadAsync(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = Resources::instance().LoadAsync(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::Internal_loadFromUUID(UUID* uuid, bool async, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		tmp__output = Resources::instance().LoadFromUUID(*uuid, async, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptResources::Internal_release(MonoObject* resource)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->GetHandle());
		Resources::instance().Release(tmpresource);
	}

	void ScriptResources::Internal_unloadAllUnused()
	{
		Resources::instance().UnloadAllUnused();
	}

	void ScriptResources::Internal_unloadAll()
	{
		Resources::instance().UnloadAll();
	}

	void ScriptResources::Internal_save(MonoObject* resource, MonoString* filePath, bool overwrite, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->GetGenericHandle());
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		Resources::instance().Save(tmpresource, tmpfilePath, overwrite, compress);
	}

	void ScriptResources::Internal_save0(MonoObject* resource, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->GetGenericHandle());
		Resources::instance().Save(tmpresource, compress);
	}

	MonoArray* ScriptResources::Internal_getDependencies(MonoString* filePath)
	{
		Vector<UUID> vec__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		vec__output = Resources::instance().GetDependencies(tmpfilePath);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptUUID>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	bool ScriptResources::Internal_isLoaded(UUID* uuid, bool checkInProgress)
	{
		bool tmp__output;
		tmp__output = Resources::instance().IsLoaded(*uuid, checkInProgress);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptResources::Internal_getLoadProgress(MonoObject* resource, bool includeDependencies)
	{
		float tmp__output;
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->GetHandle());
		tmp__output = Resources::instance().GetLoadProgress(tmpresource, includeDependencies);

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptResources::Internal_registerResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::toNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->GetInternal();
		Resources::instance().RegisterResourceManifest(tmpmanifest);
	}

	void ScriptResources::Internal_unregisterResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::toNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->GetInternal();
		Resources::instance().UnregisterResourceManifest(tmpmanifest);
	}

	MonoObject* ScriptResources::Internal_getResourceManifest(MonoString* name)
	{
		SPtr<ResourceManifest> tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = Resources::instance().GetResourceManifest(tmpname);

		MonoObject* __output;
		__output = ScriptResourceManifest::create(tmp__output);

		return __output;
	}

	bool ScriptResources::Internal_getFilePathFromUUID(UUID* uuid, MonoString** filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmp__output = Resources::instance().GetFilePathFromUUID(*uuid, tmpfilePath);

		bool __output;
		__output = tmp__output;
		MonoUtil::referenceCopy(filePath,  (MonoObject*)MonoUtil::stringToMono(tmpfilePath.ToString()));

		return __output;
	}

	bool ScriptResources::Internal_getUUIDFromFilePath(MonoString* path, UUID* uuid)
	{
		bool tmp__output;
		Path tmppath;
		tmppath = MonoUtil::monoToString(path);
		tmp__output = Resources::instance().GetUUIDFromFilePath(tmppath, *uuid);

		bool __output;
		__output = tmp__output;

		return __output;
	}
#endif
}
