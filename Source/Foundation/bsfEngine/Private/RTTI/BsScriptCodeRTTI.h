//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Resources/BsScriptCode.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ScriptCodeRTTI : public RTTIType<ScriptCode, Resource, ScriptCodeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mString, 0)
			B3D_RTTI_MEMBER_PLAIN(mEditorScript, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ScriptCode";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ScriptCode;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ScriptCode::CreatePtrInternal(L""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
