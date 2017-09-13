//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptResourceRef.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Resources/BsResources.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "BsApplication.h"

#include "BsScriptTexture.generated.h"

namespace bs
{
	ScriptResourceRef::ScriptResourceRef(MonoObject* instance, const WeakResourceHandle<Resource>& resource)
		:ScriptObject(instance), mResource(resource)
	{ }

	void ScriptResourceRef::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_IsLoaded", (void*)&ScriptResourceRef::internal_IsLoaded);
		metaData.scriptClass->addInternalCall("Internal_GetResource", (void*)&ScriptResourceRef::internal_GetResource);
		metaData.scriptClass->addInternalCall("Internal_GetUUID", (void*)&ScriptResourceRef::internal_GetUUID);
	}

	MonoObject* ScriptResourceRef::createInternal(const WeakResourceHandle<Resource>& handle)
	{
		MonoObject* obj = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptResourceRef>()) ScriptResourceRef(obj, handle);

		return obj;
	}

	bool ScriptResourceRef::internal_IsLoaded(ScriptResourceRef* nativeInstance)
	{
		return nativeInstance->mResource.isLoaded(false);
	}

	MonoObject* ScriptResourceRef::internal_GetResource(ScriptResourceRef* nativeInstance)
	{
		ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies | ResourceLoadFlag::KeepInternalRef;

		if (gApplication().isEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().load(nativeInstance->mResource, loadFlags);
		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(resource, true);

		return scriptResource->getManagedInstance();
	}

	void ScriptResourceRef::internal_GetUUID(ScriptResourceRef* thisPtr, UUID* uuid)
	{
		*uuid = thisPtr->getHandle().getUUID();
	}
}

