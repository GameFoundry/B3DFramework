//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsGpuResourceData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT GpuResourceDataRTTI : public RTTIType<GpuResourceData, IReflectable, GpuResourceDataRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GpuResourceData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GpuResourceData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
