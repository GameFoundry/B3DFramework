//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "Utility/B3DGameSettings.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT GameSettingsRTTI : public TRTTIType<GameSettings, IReflectable, GameSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(MainSceneId, 0)
			B3D_RTTI_MEMBER(Fullscreen, 1)
			B3D_RTTI_MEMBER(UseDesktopResolution, 2)
			B3D_RTTI_MEMBER(ResolutionWidth, 3)
			B3D_RTTI_MEMBER(ResolutionHeight, 4)
			B3D_RTTI_MEMBER(TitleBarText, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "GameSettings";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace b3d
