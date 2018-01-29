//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		IReflectableRTTI()
		{ }

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