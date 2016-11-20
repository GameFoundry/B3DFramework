//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableField.h"
#include "BsMemorySerializer.h"

namespace bs
{
	ScriptSerializableUtility::ScriptSerializableUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptSerializableUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Clone", &ScriptSerializableUtility::internal_Clone);
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptSerializableUtility::internal_Create);
	}

	MonoObject* ScriptSerializableUtility::internal_Clone(MonoObject* original)
	{
		if (original == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::getClass(original);
		MonoClass* engineClass = MonoManager::instance().findClass(monoClass);

		SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::instance().getTypeInfo(engineClass);
		if (typeInfo == nullptr)
		{
			LOGWRN("Cannot clone an instance of type \"" +
				engineClass->getFullName() + "\", it is not marked as serializable.");
			return nullptr;
		}

		SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::create(typeInfo, original);
		MemorySerializer ms;

		// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized clone method that does it directly,
		// but didn't feel the extra code was justified.
		UINT32 size = 0;
		UINT8* encodedData = ms.encode(data.get(), size);
		SPtr<ManagedSerializableFieldData> clonedData = std::static_pointer_cast<ManagedSerializableFieldData>(ms.decode(encodedData, size));
		clonedData->deserialize();

		return clonedData->getValueBoxed(typeInfo);
	}

	MonoObject* ScriptSerializableUtility::internal_Create(MonoReflectionType* reflType)
	{
		if (reflType == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::getClass(reflType);
		MonoClass* engineClass = MonoManager::instance().findClass(monoClass);

		SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::instance().getTypeInfo(engineClass);
		if (typeInfo == nullptr)
		{
			LOGWRN("Cannot create an instance of type \"" + 
				engineClass->getFullName() + "\", it is not marked as serializable.");
			return nullptr;
		}
			
		SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::createDefault(typeInfo);
		MemorySerializer ms;

		// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized create method that does it directly,
		// but didn't feel the extra code was justified.
		UINT32 size = 0;
		UINT8* encodedData = ms.encode(data.get(), size);
		SPtr<ManagedSerializableFieldData> createdData = std::static_pointer_cast<ManagedSerializableFieldData>(ms.decode(encodedData, size));
		createdData->deserialize();

		return createdData->getValueBoxed(typeInfo);
	}
}