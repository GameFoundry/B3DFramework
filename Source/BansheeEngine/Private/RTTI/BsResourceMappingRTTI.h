//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsGameResourceManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT ResourceMappingRTTI : public RTTIType<ResourceMapping, IReflectable, ResourceMappingRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mMapping, 0)
		BS_END_RTTI_MEMBERS

	public:
		ResourceMappingRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ResourceMapping";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceMapping;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ResourceMapping::create();
		}
	};

	/** @} */
	/** @endcond */
}