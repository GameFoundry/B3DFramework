//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsSavedResourceData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT SavedResourceDataRTTI : public RTTIType <SavedResourceData, IReflectable, SavedResourceDataRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_ARRAY(mDependencies, 0)
			BS_RTTI_MEMBER_PLAIN(mAllowAsync, 1)
			BS_RTTI_MEMBER_PLAIN(mCompressionMethod, 2)
		BS_END_RTTI_MEMBERS

	public:
		SavedResourceDataRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ResourceDependencies";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceDependencies;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SavedResourceData>();
		}
	};

	/** @} */
	/** @endcond */
}