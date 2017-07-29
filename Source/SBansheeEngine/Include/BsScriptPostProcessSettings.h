//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	struct PostProcessSettings;
	struct AutoExposureSettings;
	struct TonemappingSettings;
	struct WhiteBalanceSettings;
	struct ColorGradingSettings;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AutoExposureSettings. */
	class BS_SCR_BE_EXPORT ScriptAutoExposureSettings : public ScriptObject<ScriptAutoExposureSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AutoExposureSettings")

		/** Converts managed object its native counterpart. */
		static SPtr<AutoExposureSettings> toNative(MonoObject* object);

		/** Converts native object to its managed counterpart. */
		static MonoObject* toManaged(AutoExposureSettings& value);

	private:
		ScriptAutoExposureSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sHistogramLog2Min;
		static MonoField* sHistogramLog2Max;
		static MonoField* sHistogramPctLow;
		static MonoField* sHistogramPctHigh;
		static MonoField* sMinEyeAdaptation;
		static MonoField* sMaxEyeAdaptation;
		static MonoField* sEyeAdaptationSpeedUp;
		static MonoField* sEyeAdaptationSpeedDown;
	};

	/**	Interop class between C++ & CLR for TonemappingSettings. */
	class BS_SCR_BE_EXPORT ScriptTonemappingSettings : public ScriptObject<ScriptTonemappingSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "TonemappingSettings")

		/** Converts managed object its native counterpart. */
		static SPtr<TonemappingSettings> toNative(MonoObject* object);

		/** Converts native object to its managed counterpart. */
		static MonoObject* toManaged(TonemappingSettings& value);

	private:
		ScriptTonemappingSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sFilmicCurveShoulderStrength;
		static MonoField* sFilmicCurveLinearStrength;
		static MonoField* sFilmicCurveLinearAngle;
		static MonoField* sFilmicCurveToeStrength;
		static MonoField* sFilmicCurveToeNumerator;
		static MonoField* sFilmicCurveToeDenominator;
		static MonoField* sFilmicCurveLinearWhitePoint;
	};

	/**	Interop class between C++ & CLR for WhiteBalanceSettings. */
	class BS_SCR_BE_EXPORT ScriptWhiteBalanceSettings : public ScriptObject<ScriptWhiteBalanceSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "WhiteBalanceSettings")

		/** Converts managed object its native counterpart. */
		static SPtr<WhiteBalanceSettings> toNative(MonoObject* object);

		/** Converts native object to its managed counterpart. */
		static MonoObject* toManaged(WhiteBalanceSettings& value);

	private:
		ScriptWhiteBalanceSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sTemperature;
		static MonoField* sTint;
	};

	/**	Interop class between C++ & CLR for ColorGradingSettings. */
	class BS_SCR_BE_EXPORT ScriptColorGradingSettings : public ScriptObject<ScriptColorGradingSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ColorGradingSettings")

		/** Converts managed object its native counterpart. */
		static SPtr<ColorGradingSettings> toNative(MonoObject* object);

		/** Converts native object to its managed counterpart. */
		static MonoObject* toManaged(ColorGradingSettings& value);

	private:
		ScriptColorGradingSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sSaturation;
		static MonoField* sContrast;
		static MonoField* sGain;
		static MonoField* sOffset;
	};

	/**	Interop class between C++ & CLR for PostProcessSettings. */
	class BS_SCR_BE_EXPORT ScriptPostProcessSettings : public ScriptObject<ScriptPostProcessSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "PostProcessSettings")

		/** Converts managed object its native counterpart. */
		static SPtr<PostProcessSettings> toNative(MonoObject* object);

		/** Converts native object to its managed counterpart. */
		static MonoObject* toManaged(const SPtr<PostProcessSettings>& value);

	private:
		ScriptPostProcessSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_CreateDefault();

		static MonoField* sEnableAutoExposure;;
		static MonoField* sAutoExposure;
		static MonoField* sEnableTonemapping;
		static MonoField* sTonemapping;
		static MonoField* sWhiteBalance;
		static MonoField* sColorGrading;
		static MonoField* sExposureScale;
		static MonoField* sGamma;
	};

	/** @} */
}