//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResources.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Resources/BsResources.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManifest.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptResources::onResourceLoadedThunkDef ScriptResources::onResourceLoadedThunk; 
	ScriptResources::onResourceDestroyedThunkDef ScriptResources::onResourceDestroyedThunk; 
	ScriptResources::onResourceModifiedThunkDef ScriptResources::onResourceModifiedThunk; 

	HEvent ScriptResources::onResourceLoadedConn;
	HEvent ScriptResources::onResourceDestroyedConn;
	HEvent ScriptResources::onResourceModifiedConn;

	ScriptResources::ScriptResources(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
		mGCHandle = MonoUtil::newWeakGCHandle(managedInstance);
	}

	void ScriptResources::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_load", (void*)&ScriptResources::Internal_load);
		metaData.scriptClass->addInternalCall("Internal_loadAsync", (void*)&ScriptResources::Internal_loadAsync);
		metaData.scriptClass->addInternalCall("Internal_loadFromUUID", (void*)&ScriptResources::Internal_loadFromUUID);
		metaData.scriptClass->addInternalCall("Internal_release", (void*)&ScriptResources::Internal_release);
		metaData.scriptClass->addInternalCall("Internal_unloadAllUnused", (void*)&ScriptResources::Internal_unloadAllUnused);
		metaData.scriptClass->addInternalCall("Internal_unloadAll", (void*)&ScriptResources::Internal_unloadAll);
		metaData.scriptClass->addInternalCall("Internal_save", (void*)&ScriptResources::Internal_save);
		metaData.scriptClass->addInternalCall("Internal_save0", (void*)&ScriptResources::Internal_save0);
		metaData.scriptClass->addInternalCall("Internal_getDependencies", (void*)&ScriptResources::Internal_getDependencies);
		metaData.scriptClass->addInternalCall("Internal_isLoaded", (void*)&ScriptResources::Internal_isLoaded);
		metaData.scriptClass->addInternalCall("Internal_getLoadProgress", (void*)&ScriptResources::Internal_getLoadProgress);
		metaData.scriptClass->addInternalCall("Internal_registerResourceManifest", (void*)&ScriptResources::Internal_registerResourceManifest);
		metaData.scriptClass->addInternalCall("Internal_unregisterResourceManifest", (void*)&ScriptResources::Internal_unregisterResourceManifest);
		metaData.scriptClass->addInternalCall("Internal_getResourceManifest", (void*)&ScriptResources::Internal_getResourceManifest);
		metaData.scriptClass->addInternalCall("Internal_getFilePathFromUUID", (void*)&ScriptResources::Internal_getFilePathFromUUID);
		metaData.scriptClass->addInternalCall("Internal_getUUIDFromFilePath", (void*)&ScriptResources::Internal_getUUIDFromFilePath);

		onResourceLoadedThunk = (onResourceLoadedThunkDef)metaData.scriptClass->getMethodExact("Internal_onResourceLoaded", "RRefBase")->getThunk();
		onResourceDestroyedThunk = (onResourceDestroyedThunkDef)metaData.scriptClass->getMethodExact("Internal_onResourceDestroyed", "UUID&")->getThunk();
		onResourceModifiedThunk = (onResourceModifiedThunkDef)metaData.scriptClass->getMethodExact("Internal_onResourceModified", "RRefBase")->getThunk();
	}

	void ScriptResources::startUp()
	{
		onResourceLoadedConn = Resources::instance().onResourceLoaded.connect(&ScriptResources::onResourceLoaded);
		onResourceDestroyedConn = Resources::instance().onResourceDestroyed.connect(&ScriptResources::onResourceDestroyed);
		onResourceModifiedConn = Resources::instance().onResourceModified.connect(&ScriptResources::onResourceModified);
	}
	void ScriptResources::shutDown()
	{
		onResourceLoadedConn.disconnect();
		onResourceDestroyedConn.disconnect();
		onResourceModifiedConn.disconnect();
	}

	void ScriptResources::onResourceLoaded(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().getScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->getManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::invokeThunk(onResourceLoadedThunk, tmpp0);
	}

	void ScriptResources::onResourceDestroyed(const UUID& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptUUID::box(p0);
		MonoUtil::invokeThunk(onResourceDestroyedThunk, tmpp0);
	}

	void ScriptResources::onResourceModified(const ResourceHandle<Resource>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().getScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->getManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::invokeThunk(onResourceModifiedThunk, tmpp0);
	}
	MonoObject* ScriptResources::Internal_load(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = Resources::instance().load(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptResourceBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptResource(tmp__output, true);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::Internal_loadAsync(MonoString* filePath, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		tmp__output = Resources::instance().loadAsync(tmpfilePath, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	MonoObject* ScriptResources::Internal_loadFromUUID(UUID* uuid, bool async, ResourceLoadFlag loadFlags)
	{
		ResourceHandle<Resource> tmp__output;
		tmp__output = Resources::instance().loadFromUUID(*uuid, async, loadFlags);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptResources::Internal_release(MonoObject* resource)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->getHandle());
		Resources::instance().release(tmpresource);
	}

	void ScriptResources::Internal_unloadAllUnused()
	{
		Resources::instance().unloadAllUnused();
	}

	void ScriptResources::Internal_unloadAll()
	{
		Resources::instance().unloadAll();
	}

	void ScriptResources::Internal_save(MonoObject* resource, MonoString* filePath, bool overwrite, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->getGenericHandle());
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		Resources::instance().save(tmpresource, tmpfilePath, overwrite, compress);
	}

	void ScriptResources::Internal_save0(MonoObject* resource, bool compress)
	{
		ResourceHandle<Resource> tmpresource;
		ScriptResource* scriptresource;
		scriptresource = ScriptResource::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->getGenericHandle());
		Resources::instance().save(tmpresource, compress);
	}

	MonoArray* ScriptResources::Internal_getDependencies(MonoString* filePath)
	{
		Vector<UUID> vec__output;
		Path tmpfilePath;
		tmpfilePath = MonoUtil::monoToString(filePath);
		vec__output = Resources::instance().getDependencies(tmpfilePath);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptUUID>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, vec__output[i]);
		}
		__output = array__output.getInternal();

		return __output;
	}

	bool ScriptResources::Internal_isLoaded(UUID* uuid, bool checkInProgress)
	{
		bool tmp__output;
		tmp__output = Resources::instance().isLoaded(*uuid, checkInProgress);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptResources::Internal_getLoadProgress(MonoObject* resource, bool includeDependencies)
	{
		float tmp__output;
		ResourceHandle<Resource> tmpresource;
		ScriptRRefBase* scriptresource;
		scriptresource = ScriptRRefBase::toNative(resource);
		if(scriptresource != nullptr)
			tmpresource = static_resource_cast<Resource>(scriptresource->getHandle());
		tmp__output = Resources::instance().getLoadProgress(tmpresource, includeDependencies);

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptResources::Internal_registerResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::toNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->getInternal();
		Resources::instance().registerResourceManifest(tmpmanifest);
	}

	void ScriptResources::Internal_unregisterResourceManifest(MonoObject* manifest)
	{
		SPtr<ResourceManifest> tmpmanifest;
		ScriptResourceManifest* scriptmanifest;
		scriptmanifest = ScriptResourceManifest::toNative(manifest);
		if(scriptmanifest != nullptr)
			tmpmanifest = scriptmanifest->getInternal();
		Resources::instance().unregisterResourceManifest(tmpmanifest);
	}

	MonoObject* ScriptResources::Internal_getResourceManifest(MonoString* name)
	{
		SPtr<ResourceManifest> tmp__output;
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		tmp__output = Resources::instance().getResourceManifest(tmpname);

		MonoObject* __output;
		__output = ScriptResourceManifest::create(tmp__output);

		return __output;
	}

	bool ScriptResources::Internal_getFilePathFromUUID(UUID* uuid, MonoString** filePath)
	{
		bool tmp__output;
		Path tmpfilePath;
		tmp__output = Resources::instance().getFilePathFromUUID(*uuid, tmpfilePath);

		bool __output;
		__output = tmp__output;
		MonoUtil::referenceCopy(filePath,  (MonoObject*)MonoUtil::stringToMono(tmpfilePath.toString()));

		return __output;
	}

	bool ScriptResources::Internal_getUUIDFromFilePath(MonoString* path, UUID* uuid)
	{
		bool tmp__output;
		Path tmppath;
		tmppath = MonoUtil::monoToString(path);
		tmp__output = Resources::instance().getUUIDFromFilePath(tmppath, *uuid);

		bool __output;
		__output = tmp__output;

		return __output;
	}
#endif
}
