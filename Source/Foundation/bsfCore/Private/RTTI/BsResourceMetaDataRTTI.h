//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
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
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(DisplayName, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ResourceMetaData";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ResourceMetaData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
