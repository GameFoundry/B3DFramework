//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	ScriptSerializableUtility::ScriptSerializableUtility(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptSerializableUtility::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Clone", (void*)&ScriptSerializableUtility::InternalClone);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSerializableUtility::InternalCreate);
	}

	MonoObject* ScriptSerializableUtility::InternalClone(MonoObject* original)
	{
		if(original == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::GetClass(original);
		MonoClass* engineClass = MonoManager::Instance().FindClass(monoClass);

		SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::Instance().GetTypeInfo(engineClass);
		if(typeInfo == nullptr)
		{
			BS_LOG(Warning, Script, "Cannot clone an instance of type \"{0}\", it is not marked as serializable.", engineClass->GetFullName());
			return nullptr;
		}

		SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::Create(typeInfo, original);
		BinarySerializer bs;

		// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized clone method that does it directly,
		// but didn't feel the extra code was justified.
		SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
		bs.Encode(data.get(), stream);

		stream->Seek(0);
		SPtr<ManagedSerializableFieldData> clonedData = std::static_pointer_cast<ManagedSerializableFieldData>(bs.Decode(stream, (u32)stream->Size()));
		clonedData->Deserialize();

		return clonedData->GetValueBoxed(typeInfo);
	}

	MonoObject* ScriptSerializableUtility::InternalCreate(MonoReflectionType* reflType)
	{
		if(reflType == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::GetClass(reflType);
		MonoClass* engineClass = MonoManager::Instance().FindClass(monoClass);

		SPtr<ManagedSerializableTypeInfo> typeInfo = ScriptAssemblyManager::Instance().GetTypeInfo(engineClass);
		if(typeInfo == nullptr)
		{
			BS_LOG(Warning, Script, "Cannot create an instance of type \"{0}\", it is not marked as serializable.", engineClass->GetFullName());
			return nullptr;
		}

		SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::CreateDefault(typeInfo);
		BinarySerializer bs;

		// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized create method that does it directly,
		// but didn't feel the extra code was justified.
		SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
		bs.Encode(data.get(), stream);

		stream->Seek(0);
		SPtr<ManagedSerializableFieldData> createdData = std::static_pointer_cast<ManagedSerializableFieldData>(bs.Decode(stream, (u32)stream->Size()));
		createdData->Deserialize();

		return createdData->GetValueBoxed(typeInfo);
	}
} // namespace bs
