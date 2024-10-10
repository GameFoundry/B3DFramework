//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableList.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"
#include "BsMonoUtil.h"

using namespace bs;
ScriptSerializableList::ScriptSerializableList(MonoObject* instance, const SPtr<ManagedTypeInfoList>& typeInfo)
	: ScriptObject(instance), mTypeInfo(typeInfo)
{
}

void ScriptSerializableList::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableList::InternalCreateProperty);
}

MonoObject* ScriptSerializableList::Create(const ScriptSerializableProperty* native, MonoObject* managed)
{
	SPtr<ManagedTypeInfoList> listTypeInfo =
		std::static_pointer_cast<ManagedTypeInfoList>(native->GetTypeInfo());
	MonoReflectionType* internalElementType = MonoUtil::GetType(listTypeInfo->ElementType->GetMonoClass());

	void* params[2] = { internalElementType, managed };
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(params, 2);

	new(B3DAllocate<ScriptSerializableList>()) ScriptSerializableList(managedInstance, listTypeInfo);
	return managedInstance;
}

MonoObject* ScriptSerializableList::InternalCreateProperty(ScriptSerializableList* nativeInstance)
{
	return ScriptSerializableProperty::Create(nativeInstance->mTypeInfo->ElementType);
}
