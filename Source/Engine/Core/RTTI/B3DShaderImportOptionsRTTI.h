//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Importer/B3DShaderImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ShaderImportOptionsRTTI : public TRTTIType<ShaderImportOptions, ImportOptions, ShaderImportOptionsRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mDefines, 0)
			B3D_RTTI_MEMBER(Languages, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderImportOptions;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
