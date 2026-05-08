//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "Resources/B3DResourceMetaData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ResourceMetaDataRTTI : public TRTTIType<ResourceMetaData, IReflectable, ResourceMetaDataRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "ResourceMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ResourceMetaData;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
