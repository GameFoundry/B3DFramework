//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsPostProcessSettings.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PostProcessSettings. */
	class BS_SCR_BE_EXPORT ScriptPostProcessSettings : public ScriptObject<ScriptPostProcessSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "PostProcessSettings")

	private:
		ScriptPostProcessSettings(MonoObject* instance);

		SPtr<PostProcessSettings> mSettings;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* managedInstance);

		static bool internal_GetEnableAutoExposure(ScriptPostProcessSettings* thisPtr);
		static void internal_SetEnableAutoExposure(ScriptPostProcessSettings* thisPtr, bool value);

		static void internal_GetAutoExposureSettings(ScriptPostProcessSettings* thisPtr, AutoExposureSettings* value);
		static void internal_SetAutoExposureSettings(ScriptPostProcessSettings* thisPtr, AutoExposureSettings* value);

		static bool internal_GetEnableTonemapping(ScriptPostProcessSettings* thisPtr);
		static void internal_SetEnableTonemapping(ScriptPostProcessSettings* thisPtr, bool value);

		static void internal_GetTonemappingSettings(ScriptPostProcessSettings* thisPtr, TonemappingSettings* value);
		static void internal_SetTonemappingSettings(ScriptPostProcessSettings* thisPtr, TonemappingSettings* value);

		static void internal_GetWhiteBalanceSettings(ScriptPostProcessSettings* thisPtr, WhiteBalanceSettings* value);
		static void internal_SetWhiteBalanceSettings(ScriptPostProcessSettings* thisPtr, WhiteBalanceSettings* value);

		static void internal_GetColorGradingSettings(ScriptPostProcessSettings* thisPtr, ColorGradingSettings* value);
		static void internal_SetColorGradingSettings(ScriptPostProcessSettings* thisPtr, ColorGradingSettings* value);

		static float internal_GetExposureScale(ScriptPostProcessSettings* thisPtr);
		static void internal_SetExposureScale(ScriptPostProcessSettings* thisPtr, float value);

		static float internal_GetGamma(ScriptPostProcessSettings* thisPtr);
		static void internal_SetGamma(ScriptPostProcessSettings* thisPtr, float value);
	};

	/** @} */
}