//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DManagedResource.h"
#include "RTTI/B3DManagedResourceRTTI.h"
#include "B3DManagedResourceMetaData.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"
#include "Resources/B3DResources.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Wrappers/B3DScriptManagedResource.h"
#include "B3DScriptResourceManager.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Debug/B3DDebug.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;
ManagedResource::ManagedResource()
	: Resource(false)
{}

MonoObject* ManagedResource::GetManagedInstance() const
{
	ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)GetScriptObjectWrapper();
	if(scriptObjectWrapper == nullptr)
		return nullptr;

	return scriptObjectWrapper->GetScriptObject();
}

ResourceBackupData ManagedResource::Backup()
{
	ResourceBackupData backupData;

	if(mObjectInformation != nullptr)
	{
		MonoObject* scriptObject = nullptr;
		ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)GetScriptObjectWrapper();
		if(scriptObjectWrapper != nullptr)
			scriptObject = scriptObjectWrapper->GetScriptObject();

		TShared<ManagedSerializableObject> serializableObject = ManagedSerializableObject::CreateFromExisting(scriptObject);

		if(serializableObject != nullptr)
		{
			TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
			BinarySerializer bs;

			bs.Encode(serializableObject.get(), stream);

			backupData.Size = (u32)stream->Size();
			backupData.Data = stream->DisownMemory();
		}
		else
		{
			backupData.Size = 0;
			backupData.Data = nullptr;
		}
	}
	else
	{
		TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();

		if(mSerializedObjectData != nullptr)
		{
			BinarySerializer bs;
			bs.Encode(mSerializedObjectData.get(), stream);
		}

		backupData.Size = (u32)stream->Size();
		backupData.Data = stream->DisownMemory();
		
	}

	return backupData;
}

void ManagedResource::Restore(const ResourceBackupData& data)
{
	ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)GetScriptObjectWrapper();
	if(!B3D_ENSURE(scriptObjectWrapper != nullptr))
		return;

	MonoObject* const scriptObject = scriptObjectWrapper->GetScriptObject();
	if(scriptObject != nullptr && data.Data != nullptr)
	{
		BinarySerializer bs;
		TShared<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
			bs.Decode(B3DMakeShared<MemoryDataStream>(data.Data, data.Size), data.Size));

		if(mObjectInformation != nullptr)
			serializableObject->Deserialize(scriptObject, mObjectInformation);
		else
			mSerializedObjectData = serializableObject;
	}

	if(mObjectInformation != nullptr)
		mSerializedObjectData = nullptr;
}

HManagedResource ManagedResource::CreateUninitialized()
{
	TShared<ManagedResource> managedResourceShared = CreateUninitializedAsShared();
	HManagedResource managedResource = B3DStaticResourceCast<ManagedResource>(GetResources().CreateResourceHandle(managedResourceShared));

	return managedResource;
}

TShared<ManagedResource> ManagedResource::CreateUninitializedAsShared()
{
	TShared<ManagedResource> managedResource = B3DMakeSharedFromExisting<ManagedResource>(new(B3DAllocate<ManagedResource>()) ManagedResource());
	managedResource->SetShared(managedResource);

	return managedResource;
}

void ManagedResource::Initialize()
{
	ScriptObjectWrapper* const scriptObjectWrapper = (ScriptObjectWrapper*)GetScriptObjectWrapper();

	// Script object could already be assigned, if this was a new resource created from script code.
	if(scriptObjectWrapper != nullptr)
	{
		MonoObject* const scriptObject = scriptObjectWrapper->GetScriptObject();
		if(!B3D_ENSURE(scriptObject != nullptr))
			return;

		TShared<ManagedResourceMetaData> metaData = B3DMakeShared<ManagedResourceMetaData>();
		mMetaData = metaData;

		MonoUtil::GetClassName(scriptObject, metaData->TypeNamespace, metaData->TypeName);
	}
	// If not, we need to create the script object (e.g. if the resource is being deserialized)
	else
	{
		TShared<ManagedObjectInfo> objectInformation;
		MonoObject* const scriptObject = CreateScriptObject(objectInformation);

		ScriptObjectWrapper::Create<ScriptManagedResource>(B3DStaticResourceCast<ManagedResource>(GetHandle()), scriptObject);
		SetupScriptBindings(objectInformation);

		if(mSerializedObjectData != nullptr && mObjectInformation != nullptr)
		{
			mSerializedObjectData->Deserialize(scriptObject, mObjectInformation);
			mSerializedObjectData = nullptr;
		}
	}
}

MonoObject* ManagedResource::CreateScriptObject(TShared<ManagedObjectInfo>& outObjectInformation) const
{
	auto metaData = B3DRTTICast<ManagedResourceMetaData>(mMetaData);
	if(B3D_ENSURE(metaData != nullptr))
	{
		if(ScriptAssemblyManager::Instance().GetSerializableObjectInfo(metaData->TypeNamespace, metaData->TypeName, outObjectInformation))
			return outObjectInformation->ScriptClass->CreateInstance(true);
	}

	return ScriptAssemblyManager::Instance().GetBuiltinClasses().MissingResourceClass->CreateInstance(true);
}

void ManagedResource::SetupScriptBindings(const TShared<ManagedObjectInfo>& objectInformation)
{
	mObjectInformation = objectInformation;
}

RTTIType* ManagedResource::GetRttiStatic()
{
	return ManagedResourceRTTI::Instance();
}

RTTIType* ManagedResource::GetRtti() const
{
	return ManagedResource::GetRttiStatic();
}
