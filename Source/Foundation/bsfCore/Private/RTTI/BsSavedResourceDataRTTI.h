//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Reflection/BsRTTIPlain.h"
#include "Resources/BsSavedResourceData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT SavedResourceDataRTTI : public RTTIType<SavedResourceData, IReflectable, SavedResourceDataRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_ARRAY(mDependencies, 0)
			BS_RTTI_MEMBER_PLAIN(mAllowAsync, 1)
			BS_RTTI_MEMBER_PLAIN(mCompressionMethod, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ResourceDependencies";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ResourceDependencies;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SavedResourceData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
