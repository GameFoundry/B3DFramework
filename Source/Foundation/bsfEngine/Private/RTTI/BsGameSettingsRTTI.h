//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Utility/BsGameSettings.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT GameSettingsRTTI : public RTTIType<GameSettings, IReflectable, GameSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(MainSceneUuid, 0)
			BS_RTTI_MEMBER_PLAIN(Fullscreen, 1)
			BS_RTTI_MEMBER_PLAIN(UseDesktopResolution, 2)
			BS_RTTI_MEMBER_PLAIN(ResolutionWidth, 3)
			BS_RTTI_MEMBER_PLAIN(ResolutionHeight, 4)
			BS_RTTI_MEMBER_PLAIN(TitleBarText, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "GameSettings";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_GameSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GameSettings>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
