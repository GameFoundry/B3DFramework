//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGameResourceManager.h"
#include "BsResourceMappingRTTI.h"
#include "BsResources.h"
#include "BsFileSystem.h"

namespace BansheeEngine
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

	HResource StandaloneResourceLoader::load(const Path& path, bool keepLoaded) const
	{
		ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies;
		if (keepLoaded)
			loadFlags |= ResourceLoadFlag::KeepInternalRef;

		auto iterFind = mMapping.find(path);
		if(iterFind != mMapping.end())
			return gResources().load(iterFind->second, loadFlags);
		
		return gResources().load(path, loadFlags);
	}

	void StandaloneResourceLoader::setMapping(const SPtr<ResourceMapping>& mapping)
	{
		mMapping = mapping->getMap();
	}

	GameResourceManager::GameResourceManager()
		:mLoader(bs_shared_ptr_new<StandaloneResourceLoader>())
	{
		
	}

	HResource GameResourceManager::load(const Path& path, bool keepLoaded) const
	{
		return mLoader->load(path, keepLoaded);
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