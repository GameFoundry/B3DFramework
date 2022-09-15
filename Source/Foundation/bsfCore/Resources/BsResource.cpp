//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResource.h"
#include "Private/RTTI/BsResourceRTTI.h"
#include "Resources/BsResourceMetaData.h"

namespace bs
{
	Resource::Resource(bool initializeOnRenderThread)
		:CoreObject(initializeOnRenderThread), mSize(0), mKeepSourceData(true)
	{
		mMetaData = bs_shared_ptr_new<ResourceMetaData>();
	}

	const String& Resource::GetName() const
	{
		return mMetaData->displayName;
	}

	void Resource::SetName(const String& name)
	{
		mMetaData->displayName = name;
	}

	void Resource::GetResourceDependencies(FrameVector<HResource>& dependencies) const
	{
		Lock lock(mDependenciesMutex);

		for(auto& dependency : mDependencies)
		{
			if(dependency != nullptr)
				dependencies.push_back(static_resource_cast<Resource>(dependency));
		}
	}

	bool Resource::AreDependenciesLoaded() const
	{
		Lock lock(mDependenciesMutex);
		bs_frame_mark();

		bool areLoaded = true;
		{
			for (auto& dependency : mDependencies)
			{
				if (dependency != nullptr && !dependency.IsLoaded())
				{
					areLoaded = false;
					break;
				}
			}
		}

		bs_frame_clear();
		return areLoaded;
	}

	void Resource::AddResourceDependency(const HResource& resource)
	{
		if(resource == nullptr)
			return;

		Lock lock(mDependenciesMutex);
		mDependencies.push_back(resource.GetWeak());
	}

	void Resource::RemoveResourceDependency(const HResource& resource)
	{
		Lock lock(mDependenciesMutex);
		mDependencies.erase(std::remove(mDependencies.begin(), mDependencies.end(), resource.GetWeak()),
			mDependencies.end());
	}

	RTTITypeBase* Resource::GetRttiStatic()
	{
		return ResourceRTTI::Instance();
	}

	RTTITypeBase* Resource::GetRtti() const
	{
		return Resource::GetRttiStatic();
	}
}
