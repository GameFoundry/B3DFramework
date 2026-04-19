//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup Utility-Engine-Internal
	 *  @{
	 */

	/**
	 * Contains settings used for controlling game start-up, as well as persisting various other properties through game
	 * sessions.
	 */
	class B3D_EXPORT GameSettings : public IReflectable
	{
	public:
		GameSettings() = default;

		UUID MainSceneId; /**< Resource UUID of the default scene that is loaded when the application is started. */
		bool Fullscreen = true; /**< If true the application will be started in fullscreen using user's desktop resolution. */
		bool UseDesktopResolution = true; /**< If running in fullscreen should the user's desktop resolution be used instead of the specified resolution. */
		u32 ResolutionWidth = 1280; /**< Width of the window. */
		u32 ResolutionHeight = 720; /**< Height of the window. */
		String TitleBarText; /**< Text displayed in window's titlebar. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GameSettingsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
