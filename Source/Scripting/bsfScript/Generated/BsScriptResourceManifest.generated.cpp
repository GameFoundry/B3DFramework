//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceManifest.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManifest.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptResourceManifest::ScriptResourceManifest(MonoObject* managedInstance, const SPtr<ResourceManifest>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptResourceManifest::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getName", (void*)&ScriptResourceManifest::Internal_getName);
		metaData.scriptClass->addInternalCall("Internal_registerResource", (void*)&ScriptResourceManifest::Internal_registerResource);
		metaData.scriptClass->addInternalCall("Internal_unregisterResource", (void*)&ScriptResourceManifest::Internal_unregisterResource);
		metaData.scriptClass->addInternalCall("Internal_uuidToFilePath", (void*)&ScriptResourceManifest::Internal_uuidToFilePath);
		metaData.scriptClass->addInternalCall("Internal_filePathToUUID", (void*)&ScriptResourceManifest::Internal_filePathToUUID);
		metaData.scriptClass->addInternalCall("Internal_uuidExists", (void*)&ScriptResourceManifest::Internal_uuidExists);
		metaData.scriptClass->addInternalCall("Internal_filePathExists", (void*)&ScriptResourceManifest::Internal_filePathExists);
		metaData.scriptClass->addInternalCall("Internal_save", (void*)&ScriptResourceManifest::Internal_save);
		metaData.scriptClass->addInternalCall("Internal_load", (void*)&ScriptResourceManifest::Internal_load);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptResourceManifest::Internal_create);

	}

	MonoObject* ScriptResourceManifest::create(const SPtr<ResourceManifest>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptResourceManifest>()) ScriptResourceManifest(managedInstance, value);
		return managedInstance;
	}
	MonoString* ScriptResourceManifest::Internal_getName(ScriptResourceManifest* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->getName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	void ScriptResourceManifest::Internal_registerResource(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString* filePath)
	{
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		thisPtr->getInternal()->registerResource(*uuid, tmpfilePath);
	}

	void ScriptResourceManifest::Internal_unregisterResource(ScriptResourceManifest* thisPtr, UUID* uuid)
	{
		thisPtr->getInternal()->unregisterResource(*uuid);
	}

	bool ScriptResourceManifest::Internal_uuidToFilePath(ScriptResourceManifest* thisPtr, UUID* uuid, MonoString** filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmp__output = thisPtr->getInternal()->uuidToFilePath(*uuid, tmpfilePath);

		bool __output;
		__output = tmp__output;
		MonoUtil::referenceCopy(filePath,  (MonoObject*)MonoUtil::stringToMono(tmpfilePath.toString()));

		return __output;
	}

	bool ScriptResourceManifest::Internal_filePathToUUID(ScriptResourceManifest* thisPtr, MonoString* filePath, UUID* outUUID)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = thisPtr->getInternal()->filePathToUUID(tmpfilePath, *outUUID);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::Internal_uuidExists(ScriptResourceManifest* thisPtr, UUID* uuid)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->uuidExists(*uuid);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptResourceManifest::Internal_filePathExists(ScriptResourceManifest* thisPtr, MonoString* filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = thisPtr->getInternal()->filePathExists(tmpfilePath);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptResourceManifest::Internal_save(MonoObject* manifest, MonoString* path, MonoString* relativePath)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::toNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->getInternal();
		Path tmppath;
		tmppath = MonoUtil::monoToString(path);
		Path tmprelativePath;
		tmprelativePath = MonoUtil::monoToString(relativePath);
		ResourceManifest::save(tmpmanifest, tmppath, tmprelativePath);
	}

	MonoObject* ScriptResourceManifest::Internal_load(MonoString* path, MonoString* relativePath)
	{
		SPtr<ResourceManifest> tmp__output;
		Path tmppath;
		tmppath = MonoUtil::monoToString(path);
		Path tmprelativePath;
		tmprelativePath = MonoUtil::monoToString(relativePath);
		tmp__output = ResourceManifest::load(tmppath, tmprelativePath);

		MonoObject* __output;
		__output = ScriptResourceManifest::create(tmp__output);

		return __output;
	}

	void ScriptResourceManifest::Internal_create(MonoObject* managedInstance, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<ResourceManifest> instance = ResourceManifest::create(tmpname);
		new (bs_alloc<ScriptResourceManifest>())ScriptResourceManifest(managedInstance, instance);
	}
#endif
}
