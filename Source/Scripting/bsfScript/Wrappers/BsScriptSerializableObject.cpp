//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableObject.h"
#include "Wrappers/BsScriptSerializableField.h"
#include "Wrappers/BsScriptSerializableProperty.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

using namespace bs;
MonoField* ScriptSerializableObject::FieldsField = nullptr;

ScriptSerializableObject::ScriptSerializableObject(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
	: ScriptObject(instance), mObjInfo(objInfo)
{
}

void ScriptSerializableObject::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSerializableObject::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_GetBaseClass", (void*)&ScriptSerializableObject::InternalGetBaseClass);

	FieldsField = metaData.ScriptClass->GetField("_fields");
}

MonoObject* ScriptSerializableObject::Create(const ScriptSerializableProperty* native, MonoObject* managed, MonoReflectionType* reflType)
{
	void* params[2] = { reflType, managed };
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("Type,SerializableProperty", params);

	return managedInstance;
}

MonoObject* ScriptSerializableObject::Create(MonoObject* managed, MonoReflectionType* reflType)
{
	void* params[2] = { reflType, managed };
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("Type,object", params);

	return managedInstance;
}

void ScriptSerializableObject::InternalCreateInstance(MonoObject* instance, MonoReflectionType* type)
{
	::MonoClass* monoClass = MonoUtil::GetClass(type);

	String elementNs;
	String elementTypeName;
	MonoUtil::GetClassName(monoClass, elementNs, elementTypeName);

	SPtr<ManagedSerializableObjectInfo> objInfo;
	ScriptAssemblyManager::Instance().GetSerializableObjectInfo(elementNs, elementTypeName, objInfo);

	CreateInternal(instance, objInfo);
}

MonoObject* ScriptSerializableObject::InternalGetBaseClass(ScriptSerializableObject* thisPtr, MonoObject* owningObject)
{
	if(!thisPtr->mObjInfo->BaseClass)
		return nullptr;

	MonoReflectionType* reflType = MonoUtil::GetType(thisPtr->mObjInfo->BaseClass->ScriptClass->GetInternalClass());
	return Create(owningObject, reflType);
}

ScriptSerializableObject* ScriptSerializableObject::CreateInternal(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
{
	ScriptSerializableObject* nativeInstance = new(B3DAllocate<ScriptSerializableObject>()) ScriptSerializableObject(instance, objInfo);

	Vector<SPtr<ManagedSerializableMemberInfo>> sortedFields;

	if(objInfo != nullptr)
	{
		sortedFields.resize(objInfo->Fields.size());
		u32 i = 0;
		for(auto& fieldPair : objInfo->Fields)
		{
			sortedFields[i] = fieldPair.second;
			i++;
		}
	}

	std::sort(sortedFields.begin(), sortedFields.end(), [&](const SPtr<ManagedSerializableMemberInfo>& x, const SPtr<ManagedSerializableMemberInfo>& y)
			  { return x->FieldId < y->FieldId; });

	::MonoClass* serializableFieldClass = ScriptSerializableField::GetMetaData()->ScriptClass->GetInternalClass();
	ScriptArray scriptArray(serializableFieldClass, (u32)sortedFields.size());

	u32 i = 0;
	for(auto& field : sortedFields)
	{
		MonoObject* fieldManagedInstance = ScriptSerializableField::Create(instance, field);

		scriptArray.Set(i, fieldManagedInstance);
		i++;
	}

	FieldsField->Set(instance, scriptArray.GetInternal());

	return nativeInstance;
}
