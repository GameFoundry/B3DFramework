//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "Resources/B3DScriptCode.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ScriptCodeRTTI : public TRTTIType<ScriptCode, Resource, ScriptCodeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mString, 0)
			B3D_RTTI_MEMBER(mEditorScript, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ScriptCode";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptCode;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return ScriptCode::CreatePtrInternal(L""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
