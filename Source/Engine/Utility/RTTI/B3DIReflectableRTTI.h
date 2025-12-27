//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class B3D_EXPORT IReflectableRTTI : public TRTTIType<IReflectable, IReflectable, IReflectableRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "IReflectable";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_IReflectable;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
