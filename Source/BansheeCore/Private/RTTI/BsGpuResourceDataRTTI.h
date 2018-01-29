//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	class BS_CORE_EXPORT GpuResourceDataRTTI : public RTTIType<GpuResourceData, IReflectable, GpuResourceDataRTTI>
	{
	public:
		GpuResourceDataRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "GpuResourceData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GpuResourceData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}