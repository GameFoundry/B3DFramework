//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsResourceMetaData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ResourceMetaDataRTTI : public RTTIType<ResourceMetaData, IReflectable, ResourceMetaDataRTTI>
	{
	private:
		WString& getDisplayName(ResourceMetaData* obj) { return obj->displayName; }
		void setDisplayName(ResourceMetaData* obj, WString& name) { obj->displayName = name; }

	public:
		ResourceMetaDataRTTI()
		{
			addPlainField("displayName", 0, &ResourceMetaDataRTTI::getDisplayName, &ResourceMetaDataRTTI::setDisplayName);
		}

		const String& getRTTIName() override
		{
			static String name = "ResourceMetaData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceMetaData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
}