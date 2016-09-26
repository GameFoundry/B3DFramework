//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableObject.h"
#include "BsScriptSerializableField.h"
#include "BsScriptSerializableProperty.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	MonoField* ScriptSerializableObject::FieldsField = nullptr;

	ScriptSerializableObject::ScriptSerializableObject(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo)
		:ScriptObject(instance), mTypeInfo(typeInfo)
	{

	}

	void ScriptSerializableObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSerializableObject::internal_createInstance);

		FieldsField = metaData.scriptClass->getField("_fields");
	}

	ScriptSerializableObject* ScriptSerializableObject::create(const ScriptSerializableProperty* property)
	{
		MonoReflectionType* internalElementType = MonoUtil::getType(property->getTypeInfo()->getMonoClass());

		void* params[2] = { internalElementType, property->getManagedInstance() };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 2);

		// Managed constructor will call back to native which will create ScriptSerializableObject instance,
		// and we can now just retrieve it.
		return ScriptSerializableObject::toNative(managedInstance);
	}

	void ScriptSerializableObject::internal_createInstance(MonoObject* instance, MonoReflectionType* type)
	{
		::MonoClass* monoClass = MonoUtil::getClass(type);

		String elementNs;
		String elementTypeName;
		MonoUtil::getClassName(monoClass, elementNs, elementTypeName);

		SPtr<ManagedSerializableObjectInfo> objInfo;
		ScriptAssemblyManager::instance().getSerializableObjectInfo(elementNs, elementTypeName, objInfo);

		createInternal(instance, objInfo);
	}

	ScriptSerializableObject* ScriptSerializableObject::createInternal(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
	{
		SPtr<ManagedSerializableTypeInfo> typeInfo;
		if(objInfo != nullptr)
			typeInfo = objInfo->mTypeInfo;

		ScriptSerializableObject* nativeInstance = new (bs_alloc<ScriptSerializableObject>()) ScriptSerializableObject(instance, typeInfo);

		Vector<SPtr<ManagedSerializableMemberInfo>> sortedFields;
		
		if(objInfo != nullptr)
		{
			sortedFields.resize(objInfo->mFields.size());
			UINT32 i = 0;
			for (auto& fieldPair : objInfo->mFields)
			{
				sortedFields[i] = fieldPair.second;
				i++;
			}
		}

		std::sort(sortedFields.begin(), sortedFields.end(),
			[&](const SPtr<ManagedSerializableMemberInfo>& x, const SPtr<ManagedSerializableMemberInfo>& y)
		{
			return x->mFieldId < y->mFieldId;
		});

		::MonoClass* serializableFieldClass = ScriptSerializableField::getMetaData()->scriptClass->_getInternalClass();
		ScriptArray scriptArray(serializableFieldClass, (UINT32)sortedFields.size());

		UINT32 i = 0;
		for (auto& field : sortedFields)
		{
			ScriptSerializableField* serializableField = ScriptSerializableField::create(instance, field);
			MonoObject* fieldManagedInstance = serializableField->getManagedInstance();

			scriptArray.set(i, fieldManagedInstance);
			i++;
		}

		FieldsField->setValue(instance, scriptArray.getInternal());

		return nativeInstance;
	}
}