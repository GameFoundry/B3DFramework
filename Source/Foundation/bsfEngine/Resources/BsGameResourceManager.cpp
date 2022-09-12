//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGameResourceManager.h"
#include "Private/RTTI/BsResourceMappingRTTI.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	void ResourceMapping::Add(const Path& from, const Path& to)
	{
		mMapping[from] = to;
	}

	SPtr<ResourceMapping> ResourceMapping::Create()
	{
		return bs_shared_ptr_new<ResourceMapping>();
	}

	RTTITypeBase* ResourceMapping::getRTTIStatic()
	{
		return ResourceMappingRTTI::Instance();
	}

	RTTITypeBase* ResourceMapping::getRTTI() const
	{
		return GetRTTIStatic();
	}

	HResource StandaloneResourceLoader::Load(const Path& path, ResourceLoadFlags flags, bool async) const
	{
		auto iterFind = mMapping.Find(path);
		if(iterFind != mMapping.End())
		{
			if(!async)
				return GResources().Load(iterFind->second, flags);
			else
				return GResources().LoadAsync(iterFind->second, flags);
		}
		
		if (!async)
			return GResources().Load(path, flags);
		else
			return GResources().LoadAsync(path, flags);
	}

	void StandaloneResourceLoader::SetMapping(const SPtr<ResourceMapping>& mapping)
	{
		mMapping = mapping->GetMap();
	}

	GameResourceManager::GameResourceManager()
		:mLoader(bs_shared_ptr_new<StandaloneResourceLoader>())
	{
		
	}

	HResource GameResourceManager::Load(const Path& path, ResourceLoadFlags flags, bool async) const
	{
		return mLoader->Load(path, flags, async);
	}

	void GameResourceManager::SetMapping(const SPtr<ResourceMapping>& mapping)
	{
		mLoader->SetMapping(mapping);
	}

	void GameResourceManager::SetLoader(const SPtr<IGameResourceLoader>& loader)
	{
		mLoader = loader;

		if (mLoader == nullptr)
			mLoader = bs_shared_ptr_new<StandaloneResourceLoader>();
	}
}
