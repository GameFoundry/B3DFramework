//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGameResourceManager.h"
#include "Private/RTTI/BsResourceMappingRTTI.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	void ResourceMapping::add(const Path& from, const Path& to)
	{
		mMapping[from] = to;
	}

	SPtr<ResourceMapping> ResourceMapping::create()
	{
		return bs_shared_ptr_new<ResourceMapping>();
	}

	RTTITypeBase* ResourceMapping::getRTTIStatic()
	{
		return ResourceMappingRTTI::instance();
	}

	RTTITypeBase* ResourceMapping::getRTTI() const
	{
		return getRTTIStatic();
	}

	HResource StandaloneResourceLoader::load(const Path& path, ResourceLoadFlags flags, bool async) const
	{
		auto iterFind = mMapping.find(path);
		if(iterFind != mMapping.end())
		{
			if(!async)
				return gResources().load(iterFind->second, flags);
			else
				return gResources().loadAsync(iterFind->second, flags);
		}
		
		if (!async)
			return gResources().load(path, flags);
		else
			return gResources().loadAsync(path, flags);
	}

	void StandaloneResourceLoader::setMapping(const SPtr<ResourceMapping>& mapping)
	{
		mMapping = mapping->getMap();
	}

	GameResourceManager::GameResourceManager()
		:mLoader(bs_shared_ptr_new<StandaloneResourceLoader>())
	{
		
	}

	HResource GameResourceManager::load(const Path& path, ResourceLoadFlags flags, bool async) const
	{
		return mLoader->load(path, flags, async);
	}

	void GameResourceManager::setMapping(const SPtr<ResourceMapping>& mapping)
	{
		mLoader->setMapping(mapping);
	}

	void GameResourceManager::setLoader(const SPtr<IGameResourceLoader>& loader)
	{
		mLoader = loader;

		if (mLoader == nullptr)
			mLoader = bs_shared_ptr_new<StandaloneResourceLoader>();
	}
}