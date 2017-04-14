//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptResourceManager.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsResources.h"
#include "BsRTTIType.h"
#include "BsResource.h"
#include "BsScriptManagedResource.h"
#include "BsScriptAssemblyManager.h"

using namespace std::placeholders;

namespace bs
{
	ScriptResourceManager::ScriptResourceManager()
	{
		mResourceDestroyedConn = gResources().onResourceDestroyed.connect(std::bind(&ScriptResourceManager::onResourceDestroyed, this, _1));
	}

	ScriptResourceManager::~ScriptResourceManager()
	{
		mResourceDestroyedConn.disconnect();
	}

	ScriptManagedResource* ScriptResourceManager::createManagedScriptResource(const HManagedResource& resource, MonoObject* instance)
	{
		const String& uuid = resource.getUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		ScriptManagedResource* scriptResource = new (bs_alloc<ScriptManagedResource>()) ScriptManagedResource(instance, resource);
		mScriptResources[uuid] = scriptResource;

		return scriptResource;
	}

	ScriptResourceBase* ScriptResourceManager::createBuiltinScriptResource(const HResource& resource, MonoObject* instance)
	{
		const String& uuid = resource.getUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		UINT32 rttiId = resource->getRTTI()->getRTTIId();
		BuiltinResourceInfo* info = ScriptAssemblyManager::instance().getBuiltinResourceInfo(rttiId);

		if (info == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource = info->createCallback(resource, instance);
		mScriptResources[uuid] = scriptResource;

		return scriptResource;
	}

	ScriptResourceBase* ScriptResourceManager::getScriptResource(const HResource& resource, bool create)
	{
		String uuid = resource.getUUID();

		if (uuid.empty())
			return nullptr;

		ScriptResourceBase* output = getScriptResource(uuid);

		if (output == nullptr && create)
			return createBuiltinScriptResource(resource);

		return output;
	}

	ScriptResourceBase* ScriptResourceManager::getScriptResource(const String& uuid)
	{
		if (uuid == "")
			return nullptr;

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
			return findIter->second;

		return nullptr;
	}

	void ScriptResourceManager::destroyScriptResource(ScriptResourceBase* resource)
	{
		HResource resourceHandle = resource->getGenericHandle();
		const String& uuid = resourceHandle.getUUID();

		if(uuid == "")
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

		(resource)->~ScriptResourceBase();
		MemoryAllocator<GenAlloc>::free(resource);

		mScriptResources.erase(uuid);
	}

	void ScriptResourceManager::onResourceDestroyed(const String& UUID)
	{
		auto findIter = mScriptResources.find(UUID);
		if (findIter != mScriptResources.end())
		{
			findIter->second->notifyResourceDestroyed();
			mScriptResources.erase(findIter);
		}
	}

	void ScriptResourceManager::_throwExceptionIfInvalidOrDuplicate(const String& uuid) const
	{
		if(uuid == "")
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
		{
			BS_EXCEPT(InvalidStateException, "Provided resource handle already has a script resource. \
											 Retrieve the existing instance instead of creating a new one.");
		}
	}
}