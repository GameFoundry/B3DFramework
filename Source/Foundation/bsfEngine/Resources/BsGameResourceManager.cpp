//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsGameResourceManager.h"
#include "Private/RTTI/BsResourceMappingRTTI.h"
#include "FileSystem/BsFileSystem.h"

using namespace bs;

void ResourceMapping::Add(const Path& from, const Path& to)
{
	mMapping[from] = to;
}

SPtr<ResourceMapping> ResourceMapping::Create()
{
	return bs_shared_ptr_new<ResourceMapping>();
}

RTTITypeBase* ResourceMapping::GetRttiStatic()
{
	return ResourceMappingRTTI::Instance();
}

RTTITypeBase* ResourceMapping::GetRtti() const
{
	return GetRttiStatic();
}

HResource StandaloneResourceLoader::Load(const Path& path, ResourceLoadFlags flags, bool async) const
{
	auto iterFind = mMapping.find(path);
	if(iterFind != mMapping.end())
	{
		if(!async)
			return GetResources().Load(iterFind->second, flags);
		else
			return GetResources().LoadAsync(iterFind->second, flags);
	}

	if(!async)
		return GetResources().Load(path, flags);
	else
		return GetResources().LoadAsync(path, flags);
}

void StandaloneResourceLoader::SetMapping(const SPtr<ResourceMapping>& mapping)
{
	mMapping = mapping->GetMap();
}

GameResourceManager::GameResourceManager()
	: mLoader(bs_shared_ptr_new<StandaloneResourceLoader>())
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

	if(mLoader == nullptr)
		mLoader = bs_shared_ptr_new<StandaloneResourceLoader>();
}
