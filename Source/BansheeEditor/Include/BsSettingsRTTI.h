//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsSettings.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class SettingsRTTI : public RTTIType <Settings, IReflectable, SettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mFloatProperties, 13)
			BS_RTTI_MEMBER_PLAIN(mIntProperties, 14)
			BS_RTTI_MEMBER_PLAIN(mBoolProperties, 15)
			BS_RTTI_MEMBER_PLAIN(mStringProperties, 16)
		BS_END_RTTI_MEMBERS
	public:
		SettingsRTTI()
			:mInitMembers(this)
		{ }

		virtual const String& getRTTIName() override
		{
			static String name = "Settings";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_Settings;
		}

		virtual SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<Settings>();
		}
	};

	/** @} */
	/** @endcond */
}