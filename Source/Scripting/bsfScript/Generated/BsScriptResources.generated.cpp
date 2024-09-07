//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResources.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Resources/BsResource.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptResources::OnResourceLoadedThunkDefinition ScriptResources::OnResourceLoadedThunk; 
	ScriptResources::OnResourceDestroyedThunkDefinition ScriptResources::OnResourceDestroyedThunk; 
	ScriptResources::OnResourceModifiedThunkDefinition ScriptResources::OnResourceModifiedThunk; 

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
		metaData.ScriptClass->AddInternalCall("Internal_ReleaseInternalReference", (void*)&ScriptResources::InternalReleaseInternalReference);
		metaData.ScriptClass->AddInternalCall("Internal_UnloadAllUnused", (void*)&ScriptResources::InternalUnloadAllUnused);
		metaData.ScriptClass->AddInternalCall("Internal_UnloadAll", (void*)&ScriptResources::InternalUnloadAll);
		metaData.ScriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptResources::InternalIsLoaded);
		metaData.ScriptClass->AddInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::InternalGetLoadProgress);

		OnResourceLoadedThunk = (OnResourceLoadedThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnResourceLoaded", "RRefBase")->GetThunk();
		OnResourceDestroyedThunk = (OnResourceDestroyedThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnResourceDestroyed", "UUID&")->GetThunk();
		OnResourceModifiedThunk = (OnResourceModifiedThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnResourceModified", "RRefBase")->GetThunk();
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

	void ScriptResources::OnResourceLoaded(const TResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetScriptObject();
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

	void ScriptResources::OnResourceModified(const TResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetScriptObject();
		else
			tmpp0 = nullptr;
		MonoUtil::InvokeThunk(OnResourceModifiedThunk, tmpp0);
	}

	void ScriptResources::InternalReleaseInternalReference(MonoObject* resource)
	{
		TResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptObjectWrapperresource;
		scriptObjectWrapperresource = ScriptRRefBase::GetScriptObjectWrapper(resource);
		if(scriptObjectWrapperresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptObjectWrapperresource->GetNativeObject());
		Resources::Instance().ReleaseInternalReference(tmpresource);
	}

	void ScriptResources::InternalUnloadAllUnused()
	{
		Resources::Instance().UnloadAllUnused();
	}

	void ScriptResources::InternalUnloadAll()
	{
		Resources::Instance().UnloadAll();
	}

	bool ScriptResources::InternalIsLoaded(UUID* uuid, bool checkInProgress)
	{
		bool tmp__output;
		tmp__output = Resources::Instance().IsLoaded(*uuid, checkInProgress);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptResources::InternalGetLoadProgress(MonoObject* resource)
	{
		float tmp__output;
		TResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptObjectWrapperresource;
		scriptObjectWrapperresource = ScriptRRefBase::GetScriptObjectWrapper(resource);
		if(scriptObjectWrapperresource != nullptr)
			tmpresource = B3DStaticResourceCast<Resource>(scriptObjectWrapperresource->GetNativeObject());
		tmp__output = Resources::Instance().GetLoadProgress(tmpresource);

		float __output;
		__output = tmp__output;

		return __output;
	}
#endif
}
