//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Resources/BsResource.h"
#include "Resources/BsResourceMetaData.h"

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

		void onDeserializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			Resource* resource = static_cast<Resource*>(obj);

			if(context)
				resource->mKeepSourceData = (context->flags & SF_KeepResourceSourceData) != 0;
			else
				resource->mKeepSourceData = false;
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
