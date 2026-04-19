//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DManagedTypeUtility.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "Serialization/B3DManagedSerializableArray.h"
#include "Serialization/B3DManagedSerializableDictionary.h"
#include "Serialization/B3DManagedSerializableList.h"
#include "Serialization/B3DManagedSerializableObject.h"

using namespace b3d;

SPtr<ManagedTypeInfo> ManagedTypeUtility::GetTypeInfo(MonoReflectionType* objectType)
{
	if(objectType == nullptr)
		return nullptr;

	::MonoClass* monoClass = MonoUtil::GetClass(objectType);
	MonoClass* scriptClass = MonoManager::Instance().FindClass(monoClass);

	return ScriptAssemblyManager::Instance().GetTypeInfo(scriptClass);
}

SPtr<ManagedObjectInfo> ManagedTypeUtility::GetSerializableObjectInfo(MonoReflectionType* objectType)
{
	if(objectType == nullptr)
		return nullptr;

	return ScriptAssemblyManager::Instance().GetSerializableObjectInfo(objectType);
}

u32 ManagedTypeUtility::GetRTTITypeId(MonoReflectionType* objectType)
{
	if(objectType == nullptr)
		return ~0u;

	const ScriptTypeMetaData* const scriptObjectWrapperMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(objectType);
	if(scriptObjectWrapperMetaData == nullptr)
		return ~0u;

	return scriptObjectWrapperMetaData->TypeId;
}

MonoObject* ManagedTypeUtility::CreateSerializableObject(const SPtr<ManagedTypeInfoObject>& typeInfo)
{
	if(typeInfo == nullptr)
		return nullptr;

	return ManagedSerializableObject::CreateManagedInstance(typeInfo);
}

MonoObject* ManagedTypeUtility::CreateArray(const SPtr<ManagedTypeInfoArray>& typeInfo, const Vector<u32>& arraySizes)
{
	if(typeInfo == nullptr)
		return nullptr;

	return ManagedSerializableArray::CreateManagedInstance(typeInfo, arraySizes);
}

MonoObject* ManagedTypeUtility::CreateList(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size)
{
	if(typeInfo == nullptr)
		return nullptr;

	return ManagedSerializableList::CreateManagedInstance(typeInfo, size);
}

MonoObject* ManagedTypeUtility::CreateDictionary(const SPtr<ManagedTypeInfoDictionary>& typeInfo)
{
	if(typeInfo == nullptr)
		return nullptr;

	return ManagedSerializableDictionary::CreateManagedInstance(typeInfo);
}

MonoObject* ManagedTypeUtility::CloneObject(MonoObject* original)
{
	if(original == nullptr)
		return nullptr;

	::MonoClass* monoClass = MonoUtil::GetClass(original);
	MonoClass* engineClass = MonoManager::Instance().FindClass(monoClass);

	SPtr<ManagedTypeInfo> typeInfo = ScriptAssemblyManager::Instance().GetTypeInfo(engineClass);
	if(typeInfo == nullptr)
	{
		B3D_LOG(Warning, LogScript, "Cannot clone an instance of type \"{0}\", it is not marked as serializable.", engineClass->GetFullName());
		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::Create(typeInfo, original);
	BinarySerializer bs;

	// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized clone method that does it directly,
	// but didn't feel the extra code was justified.
	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	bs.Encode(data.get(), stream);

	stream->Seek(0);
	SPtr<ManagedSerializableFieldData> clonedData = std::static_pointer_cast<ManagedSerializableFieldData>(bs.Decode(stream, (u32)stream->Size()));
	clonedData->Deserialize();

	return clonedData->GetValueBoxed(typeInfo);
}

MonoObject* ManagedTypeUtility::CreateObjectOfType(MonoReflectionType* reflType)
{
	if(reflType == nullptr)
		return nullptr;

	::MonoClass* monoClass = MonoUtil::GetClass(reflType);
	MonoClass* engineClass = MonoManager::Instance().FindClass(monoClass);

	SPtr<ManagedTypeInfo> typeInfo = ScriptAssemblyManager::Instance().GetTypeInfo(engineClass);
	if(typeInfo == nullptr)
	{
		B3D_LOG(Warning, LogScript, "Cannot create an instance of type \"{0}\", it is not marked as serializable.", engineClass->GetFullName());
		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> data = ManagedSerializableFieldData::CreateDefault(typeInfo);
	BinarySerializer bs;

	// Note: This code unnecessarily encodes to binary and decodes from it. I could have added a specialized create method that does it directly,
	// but didn't feel the extra code was justified.
	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	bs.Encode(data.get(), stream);

	stream->Seek(0);
	SPtr<ManagedSerializableFieldData> createdData = std::static_pointer_cast<ManagedSerializableFieldData>(bs.Decode(stream, (u32)stream->Size()));
	createdData->Deserialize();

	return createdData->GetValueBoxed(typeInfo);
}
