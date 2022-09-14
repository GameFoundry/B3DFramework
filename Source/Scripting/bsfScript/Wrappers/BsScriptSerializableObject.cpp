//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableObject.h"
#include "Wrappers/BsScriptSerializableField.h"
#include "Wrappers/BsScriptSerializableProperty.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

namespace bs
{
	MonoField* ScriptSerializableObject::FieldsField = nullptr;

	ScriptSerializableObject::ScriptSerializableObject(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
		:ScriptObject(instance), mObjInfo(objInfo)
	{

	}

	void ScriptSerializableObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptSerializableObject::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetBaseClass", (void*)&ScriptSerializableObject::internal_getBaseClass);

		FieldsField = metaData.scriptClass->getField("_fields");
	}

	MonoObject* ScriptSerializableObject::create(const ScriptSerializableProperty* native, MonoObject* managed,
		MonoReflectionType* reflType)
	{
		void* params[2] = { reflType, managed };
		MonoObject* managedInstance = metaData.scriptClass->createInstance("Type,SerializableProperty", params);

		return managedInstance;
	}

	MonoObject* ScriptSerializableObject::create(MonoObject* managed, MonoReflectionType* reflType)
	{
		void* params[2] = { reflType, managed };
		MonoObject* managedInstance = metaData.scriptClass->createInstance("Type,object", params);

		return managedInstance;
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

	MonoObject* ScriptSerializableObject::internal_getBaseClass(ScriptSerializableObject* thisPtr, MonoObject* owningObject)
	{
		if(!thisPtr->mObjInfo->mBaseClass)
			return nullptr;

		MonoReflectionType* reflType = MonoUtil::getType(thisPtr->mObjInfo->mBaseClass->mMonoClass->GetInternalClassInternal());
		return create(owningObject, reflType);
	}

	ScriptSerializableObject* ScriptSerializableObject::createInternal(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
	{
		ScriptSerializableObject* nativeInstance = new (bs_alloc<ScriptSerializableObject>()) ScriptSerializableObject(instance, objInfo);

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

		::MonoClass* serializableFieldClass = ScriptSerializableField::getMetaData()->scriptClass->GetInternalClassInternal();
		ScriptArray scriptArray(serializableFieldClass, (UINT32)sortedFields.size());

		UINT32 i = 0;
		for (auto& field : sortedFields)
		{
			MonoObject* fieldManagedInstance = ScriptSerializableField::create(instance, field);

			scriptArray.set(i, fieldManagedInstance);
			i++;
		}

		FieldsField->set(instance, scriptArray.getInternal());

		return nativeInstance;
	}
}
