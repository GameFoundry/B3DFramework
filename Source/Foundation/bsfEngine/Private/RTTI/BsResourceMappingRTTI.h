//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsPathRTTI.h"
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
		const String& GetRTTIName() override
		{
			static String name = "ResourceMapping";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ResourceMapping;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ResourceMapping::Create();
		}
	};

	/** @} */
	/** @endcond */
}
