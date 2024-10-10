//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableArray.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"

using namespace bs;
ScriptSerializableArray::ScriptSerializableArray(MonoObject* instance, const SPtr<ManagedTypeInfoArray>& typeInfo)
	: ScriptObject(instance), mTypeInfo(typeInfo)
{
}

void ScriptSerializableArray::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableArray::InternalCreateProperty);
}

MonoObject* ScriptSerializableArray::Create(const ScriptSerializableProperty* native, MonoObject* managed)
{
	SPtr<ManagedTypeInfoArray> arrayTypeInfo =
		std::static_pointer_cast<ManagedTypeInfoArray>(native->GetTypeInfo());

	MonoReflectionType* internalElementType = MonoUtil::GetType(arrayTypeInfo->ElementType->GetMonoClass());

	void* params[2] = { internalElementType, managed };
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(params, 2);

	new(B3DAllocate<ScriptSerializableArray>()) ScriptSerializableArray(managedInstance, arrayTypeInfo);

	return managedInstance;
}

MonoObject* ScriptSerializableArray::InternalCreateProperty(ScriptSerializableArray* nativeInstance)
{
	return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->ElementType);
}
