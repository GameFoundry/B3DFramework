//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Resources/BsScriptCodeImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ScriptCodeImportOptionsRTTI : public RTTIType<ScriptCodeImportOptions, ImportOptions, ScriptCodeImportOptionsRTTI>
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

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ScriptCodeImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
