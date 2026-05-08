//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Resources/B3DScriptCodeImportOptions.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ScriptCodeImportOptionsRTTI : public TRTTIType<ScriptCodeImportOptions, ImportOptions, ScriptCodeImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(EditorScript, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ScriptCodeImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptCodeImportOptions;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ScriptCodeImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
