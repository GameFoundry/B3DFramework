//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Resources/B3DGpuResourceData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT GpuResourceDataRTTI : public TRTTIType<GpuResourceData, IReflectable, GpuResourceDataRTTI>
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

		TShared<IReflectable> NewRttiObject()
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
