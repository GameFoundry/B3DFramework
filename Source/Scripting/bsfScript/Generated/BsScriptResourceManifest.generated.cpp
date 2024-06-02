//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceManifest.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManifest.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptResourceManifest::ScriptResourceManifest(MonoObject* managedInstance, const SPtr<ResourceManifest>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptResourceManifest::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptResourceManifest::InternalGetName);
		metaData.ScriptClass->AddInternalCall("Internal_RegisterResource", (void*)&ScriptResourceManifest::InternalRegisterResource);
		metaData.ScriptClass->AddInternalCall("Internal_UnregisterResource", (void*)&ScriptResourceManifest::InternalUnregisterResource);
		metaData.ScriptClass->AddInternalCall("Internal_UUIDToPhysicalFilePath", (void*)&ScriptResourceManifest::InternalUUIDToPhysicalFilePath);
		metaData.ScriptClass->AddInternalCall("Internal_PhysicalFilePathToUUID", (void*)&ScriptResourceManifest::InternalPhysicalFilePathToUUID);
		metaData.ScriptClass->AddInternalCall("Internal_VirtualFilePathToUUID", (void*)&ScriptResourceManifest::InternalVirtualFilePathToUUID);
		metaData.ScriptClass->AddInternalCall("Internal_UuidExists", (void*)&ScriptResourceManifest::InternalUuidExists);
		metaData.ScriptClass->AddInternalCall("Internal_FilePathExists", (void*)&ScriptResourceManifest::InternalFilePathExists);
		metaData.ScriptClass->AddInternalCall("Internal_VirtualToPhysicalPath", (void*)&ScriptResourceManifest::InternalVirtualToPhysicalPath);
		metaData.ScriptClass->AddInternalCall("Internal_Save", (void*)&ScriptResourceManifest::InternalSave);
		metaData.ScriptClass->AddInternalCall("Internal_Load", (void*)&ScriptResourceManifest::InternalLoad);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptResourceManifest::InternalCreate);

	}

	MonoObject* ScriptResourceManifest::Create(const SPtr<ResourceManifest>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptResourceManifest>()) ScriptResourceManifest(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptResourceManifest::InternalGetName(ScriptResourceManifest* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetInternal()->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptResourceManifest::InternalRegisterResource(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString* filePath)
	{
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		thisPtr->GetInternal()->RegisterResource(*uuid, tmpfilePath);
	}

	void ScriptResourceManifest::InternalUnregisterResource(ScriptResourceManifest* thisPtr, UUID* uuid)
	{
		thisPtr->GetInternal()->UnregisterResource(*uuid);
	}

	bool ScriptResourceManifest::InternalUUIDToPhysicalFilePath(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString** filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmp__output = thisPtr->GetInternal()->UUIDToPhysicalFilePath(*uuid, tmpfilePath);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(filePath,  (MonoObject*)MonoUtil::StringToMono(tmpfilePath.ToString()));

		return __output;
	}

	bool ScriptResourceManifest::InternalPhysicalFilePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		tmp__output = thisPtr->GetInternal()->PhysicalFilePathToUUID(tmpfilePath, *outUUID);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::InternalVirtualFilePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		tmp__output = thisPtr->GetInternal()->VirtualFilePathToUUID(tmpfilePath, *outUUID);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::InternalUuidExists(ScriptResourceManifest* thisPtr, UUID* uuid)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->UuidExists(*uuid);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::InternalFilePathExists(ScriptResourceManifest* thisPtr, MonoString* filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::MonoToString(filePath);
		tmp__output = thisPtr->GetInternal()->FilePathExists(tmpfilePath);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::InternalVirtualToPhysicalPath(ScriptResourceManifest* thisPtr, MonoString* virtualPath, MonoString** outPhysicalPath)
	{
		bool tmp__output;
		Path tmpvirtualPath;
		tmpvirtualPath = MonoUtil::MonoToString(virtualPath);
		Path tmpoutPhysicalPath;
		tmp__output = thisPtr->GetInternal()->VirtualToPhysicalPath(tmpvirtualPath, tmpoutPhysicalPath);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(outPhysicalPath,  (MonoObject*)MonoUtil::StringToMono(tmpoutPhysicalPath.ToString()));

		return __output;
	}

	void ScriptResourceManifest::InternalSave(MonoObject* manifest, MonoString* path, MonoString* physicalPathPrefix)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::ToNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->GetInternal();
		Path tmppath;
		tmppath = MonoUtil::MonoToString(path);
		Path tmpphysicalPathPrefix;
		tmpphysicalPathPrefix = MonoUtil::MonoToString(physicalPathPrefix);
		ResourceManifest::Save(tmpmanifest, tmppath, tmpphysicalPathPrefix);
	}

	MonoObject* ScriptResourceManifest::InternalLoad(MonoString* path, MonoString* physicalPathPrefix, MonoString* virtualPathPrefix)
	{
		SPtr<ResourceManifest> tmp__output;
		Path tmppath;
		tmppath = MonoUtil::MonoToString(path);
		Path tmpphysicalPathPrefix;
		tmpphysicalPathPrefix = MonoUtil::MonoToString(physicalPathPrefix);
		Path tmpvirtualPathPrefix;
		tmpvirtualPathPrefix = MonoUtil::MonoToString(virtualPathPrefix);
		tmp__output = ResourceManifest::Load(tmppath, tmpphysicalPathPrefix, tmpvirtualPathPrefix);

		MonoObject* __output;
		__output = ScriptResourceManifest::Create(tmp__output);

		return __output;
	}

	void ScriptResourceManifest::InternalCreate(MonoObject* managedInstance, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<ResourceManifest> instance = ResourceManifest::Create(tmpname);
		new (B3DAllocate<ScriptResourceManifest>())ScriptResourceManifest(managedInstance, instance);
	}
#endif
}
