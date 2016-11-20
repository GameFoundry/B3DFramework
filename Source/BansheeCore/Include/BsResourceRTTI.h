//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsResource.h"
#include "BsResourceMetaData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ResourceRTTI : public RTTIType<Resource, IReflectable, ResourceRTTI>
	{
	private:
		UINT32& getSize(Resource* obj) { return obj->mSize; }
		void setSize(Resource* obj, UINT32& size) { obj->mSize = size; } 

		SPtr<ResourceMetaData> getMetaData(Resource* obj) { return obj->mMetaData; }
		void setMetaData(Resource* obj, SPtr<ResourceMetaData> value) { obj->mMetaData = value; }

	public:
		ResourceRTTI()
		{
			addPlainField("mSize", 0, &ResourceRTTI::getSize, &ResourceRTTI::setSize);
			addReflectablePtrField("mMetaData", 1, &ResourceRTTI::getMetaData, &ResourceRTTI::setMetaData);
		}

		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<bool, UINT64>& params)
		{
			Resource* resource = static_cast<Resource*>(obj);

			auto iterFind = params.find("keepSourceData");
			resource->mKeepSourceData = iterFind != params.end() && iterFind->second > 0;
		}

		const String& getRTTIName() override
		{
			static String name = "Resource";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return 100;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}