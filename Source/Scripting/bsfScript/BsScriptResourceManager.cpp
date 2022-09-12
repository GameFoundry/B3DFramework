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
		mResourceDestroyedConn = gResources().onResourceDestroyed.Connect(std::bind(&ScriptResourceManager::onResourceDestroyed, this, _1));
		mDomainUnloadedConn = MonoManager::instance().onDomainUnload.Connect(std::bind(&ScriptResourceManager::clearRRefs, this));
	}

	ScriptResourceManager::~ScriptResourceManager()
	{
		mDomainUnloadedConn.Disconnect();
		mResourceDestroyedConn.Disconnect();
	}

	ScriptManagedResource* ScriptResourceManager::createManagedScriptResource(const HManagedResource& resource, MonoObject* instance)
	{
		const UUID& uuid = resource.GetUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		ScriptManagedResource* scriptResource = new (bs_alloc<ScriptManagedResource>()) ScriptManagedResource(instance, resource);
		mScriptResources[uuid] = scriptResource;

		return scriptResource;
	}

	ScriptResourceBase* ScriptResourceManager::createBuiltinScriptResource(const HResource& resource, MonoObject* instance)
	{
		const UUID& uuid = resource.GetUUID();
#if BS_DEBUG_MODE
		_throwExceptionIfInvalidOrDuplicate(uuid);
#endif

		if (!resource.IsLoaded(false))
			return nullptr;

		UINT32 rttiId = resource->GetRTTI()->getRTTIId();
		BuiltinResourceInfo* info = ScriptAssemblyManager::instance().GetBuiltinResourceInfo(rttiId);

		if (info == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource = info->CreateCallback(resource, instance);
		mScriptResources[uuid] = scriptResource;

		return scriptResource;
	}

	ScriptResourceBase* ScriptResourceManager::getScriptResource(const HResource& resource, bool create)
	{
		const UUID& uuid = resource.GetUUID();

		if (uuid.empty())
			return nullptr;

		ScriptResourceBase* output = getScriptResource(uuid);

		if (output == nullptr && create)
			return CreateBuiltinScriptResource(resource);

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
		rrefs[resource.GetUUID()] = newRRef;

		return newRRef;
	}

	void ScriptResourceManager::DestroyScriptResource(ScriptResourceBase* resource)
	{
		HResource resourceHandle = resource->GetGenericHandle();
		const UUID& uuid = resourceHandle.GetUUID();

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

	void ScriptResourceManager::OnResourceDestroyed(const UUID& uuid)
	{
		for(auto& kvp : mScriptRRefsPerType)
		{
			UnorderedMap<UUID, ScriptRRefBase*>& rrefs = kvp.second;

			const auto iterFind = rrefs.find(uuid);
			if (iterFind != rrefs.end())
				iterFind->second->ClearResource();
		}

		auto findIter = mScriptResources.find(uuid);
		if (findIter != mScriptResources.end())
		{
			findIter->second->NotifyResourceDestroyed();
			mScriptResources.erase(findIter);
		}
	}

	void ScriptResourceManager::ClearRRefs()
	{
		mScriptRRefsPerType.Clear();
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
