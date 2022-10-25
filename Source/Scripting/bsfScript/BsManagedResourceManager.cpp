//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedResourceManager.h"
#include "Resources/BsResources.h"

namespace bs
{
	ManagedResourceManager::~ManagedResourceManager()
	{
		Clear();
	}

	void ManagedResourceManager::Clear()
	{
		UnorderedMap<UUID, WeakResourceHandle<ManagedResource>> resourceCopy = mResources;
		for(auto& resourcePair : resourceCopy)
		{
			WeakResourceHandle<ManagedResource> resource = resourcePair.second;
			gResources().Release((WeakResourceHandle<Resource>&)resource);
		}

		mResources.clear();
	}

	void ManagedResourceManager::RegisterManagedResource(const WeakResourceHandle<ManagedResource>& resource)
	{
		mResources.insert(std::make_pair(resource.GetUuid(), resource));
	}

	void ManagedResourceManager::UnregisterManagedResource(const WeakResourceHandle<ManagedResource>& resource)
	{
		mResources.erase(resource.GetUuid());
	}
} // namespace bs
