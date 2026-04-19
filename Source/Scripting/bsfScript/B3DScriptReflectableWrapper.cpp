//*********************************** B3D Framework - Copyright 2026 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptReflectableWrapper.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"

using namespace b3d;

MonoObject* ScriptReflectableWrapper::GetOrCreateScriptObject(const SPtr<IReflectable>& nativeObject)
{
	if(nativeObject == nullptr)
		return nullptr;

	const u32 rttiId = nativeObject->GetTypeId();
	const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData == nullptr)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Mapping between a reflectable object and a managed type is missing for type \"{0}\"", rttiId);
		return nullptr;
	}

	if(scriptWrapperObjectMetaData->CreateCallbackType != ScriptWrapperCreateCallbackType::Reflectable)
	{
		B3D_LOG(Error, LogScript, "Cannot retrieve script object. Script wrapper for type \"{0}\" does not support creation of an IReflectable shared pointer.", rttiId);
		return nullptr;
	}

	if(!B3D_ENSURE(scriptWrapperObjectMetaData->GetScriptExportable != nullptr))
		return nullptr;

	IScriptExportable* const scriptExportableObject = scriptWrapperObjectMetaData->GetScriptExportable(nativeObject.get());
	if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)scriptExportableObject->GetScriptObjectWrapper())
		return scriptObjectWrapper->GetScriptObject();

	return scriptWrapperObjectMetaData->ReflectableCreateCallback(nativeObject);
}

ScriptReflectableWrapper* ScriptReflectableWrapper::GetScriptObjectWrapper(const ScriptTypeMetaData& wrapperMetaData, MonoObject* scriptObject)
{
	ScriptReflectableWrapper* scriptObjectWrapper = nullptr;

	if(wrapperMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
		wrapperMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

	return scriptObjectWrapper;
}
