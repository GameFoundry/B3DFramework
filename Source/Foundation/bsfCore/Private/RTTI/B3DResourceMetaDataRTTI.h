//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Resources/BsResourceMetaData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ResourceMetaDataRTTI : public TRTTIType<ResourceMetaData, IReflectable, ResourceMetaDataRTTI>
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

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ResourceMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
