//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptResources.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsResources.h"
#include "BsGameResourceManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptResource.h"
#include "BsScriptResourceRef.h"
#include "BsApplication.h"

namespace BansheeEngine
{
	ScriptResources::ScriptResources(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptResources::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Load", &ScriptResources::internal_Load);
		metaData.scriptClass->addInternalCall("Internal_LoadRef", &ScriptResources::internal_LoadRef);
		metaData.scriptClass->addInternalCall("Internal_UnloadUnused", &ScriptResources::internal_UnloadUnused);
		metaData.scriptClass->addInternalCall("Internal_Release", &ScriptResources::internal_Release);
		metaData.scriptClass->addInternalCall("Internal_ReleaseRef", &ScriptResources::internal_ReleaseRef);
	}

	MonoObject* ScriptResources::internal_Load(MonoString* path, bool keepLoaded)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		HResource resource = GameResourceManager::instance().load(nativePath, keepLoaded);
		if (resource == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource;
		ScriptResourceManager::instance().getScriptResource(resource, &scriptResource, true);

		return scriptResource->getManagedInstance();
	}

	MonoObject* ScriptResources::internal_LoadRef(MonoObject* reference, bool keepLoaded)
	{
		ScriptResourceRef* scriptRef = ScriptResourceRef::toNative(reference);
		if (scriptRef == nullptr)
			return nullptr;

		ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies;
		if (keepLoaded)
			loadFlags |= ResourceLoadFlag::KeepInternalRef;

		if (gApplication().isEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().load(scriptRef->getHandle(), loadFlags);
		if (resource == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource;
		ScriptResourceManager::instance().getScriptResource(resource, &scriptResource, true);

		return scriptResource->getManagedInstance();
	}

	void ScriptResources::internal_Release(ScriptResourceBase* resource)
	{
		resource->getGenericHandle().release();
	}

	void ScriptResources::internal_ReleaseRef(ScriptResourceRef* resourceRef)
	{
		resourceRef->getHandle().release();
	}

	void ScriptResources::internal_UnloadUnused()
	{
		gResources().unloadAllUnused();
	}
}