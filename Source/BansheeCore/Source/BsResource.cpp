//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsResource.h"
#include "BsResourceRTTI.h"
#include "BsResourceMetaData.h"

namespace bs
{
	Resource::Resource(bool initializeOnRenderThread)
		:CoreObject(initializeOnRenderThread), mSize(0), mKeepSourceData(true)
	{ 
		mMetaData = bs_shared_ptr_new<ResourceMetaData>();
	}

	const WString& Resource::getName() const 
	{ 
		return mMetaData->displayName; 
	}

	void Resource::setName(const WString& name) 
	{ 
		mMetaData->displayName = name; 
	}

	bool Resource::areDependenciesLoaded() const
	{
		bs_frame_mark();

		bool areLoaded = true;
		{
			FrameVector<HResource> dependencies;
			getResourceDependencies(dependencies);

			for (auto& dependency : dependencies)
			{
				if (dependency != nullptr && !dependency.isLoaded())
				{
					areLoaded = false;
					break;
				}
			}
		}

		bs_frame_clear();
		return areLoaded;
	}

	RTTITypeBase* Resource::getRTTIStatic()
	{
		return ResourceRTTI::instance();
	}

	RTTITypeBase* Resource::getRTTI() const
	{
		return Resource::getRTTIStatic();
	}
}