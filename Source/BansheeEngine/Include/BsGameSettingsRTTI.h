//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsGameSettings.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT GameSettingsRTTI : public RTTIType <GameSettings, IReflectable, GameSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mainSceneUUID, 0)
			BS_RTTI_MEMBER_PLAIN(fullscreen, 1)
			BS_RTTI_MEMBER_PLAIN(useDesktopResolution, 2)
			BS_RTTI_MEMBER_PLAIN(resolutionWidth, 3)
			BS_RTTI_MEMBER_PLAIN(resolutionHeight, 4)
			BS_RTTI_MEMBER_PLAIN(titleBarText, 5)
		BS_END_RTTI_MEMBERS

	public:
		GameSettingsRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "GameSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GameSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<GameSettings>();
		}
	};

	/** @} */
	/** @endcond */
}