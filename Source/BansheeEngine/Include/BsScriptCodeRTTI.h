//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsScriptCode.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT ScriptCodeRTTI : public RTTIType <ScriptCode, Resource, ScriptCodeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mString, 0)
			BS_RTTI_MEMBER_PLAIN(mEditorScript, 1)
		BS_END_RTTI_MEMBERS

	public:
		ScriptCodeRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptCode";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptCode;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ScriptCode::_createPtr(L""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
}