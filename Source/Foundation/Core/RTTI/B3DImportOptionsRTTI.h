//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Reflection/B3DRTTIType.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ImportOptionsRTTI : public TRTTIType<ImportOptions, IReflectable, ImportOptionsRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "ImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ImportOptions;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
