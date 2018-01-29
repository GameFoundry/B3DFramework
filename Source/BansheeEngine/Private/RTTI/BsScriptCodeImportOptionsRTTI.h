//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsScriptCodeImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT ScriptCodeImportOptionsRTTI : public RTTIType <ScriptCodeImportOptions, ImportOptions, ScriptCodeImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mEditorScript, 0)
		BS_END_RTTI_MEMBERS

	public:
		ScriptCodeImportOptionsRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "ScriptCodeImportOptions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptCodeImportOptions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ScriptCodeImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}