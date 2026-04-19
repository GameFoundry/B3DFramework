//*********************************** B3D Framework - Copyright 2026 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGameObjectWrapper.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"

using namespace b3d;

MonoObject* ScriptGameObjectWrapper::GetOrCreateScriptObject(const HGameObject& nativeObject)
{
	if(!nativeObject.IsValid())
		return nullptr;

	const u32 rttiId = nativeObject->GetTypeId();
	const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData == nullptr)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Mapping between a game object and a managed type is missing for type \"{0}\"", rttiId);
		return nullptr;
	}

	if(scriptWrapperObjectMetaData->CreateCallbackType != ScriptWrapperCreateCallbackType::GameObject)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Script wrapper for type \"{0}\" does not support creation of a GameObject handle.", rttiId);
		return nullptr;
	}

	if(!B3D_ENSURE(scriptWrapperObjectMetaData->GetScriptExportable != nullptr))
		return nullptr;

	IScriptExportable* const scriptExportableObject = scriptWrapperObjectMetaData->GetScriptExportable(nativeObject.Get());
	if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)scriptExportableObject->GetScriptObjectWrapper())
		return scriptObjectWrapper->GetScriptObject();

	return scriptWrapperObjectMetaData->GameObjectCreateCallback(nativeObject);
}

ScriptGameObjectWrapper* ScriptGameObjectWrapper::GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject)
{
	ScriptGameObjectWrapper* scriptObjectWrapper = nullptr;

	if(wrapperMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
		wrapperMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

	return scriptObjectWrapper;
}

ScriptGameObject::ScriptGameObject(const HGameObject& nativeObject)
	: TScriptGameObjectWrapper(nativeObject)
{}

void ScriptGameObject::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetId", (void*)&ScriptGameObject::Internal_GetId);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsDestroyed", (void*)&ScriptGameObject::Internal_IsDestroyed);
}

void ScriptGameObject::Internal_GetId(ScriptGameObject* nativeInstance, UUID* outId)
{
	if(!nativeInstance->IsNativeObjectValid())
	{
		*outId = UUID::kEmpty;
		return;
	}

	*outId = nativeInstance->GetNativeObjectAsHandle()->GetId();
}

bool ScriptGameObject::Internal_IsDestroyed(ScriptGameObject* nativeInstance)
{
	return nativeInstance->GetNativeObjectAsHandle().IsDestroyed(true);
}
