//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class BS_UTILITY_EXPORT IReflectableRTTI : public RTTIType <IReflectable, IReflectable, IReflectableRTTI>
	{
	public:
		IReflectableRTTI() = default;

		const String& getRTTIName() override
		{
			static String name = "IReflectable";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_IReflectable;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
