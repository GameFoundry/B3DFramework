//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
#if !B3D_IS_ENGINE
	ScriptResources::OnResourceLoadedThunkDef ScriptResources::OnResourceLoadedThunk; 
	ScriptResources::OnResourceDestroyedThunkDef ScriptResources::OnResourceDestroyedThunk; 
	ScriptResources::OnResourceModifiedThunkDef ScriptResources::OnResourceModifiedThunk; 

	HEvent ScriptResources::OnResourceLoadedConn;
	HEvent ScriptResources::OnResourceDestroyedConn;
	HEvent ScriptResources::OnResourceModifiedConn;

	ScriptResources::ScriptResources(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
		mGCHandle = MonoUtil::NewWeakGcHandle(managedInstance);
	}

	void ScriptResources::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Load", (void*)&ScriptResources::InternalLoad);
		metaData.ScriptClass->AddInternalCall("Internal_LoadAsync", (void*)&ScriptResources::InternalLoadAsync);
		metaData.ScriptClass->AddInternalCall("Internal_LoadFromUuid", (void*)&ScriptResources::InternalLoadFromUuid);
		metaData.ScriptClass->AddInternalCall("Internal_Release", (void*)&ScriptResources::InternalRelease);
		metaData.ScriptClass->AddInternalCall("Internal_UnloadAllUnused", (void*)&ScriptResources::InternalUnloadAllUnused);
		metaData.ScriptClass->AddInternalCall("Internal_UnloadAll", (void*)&ScriptResources::InternalUnloadAll);
		metaData.ScriptClass->AddInternalCall("Internal_Save", (void*)&ScriptResources::InternalSave);
		metaData.ScriptClass->AddInternalCall("Internal_Save0", (void*)&ScriptResources::InternalSave0);
		metaData.ScriptClass->AddInternalCall("Internal_GetDependencies", (void*)&ScriptResources::InternalGetDependencies);
		metaData.ScriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptResources::InternalIsLoaded);
		metaData.ScriptClass->AddInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::InternalGetLoadProgress);
		metaData.ScriptClass->AddInternalCall("Internal_RegisterResourceManifest", (void*)&ScriptResources::InternalRegisterResourceManifest);
		metaData.ScriptClass->AddInternalCall("Internal_UnregisterResourceManifest", (void*)&ScriptResources::InternalUnregisterResourceManifest);
		metaData.ScriptClass->AddInternalCall("Internal_GetResourceManifest", (void*)&ScriptResources::InternalGetResourceManifest);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilePathFromUuid", (void*)&ScriptResources::InternalGetFilePathFromUuid);
		metaData.ScriptClass->AddInternalCall("Internal_GetUuidFromFilePath", (void*)&ScriptResources::InternalGetUuidFromFilePath);

		OnResourceLoadedThunk = (OnResourceLoadedThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnResourceLoaded", "RRefBase")->GetThunk();
		OnResourceDestroyedThunk = (OnResourceDestroyedThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnResourceDestroyed", "UUID&")->GetThunk();
		OnResourceModifiedThunk = (OnResourceModifiedThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnResourceModified", "RRefBase")->GetThunk();
	}

	void ScriptResources::StartUp()
	{
		OnResourceLoadedConn = Resources::Instance().OnResourceLoaded.Connect(&ScriptResources::OnResourceLoaded);
		OnResourceDestroyedConn = Resources::Instance().OnResourceDestroyed.Connect(&ScriptResources::OnResourceDestroyed);
		OnResourceModifiedConn = Resources::Instance().OnResourceModified.Connect(&ScriptResources::OnResourceModified);
	}
	void ScriptResources::ShutDown()
	{
		OnResourceLoadedConn.Disconnect();
		OnResourceDestroyedConn.Disconnect();
		OnResourceModifiedConn.Disconnect();
	}

	void ScriptResources::OnResourceLoaded(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::InvokeThunk(OnResourceLoadedThunk, tmpp0);
	}

	void ScriptResources::OnResourceDestroyed(const UUID& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptUUID::Box(p0);
		MonoUtil::InvokeThunk(OnResourceDestroyedThunk, tmpp0);
	}

	void ScriptResources::OnResourceModified(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::InvokeThunk(OnResourceModifiedThunk, tmpp0);
	}

	MonoObject* ScriptResources::InternalLoad(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		tmp__output = Resources::Instance().Load(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::InternalLoadAsync(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		tmp__output = Resources::Instance().LoadAsync(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::InternalLoadFromUuid(UUID* uuid, bool async, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		tmp__output = Resources::Instance().LoadFromUuid(*uuid, async, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptResources::InternalRelease(MonoObject* resource)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::ToNative(resource);
		if(scriptresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptresource->GetHandle());
		Resources::Instance().Release(tmpresource);
	}

	void ScriptResources::InternalUnloadAllUnused()
	{
		Resources::Instance().UnloadAllUnused();
	}

	void ScriptResources::InternalUnloadAll()
	{
		Resources::Instance().UnloadAll();
	}

	void ScriptResources::InternalSave(MonoObject* resource, MonoString* filePath, bool overwrite, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::ToNative(resource);
		if(scriptresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptresource->GetGenericHandle());
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		Resources::Instance().Save(tmpresource, tmpfilePath, overwrite, compress);
	}

	void ScriptResources::InternalSave0(MonoObject* resource, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::ToNative(resource);
		if(scriptresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptresource->GetGenericHandle());
		Resources::Instance().Save(tmpresource, compress);
	}

	MonoArray* ScriptResources::InternalGetDependencies(MonoString* filePath)
	{
		Vector<UUID> vec__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		vec__output = Resources::Instance().GetDependencies(tmpfilePath);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptUUID>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	bool ScriptResources::InternalIsLoaded(UUID* uuid, bool checkInProgress)
	{
		bool tmp__output;
		tmp__output = Resources::Instance().IsLoaded(*uuid, checkInProgress);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptResources::InternalGetLoadProgress(MonoObject* resource, bool includeDependencies)
	{
		float tmp__output;
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::ToNative(resource);
		if(scriptresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptresource->GetHandle());
		tmp__output = Resources::Instance().GetLoadProgress(tmpresource, includeDependencies);

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptResources::InternalRegisterResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::ToNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->GetInternal();
		Resources::Instance().RegisterResourceManifest(tmpmanifest);
	}

	void ScriptResources::InternalUnregisterResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::ToNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->GetInternal();
		Resources::Instance().UnregisterResourceManifest(tmpmanifest);
	}

	MonoObject* ScriptResources::InternalGetResourceManifest(MonoString* name)
	{
		SPtr<ResourceManifest> tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = Resources::Instance().GetResourceManifest(tmpname);

		MonoObject* __output;
		__output = ScriptResourceManifest::Create(tmp__output);

		return __output;
	}

	bool ScriptResources::InternalGetFilePathFromUuid(UUID* uuid, MonoString** filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmp__output = Resources::Instance().GetFilePathFromUuid(*uuid, tmpfilePath);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(filePath,  (MonoObject*)MonoUtil::StringToMono(tmpfilePath.ToString()));

		return __output;
	}

	bool ScriptResources::InternalGetUuidFromFilePath(MonoString* path, UUID* uuid)
	{
		bool tmp__output;
		Path tmppath;
		tmppath = MonoUtil::MonoToString(path);
		tmp__output = Resources::Instance().GetUuidFromFilePath(tmppath, *uuid);

		bool __output;
		__output = tmp__output;

		return __output;
	}
#endif
}
