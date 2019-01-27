//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedResourceManager.h"
#include "Resources/BsResources.h"

namespace bs
{
	ManagedResourceManager::ManagedResourceManager()
	{

	}

	ManagedResourceManager::~ManagedResourceManager()
	{
		clear();
	}

	void ManagedResourceManager::clear()
	{
		UnorderedMap<UUID, WeakResourceHandle<ManagedResource>> resourceCopy = mResources;
		for (auto& resourcePair : resourceCopy)
		{
			WeakResourceHandle<ManagedResource> resource = resourcePair.second;
			gResources().release((WeakResourceHandle<Resource>&)resource);
		}

		mResources.clear();
	}

	void ManagedResourceManager::registerManagedResource(const WeakResourceHandle<ManagedResource>& resource)
	{
		mResources.insert(std::make_pair(resource.getUUID(), resource));
	}

	void ManagedResourceManager::unregisterManagedResource(const WeakResourceHandle<ManagedResource>& resource)
	{
		mResources.erase(resource.getUUID());
	}
}