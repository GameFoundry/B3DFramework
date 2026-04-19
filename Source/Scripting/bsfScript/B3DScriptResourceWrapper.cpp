//*********************************** B3D Framework - Copyright 2026 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptResourceWrapper.h"

#include "B3DManagedResource.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Wrappers/B3DScriptManagedResource.h"
#include "Wrappers/B3DScriptRRefBase.h"

using namespace b3d;

MonoObject* ScriptResourceWrapper::GetOrCreateScriptObject(const HResource& nativeObject)
{
	if(!nativeObject.IsValid())
		return nullptr;

	const u32 rttiId = nativeObject->GetTypeId();
	const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData == nullptr)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Mapping between a resource and a managed type is missing for type \"{0}\"", rttiId);
		return nullptr;
	}

	if(scriptWrapperObjectMetaData->CreateCallbackType != ScriptWrapperCreateCallbackType::Resource)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Script wrapper for type \"{0}\" does not support creation of a Resource handle.", rttiId);
		return nullptr;
	}

	if(!B3D_ENSURE(scriptWrapperObjectMetaData->GetScriptExportable != nullptr))
		return nullptr;

	IScriptExportable* const scriptExportableObject = scriptWrapperObjectMetaData->GetScriptExportable(nativeObject.Get());
	if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)scriptExportableObject->GetScriptObjectWrapper())
		return scriptObjectWrapper->GetScriptObject();

	return scriptWrapperObjectMetaData->ResourceCreateCallback(nativeObject);
}

ScriptResourceWrapper* ScriptResourceWrapper::GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject)
{
	ScriptResourceWrapper* scriptObjectWrapper = nullptr;

	if(wrapperMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
		wrapperMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

	return scriptObjectWrapper;
}

MonoObject* ScriptResourceWrapper::GetOrCreateResourceReference(const HResource& resource, u32 rttiId)
{
	::MonoClass* const resourceReferenceScriptClass = GetResourceReferenceScriptClass(rttiId);
	if(!resourceReferenceScriptClass)
		return nullptr;

	ScriptRRefBase* resourceReferenceScriptObject = ScriptResourceManager::Instance().GetScriptRRef(resource, resourceReferenceScriptClass);
	if(!resourceReferenceScriptObject)
		return nullptr;

	return resourceReferenceScriptObject->GetScriptObject();
}

::MonoClass* ScriptResourceWrapper::GetResourceScriptClass(u32 rttiId)
{
	if(rttiId == Resource::GetRttiStatic()->GetRttiId())
		return ScriptResource::GetMetaData()->ScriptClass->GetInternalClass();
	else if(rttiId == ManagedResource::GetRttiStatic()->GetRttiId())
		return ScriptManagedResource::GetMetaData()->ScriptClass->GetInternalClass();
	else
	{
		const ScriptTypeMetaData* const metaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);

		if(metaData == nullptr)
			return nullptr;

		return metaData->ScriptClass->GetInternalClass();
	}
}

::MonoClass* ScriptResourceWrapper::GetResourceReferenceScriptClass(u32 rttiId)
{
	::MonoClass* const resourceScriptClass = GetResourceScriptClass(rttiId);
	if(!resourceScriptClass)
		return nullptr;

	return ScriptRRefBase::BindGenericParam(resourceScriptClass);
}

ScriptResource::ScriptResource(const HResource& nativeObject)
	:TScriptResourceWrapper(nativeObject)
{ }

void ScriptResource::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptResource::InternalGetName);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptResource::InternalGetUuid);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Release", (void*)&ScriptResource::InternalRelease);
}

Resource* ScriptResource::GetNativeObject() const
{
	return static_cast<Resource*>(TScriptResourceWrapper::GetNativeObject());
}

MonoString* ScriptResource::InternalGetName(ScriptResourceWrapper* self)
{
	return MonoUtil::StringToMono(static_cast<Resource*>(self->GetNativeObject())->GetName());
}

void ScriptResource::InternalGetUuid(ScriptResourceWrapper* self, UUID* uuid)
{
	*uuid = self->GetBaseNativeObjectAsHandle().GetId();
}

void ScriptResource::InternalRelease(ScriptResourceWrapper* self)
{
	HResource mutableResourceHandle = self->GetBaseNativeObjectAsHandle();
	mutableResourceHandle.ReleaseInternalReference();
}

ScriptUUID::ScriptUUID()
	: TScriptTypeDefinition()
{}

MonoObject* ScriptUUID::Box(const UUID& value)
{
	// We're casting away const but it's fine since structs are passed by value anyway
	return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
}

UUID ScriptUUID::Unbox(MonoObject* obj)
{
	return *(UUID*)MonoUtil::Unbox(obj);
}
