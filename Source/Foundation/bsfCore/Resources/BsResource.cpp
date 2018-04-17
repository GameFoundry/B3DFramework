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

	const String& Resource::getName() const 
	{ 
		return mMetaData->displayName; 
	}

	void Resource::setName(const String& name) 
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