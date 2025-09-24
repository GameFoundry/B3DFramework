//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptResources.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Resources/B3DResources.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "Resources/B3DResource.h"
#include "B3DScriptResourceLoadOptions.generated.h"
#include "B3DScriptResourceWrapper.h"

namespace b3d
{
#if B3D_IS_ENGINE
	ScriptResources::OnResourceLoadedThunkDefinition ScriptResources::OnResourceLoadedThunk; 
	ScriptResources::OnResourceDestroyedThunkDefinition ScriptResources::OnResourceDestroyedThunk; 
	ScriptResources::OnResourceModifiedThunkDefinition ScriptResources::OnResourceModifiedThunk; 

	HEvent ScriptResources::OnResourceLoadedConnection;
	HEvent ScriptResources::OnResourceDestroyedConnection;
	HEvent ScriptResources::OnResourceModifiedConnection;

	ScriptResources::ScriptResources()
		:TScriptTypeDefinition()
	{
	}

	void ScriptResources::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Load", (void*)&ScriptResources::InternalLoad);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Load0", (void*)&ScriptResources::InternalLoad0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Exists", (void*)&ScriptResources::InternalExists);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Exists0", (void*)&ScriptResources::InternalExists0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ReleaseInternalReference", (void*)&ScriptResources::InternalReleaseInternalReference);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnloadAllUnused", (void*)&ScriptResources::InternalUnloadAllUnused);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnloadAll", (void*)&ScriptResources::InternalUnloadAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptResources::InternalIsLoaded);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::InternalGetLoadProgress);

		OnResourceLoadedThunk = (OnResourceLoadedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnResourceLoaded", "RRefBase")->GetThunk();
		OnResourceDestroyedThunk = (OnResourceDestroyedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnResourceDestroyed", "UUID&")->GetThunk();
		OnResourceModifiedThunk = (OnResourceModifiedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnResourceModified", "RRefBase")->GetThunk();
	}

	void ScriptResources::StartUp()
	{
		OnResourceLoadedConnection = Resources::Instance().OnResourceLoaded.Connect(&ScriptResources::OnResourceLoaded);
		OnResourceDestroyedConnection = Resources::Instance().OnResourceDestroyed.Connect(&ScriptResources::OnResourceDestroyed);
		OnResourceModifiedConnection = Resources::Instance().OnResourceModified.Connect(&ScriptResources::OnResourceModified);
	}
	void ScriptResources::ShutDown()
	{
		OnResourceLoadedConnection.Disconnect();
		OnResourceDestroyedConnection.Disconnect();
		OnResourceModifiedConnection.Disconnect();
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

	MonoObject* ScriptResources::InternalLoad(MonoString* resourcePath, ResourceLoadOptions* loadOptions)
	{
		TResourceHandle<Resource> tmp__output;
		Path tmpresourcePath;
		tmpresourcePath = MonoUtil::MonoToString(resourcePath);
		tmp__output = Resources::Instance().Load(tmpresourcePath, *loadOptions);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::InternalLoad0(UUID* resourceId, ResourceLoadOptions* loadOptions)
	{
		TResourceHandle<Resource> tmp__output;
		tmp__output = Resources::Instance().Load(*resourceId, *loadOptions);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	bool ScriptResources::InternalExists(MonoString* resourcePath)
	{
		bool tmp__output;
		Path tmpresourcePath;
		tmpresourcePath = MonoUtil::MonoToString(resourcePath);
		tmp__output = Resources::Instance().Exists(tmpresourcePath);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResources::InternalExists0(UUID* resourceId)
	{
		bool tmp__output;
		tmp__output = Resources::Instance().Exists(*resourceId);

		bool __output;
		__output = tmp__output;

		return __output;
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
