//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPostProcessSettings.h"
#include "BsPostProcessSettingsRTTI.h"

namespace BansheeEngine
{
	AutoExposureSettings::AutoExposureSettings()
		: histogramLog2Min(-8.0f), histogramLog2Max(4.0f), histogramPctLow(0.8f), histogramPctHigh(0.985f)
		, minEyeAdaptation(0.3f), maxEyeAdaptation(2.0f), eyeAdaptationSpeedUp(3.0f), eyeAdaptationSpeedDown(3.0f)
	{ }

	TonemappingSettings::TonemappingSettings()
		: filmicCurveShoulderStrength(0.22f), filmicCurveLinearStrength(0.3f), filmicCurveLinearAngle(0.1f)
		, filmicCurveToeStrength(0.2f), filmicCurveToeNumerator(0.01f), filmicCurveToeDenominator(0.3f)
		, filmicCurveLinearWhitePoint(11.2f)
	{ }

	WhiteBalanceSettings::WhiteBalanceSettings()
		: temperature(6500.0f), tint(0.0f)
	{ }

	ColorGradingSettings::ColorGradingSettings()
		: saturation(Vector3::ONE), contrast(Vector3::ONE), gain(Vector3::ONE), offset(Vector3::ZERO)
	{ }

	PostProcessSettings::PostProcessSettings()
		: enableAutoExposure(true), enableTonemapping(true), exposureScale(0.0f), gamma(2.2f)
	{ }

	SPtr<PostProcessSettings> PostProcessSettings::create()
	{
		return bs_shared_ptr_new<PostProcessSettings>();
	}

	RTTITypeBase* PostProcessSettings::getRTTIStatic()
	{
		return PostProcessSettingsRTTI::instance();
	}

	RTTITypeBase* PostProcessSettings::getRTTI() const
	{
		return PostProcessSettings::getRTTIStatic();
	}
}