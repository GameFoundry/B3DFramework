//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptResourceManager.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "Resources/BsResources.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsResource.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsManagedResource.h"
#include "Wrappers/BsScriptRRefBase.h"

using namespace std::placeholders;

namespace bs
{
	ScriptResourceManager::ScriptResourceManager()
	{
		mResourceDestroyedConn = gResources().onResourceDestroyed.connect(std::bind(&ScriptResourceManager::onResourceDestroyed, this, _1));
		mDomainUnloadedConn = MonoManager::instance().onDomainUnload.connect(std::bind(&ScriptResourceManager::clearRRefs, this));
	}

	ScriptResourceManager::~ScriptResourceManager()
	{
		mDomainUnloadedConn.disconnect();
		mResourceDestroyedConn.disconnect();
	}

	ScriptManagedResource* ScriptResourceManager::createManagedScriptResource(const HManagedResource& resource, MonoObject* instance)
	{
		const UUID& uuid = resource.getUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		ScriptManagedResource* scriptResource = new (bs_alloc<ScriptManagedResource>()) ScriptManagedResource(instance, resource);
		mScriptResources[uuid] = scriptResource;

		return scriptResource;
	}

	ScriptResourceBase* ScriptResourceManager::createBuiltinScriptResource(const HResource& resource, MonoObject* instance)
	{
		const UUID& uuid = resource.getUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		if (!resource.isLoaded(false))
			return nullptr;

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
		const UUID& uuid = resource.getUUID();

		if (uuid.empty())
			return nullptr;

		ScriptResourceBase* output = getScriptResource(uuid);

		if (output == nullptr && create)
			return createBuiltinScriptResource(resource);

		return output;
	}

	ScriptResourceBase* ScriptResourceManager::getScriptResource(const UUID& uuid)
	{
		if (uuid.empty())
			return nullptr;

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
			return findIter->second;

		return nullptr;
	}

	ScriptRRefBase* ScriptResourceManager::getScriptRRef(const HResource& resource, ::MonoClass* rrefClass)
	{
		UnorderedMap<UUID, ScriptRRefBase*>& rrefs = mScriptRRefsPerType[rrefClass];
		const auto iterFind = rrefs.find(resource.getUUID());
		if (iterFind != rrefs.end())
			return iterFind->second;

		ScriptRRefBase* newRRef = ScriptRRefBase::create(resource, rrefClass);
		rrefs[resource.getUUID()] = newRRef;

		return newRRef;
	}

	void ScriptResourceManager::destroyScriptResource(ScriptResourceBase* resource)
	{
		HResource resourceHandle = resource->getGenericHandle();
		const UUID& uuid = resourceHandle.getUUID();

		if(uuid.empty())
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

#if BS_DEBUG_MODE
		for(auto& kvp : mScriptRRefsPerType)
		{
			UnorderedMap<UUID, ScriptRRefBase*>& rrefs = kvp.second;

			// No handles should exist at this point because we only manually free the ScriptResourceBase object if the
			// native resource is destroyed, which we handle in onResourceDestroyed. And only other destruction should
			// happen during assembly refresh, which we handled in clearRRefs().
			const auto iterFind = rrefs.find(uuid);
			assert(iterFind == rrefs.end());
		}
#endif

		(resource)->~ScriptResourceBase();
		MemoryAllocator<GenAlloc>::free(resource);

		mScriptResources.erase(uuid);
	}

	void ScriptResourceManager::onResourceDestroyed(const UUID& uuid)
	{
		for(auto& kvp : mScriptRRefsPerType)
		{
			UnorderedMap<UUID, ScriptRRefBase*>& rrefs = kvp.second;

			const auto iterFind = rrefs.find(uuid);
			if (iterFind != rrefs.end())
				iterFind->second->clearResource();
		}

		auto findIter = mScriptResources.find(uuid);
		if (findIter != mScriptResources.end())
		{
			findIter->second->notifyResourceDestroyed();
			mScriptResources.erase(findIter);
		}
	}

	void ScriptResourceManager::clearRRefs()
	{
		mScriptRRefsPerType.clear();
	}

	void ScriptResourceManager::_throwExceptionIfInvalidOrDuplicate(const UUID& uuid) const
	{
		if(uuid.empty())
			BS_EXCEPT(InvalidParametersException, "Provided resource handle has an undefined resource UUID.");

		auto findIter = mScriptResources.find(uuid);
		if(findIter != mScriptResources.end())
		{
			BS_EXCEPT(InvalidStateException, "Provided resource handle already has a script resource. \
											 Retrieve the existing instance instead of creating a new one.");
		}
	}
}