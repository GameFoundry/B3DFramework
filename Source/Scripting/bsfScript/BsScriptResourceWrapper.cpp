//*********************************** bs::framework - Copyright 2024 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceWrapper.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"

using namespace bs;

MonoObject* ScriptResourceWrapper::GetOrCreateScriptObject(const HResource& nativeObject)
{
	if(!nativeObject.IsValid())
		return nullptr;

	const u32 rttiId = nativeObject->GetTypeId();
	const ScriptWrapperObjectMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData == nullptr)
	{
		B3D_LOG(Error, Script, "Cannot retrieve script object. Mapping between a resource and a managed type is missing for type \"{0}\"", rttiId);
		return nullptr;
	}

	if(scriptWrapperObjectMetaData->CreateCallbackType != ScriptWrapperCreateCallbackType::Resource)
	{
		B3D_LOG(Error, Script, "Cannot retrieve script object. Script wrapper for type \"{0}\" does not support creation of a Resource handle.", rttiId);
		return nullptr;
	}

	if(!B3D_ENSURE(scriptWrapperObjectMetaData->GetScriptExportable != nullptr))
		return nullptr;

	IScriptExportable* const scriptExportableObject = scriptWrapperObjectMetaData->GetScriptExportable(nativeObject.Get());
	if(ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)scriptExportableObject->GetScriptObjectWrapper())
		return scriptObjectWrapper->GetScriptObject();

	return scriptWrapperObjectMetaData->ResourceCreateCallback(nativeObject);
}

ScriptResourceWrapper* ScriptResourceWrapper::GetScriptObjectWrapper(const ScriptWrapperObjectMetaData& wrapperMetaData, MonoObject* scriptObject)
{
	ScriptResourceWrapper* scriptObjectWrapper = nullptr;

	if(wrapperMetaData.ScriptObjectWrapperPointerField != nullptr && scriptObject != nullptr)
		wrapperMetaData.ScriptObjectWrapperPointerField->Get(scriptObject, &scriptObjectWrapper);

	return scriptObjectWrapper;
}
