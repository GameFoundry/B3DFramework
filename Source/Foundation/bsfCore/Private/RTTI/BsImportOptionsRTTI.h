//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ImportOptionsRTTI : public RTTIType<ImportOptions, IReflectable, ImportOptionsRTTI>
	{
	public:
		ImportOptionsRTTI()
		{}

		const String& GetRttiName()
		{
			static String name = "ImportOptions";
			return name;
		}

		u32 GetRttiId()
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
} // namespace bs
