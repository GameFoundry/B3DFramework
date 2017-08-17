//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderSettings.h"
#include "BsRenderSettingsRTTI.h"

namespace bs
{
	AutoExposureSettings::AutoExposureSettings()
		: histogramLog2Min(-8.0f), histogramLog2Max(4.0f), histogramPctLow(0.8f), histogramPctHigh(0.985f)
		, minEyeAdaptation(0.3f), maxEyeAdaptation(2.0f), eyeAdaptationSpeedUp(3.0f), eyeAdaptationSpeedDown(3.0f)
	{ }

	RTTITypeBase* AutoExposureSettings::getRTTIStatic()
	{
		return AutoExposureSettingsRTTI::instance();
	}

	RTTITypeBase* AutoExposureSettings::getRTTI() const
	{
		return AutoExposureSettings::getRTTIStatic();
	}

	TonemappingSettings::TonemappingSettings()
		: filmicCurveShoulderStrength(0.15f), filmicCurveLinearStrength(0.5f), filmicCurveLinearAngle(0.1f)
		, filmicCurveToeStrength(0.2f), filmicCurveToeNumerator(0.02f), filmicCurveToeDenominator(0.3f)
		, filmicCurveLinearWhitePoint(11.2f)
	{ }

	RTTITypeBase* TonemappingSettings::getRTTIStatic()
	{
		return TonemappingSettingsRTTI::instance();
	}

	RTTITypeBase* TonemappingSettings::getRTTI() const
	{
		return TonemappingSettings::getRTTIStatic();
	}

	WhiteBalanceSettings::WhiteBalanceSettings()
		: temperature(6500.0f), tint(0.0f)
	{ }

	RTTITypeBase* WhiteBalanceSettings::getRTTIStatic()
	{
		return WhiteBalanceSettingsRTTI::instance();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTI() const
	{
		return WhiteBalanceSettings::getRTTIStatic();
	}

	ColorGradingSettings::ColorGradingSettings()
		: saturation(Vector3::ONE), contrast(Vector3::ONE), gain(Vector3::ONE), offset(Vector3::ZERO)
	{ }

	RTTITypeBase* ColorGradingSettings::getRTTIStatic()
	{
		return ColorGradingSettingsRTTI::instance();
	}

	RTTITypeBase* ColorGradingSettings::getRTTI() const
	{
		return ColorGradingSettings::getRTTIStatic();
	}

	AmbientOcclusionSettings::AmbientOcclusionSettings()
		: enabled(true), radius(1.5f), bias(1.0f), fadeDistance(500.0f), fadeRange(50.0f), intensity(1.0f), power(4.0f)
		, quality(3)
	{ }

	RTTITypeBase* AmbientOcclusionSettings::getRTTIStatic()
	{
		return AmbientOcclusionSettingsRTTI::instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTI() const
	{
		return AmbientOcclusionSettings::getRTTIStatic();
	}

	DepthOfFieldSettings::DepthOfFieldSettings()
		: enabled(false), focalDistance(0.75f), focalRange(0.75f), nearTransitionRange(0.25f), farTransitionRange(0.25f)
		, nearBlurAmount(0.15f), farBlurAmount(0.15f)
	{ }

	RTTITypeBase* DepthOfFieldSettings::getRTTIStatic()
	{
		return DepthOfFieldSettingsRTTI::instance();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTI() const
	{
		return DepthOfFieldSettings::getRTTIStatic();
	}

	ScreenSpaceReflectionsSettings::ScreenSpaceReflectionsSettings()
		: enabled(true), intensity(1.0f), maxRoughness(0.8f), quality(2)
	{ }

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTIStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTI() const
	{
		return ScreenSpaceReflectionsSettings::getRTTIStatic();
	}

	RenderSettings::RenderSettings()
		: enableAutoExposure(true), enableTonemapping(true), enableFXAA(true), exposureScale(0.0f), gamma(2.2f)
		, enableHDR(true), enableLighting(true), enableShadows(true), enableIndirectLighting(true), overlayOnly(false)
	{ }

	RTTITypeBase* RenderSettings::getRTTIStatic()
	{
		return RenderSettingsRTTI::instance();
	}

	RTTITypeBase* RenderSettings::getRTTI() const
	{
		return RenderSettings::getRTTIStatic();
	}

	void RenderSettings::_getSyncData(UINT8* buffer, UINT32& size)
	{
		UINT32 bufferSize = 0;
		bufferSize += rttiGetElemSize(enableAutoExposure);
		bufferSize += rttiGetElemSize(enableTonemapping);
		bufferSize += rttiGetElemSize(exposureScale);
		bufferSize += rttiGetElemSize(gamma);
		bufferSize += rttiGetElemSize(enableFXAA);
		bufferSize += rttiGetElemSize(enableHDR);
		bufferSize += rttiGetElemSize(enableLighting);
		bufferSize += rttiGetElemSize(enableShadows);
		bufferSize += rttiGetElemSize(enableIndirectLighting);
		bufferSize += rttiGetElemSize(overlayOnly);

		bufferSize += rttiGetElemSize(autoExposure.histogramLog2Min);
		bufferSize += rttiGetElemSize(autoExposure.histogramLog2Max);
		bufferSize += rttiGetElemSize(autoExposure.histogramPctLow);
		bufferSize += rttiGetElemSize(autoExposure.histogramPctHigh);
		bufferSize += rttiGetElemSize(autoExposure.minEyeAdaptation);
		bufferSize += rttiGetElemSize(autoExposure.maxEyeAdaptation);
		bufferSize += rttiGetElemSize(autoExposure.eyeAdaptationSpeedUp);
		bufferSize += rttiGetElemSize(autoExposure.eyeAdaptationSpeedDown);

		bufferSize += rttiGetElemSize(tonemapping.filmicCurveShoulderStrength);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveLinearStrength);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveLinearAngle);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveToeStrength);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveToeNumerator);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveToeDenominator);
		bufferSize += rttiGetElemSize(tonemapping.filmicCurveLinearWhitePoint);

		bufferSize += rttiGetElemSize(whiteBalance.temperature);
		bufferSize += rttiGetElemSize(whiteBalance.tint);

		bufferSize += rttiGetElemSize(colorGrading.saturation);
		bufferSize += rttiGetElemSize(colorGrading.contrast);
		bufferSize += rttiGetElemSize(colorGrading.gain);
		bufferSize += rttiGetElemSize(colorGrading.offset);

		bufferSize += rttiGetElemSize(depthOfField.enabled);
		bufferSize += rttiGetElemSize(depthOfField.focalDistance);
		bufferSize += rttiGetElemSize(depthOfField.focalRange);
		bufferSize += rttiGetElemSize(depthOfField.nearTransitionRange);
		bufferSize += rttiGetElemSize(depthOfField.farTransitionRange);
		bufferSize += rttiGetElemSize(depthOfField.nearBlurAmount);
		bufferSize += rttiGetElemSize(depthOfField.farBlurAmount);

		bufferSize += rttiGetElemSize(ambientOcclusion.enabled);
		bufferSize += rttiGetElemSize(ambientOcclusion.radius);
		bufferSize += rttiGetElemSize(ambientOcclusion.bias);
		bufferSize += rttiGetElemSize(ambientOcclusion.fadeRange);
		bufferSize += rttiGetElemSize(ambientOcclusion.fadeDistance);
		bufferSize += rttiGetElemSize(ambientOcclusion.intensity);
		bufferSize += rttiGetElemSize(ambientOcclusion.power);
		bufferSize += rttiGetElemSize(ambientOcclusion.quality);

		bufferSize += rttiGetElemSize(screenSpaceReflections.enabled);
		bufferSize += rttiGetElemSize(screenSpaceReflections.intensity);
		bufferSize += rttiGetElemSize(screenSpaceReflections.maxRoughness);
		bufferSize += rttiGetElemSize(screenSpaceReflections.quality);

		if (buffer == nullptr)
		{
			size = bufferSize;
			return;
		}

		if (bufferSize != size)
		{
			LOGERR("Invalid buffer size");
			return;
		}

		char* writeDst = (char*)buffer;
		writeDst = rttiWriteElem(enableAutoExposure, writeDst);
		writeDst = rttiWriteElem(enableTonemapping, writeDst);
		writeDst = rttiWriteElem(exposureScale, writeDst);
		writeDst = rttiWriteElem(gamma, writeDst);
		writeDst = rttiWriteElem(enableFXAA, writeDst);
		writeDst = rttiWriteElem(enableHDR, writeDst);
		writeDst = rttiWriteElem(enableLighting, writeDst);
		writeDst = rttiWriteElem(enableShadows, writeDst);
		writeDst = rttiWriteElem(enableIndirectLighting, writeDst);
		writeDst = rttiWriteElem(overlayOnly, writeDst);

		writeDst = rttiWriteElem(autoExposure.histogramLog2Min, writeDst);
		writeDst = rttiWriteElem(autoExposure.histogramLog2Max, writeDst);
		writeDst = rttiWriteElem(autoExposure.histogramPctLow, writeDst);
		writeDst = rttiWriteElem(autoExposure.histogramPctHigh, writeDst);
		writeDst = rttiWriteElem(autoExposure.minEyeAdaptation, writeDst);
		writeDst = rttiWriteElem(autoExposure.maxEyeAdaptation, writeDst);
		writeDst = rttiWriteElem(autoExposure.eyeAdaptationSpeedUp, writeDst);
		writeDst = rttiWriteElem(autoExposure.eyeAdaptationSpeedDown, writeDst);

		writeDst = rttiWriteElem(tonemapping.filmicCurveShoulderStrength, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveLinearStrength, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveLinearAngle, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveToeStrength, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveToeNumerator, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveToeDenominator, writeDst);
		writeDst = rttiWriteElem(tonemapping.filmicCurveLinearWhitePoint, writeDst);

		writeDst = rttiWriteElem(whiteBalance.temperature, writeDst);
		writeDst = rttiWriteElem(whiteBalance.tint, writeDst);

		writeDst = rttiWriteElem(colorGrading.saturation, writeDst);
		writeDst = rttiWriteElem(colorGrading.contrast, writeDst);
		writeDst = rttiWriteElem(colorGrading.gain, writeDst);
		writeDst = rttiWriteElem(colorGrading.offset, writeDst);

		writeDst = rttiWriteElem(depthOfField.enabled, writeDst);
		writeDst = rttiWriteElem(depthOfField.focalDistance, writeDst);
		writeDst = rttiWriteElem(depthOfField.focalRange, writeDst);
		writeDst = rttiWriteElem(depthOfField.nearTransitionRange, writeDst);
		writeDst = rttiWriteElem(depthOfField.farTransitionRange, writeDst);
		writeDst = rttiWriteElem(depthOfField.nearBlurAmount, writeDst);
		writeDst = rttiWriteElem(depthOfField.farBlurAmount, writeDst);

		writeDst = rttiWriteElem(ambientOcclusion.enabled, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.radius, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.bias, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.fadeRange, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.fadeDistance, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.intensity, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.power, writeDst);
		writeDst = rttiWriteElem(ambientOcclusion.quality, writeDst);

		writeDst = rttiWriteElem(screenSpaceReflections.enabled, writeDst);
		writeDst = rttiWriteElem(screenSpaceReflections.intensity, writeDst);
		writeDst = rttiWriteElem(screenSpaceReflections.maxRoughness, writeDst);
		writeDst = rttiWriteElem(screenSpaceReflections.quality, writeDst);
	}

	void RenderSettings::_setSyncData(UINT8* buffer, UINT32 size)
	{
		char* readSource = (char*)buffer;

		readSource = rttiReadElem(enableAutoExposure, readSource);
		readSource = rttiReadElem(enableTonemapping, readSource);
		readSource = rttiReadElem(exposureScale, readSource);
		readSource = rttiReadElem(gamma, readSource);
		readSource = rttiReadElem(enableFXAA, readSource);
		readSource = rttiReadElem(enableHDR, readSource);
		readSource = rttiReadElem(enableLighting, readSource);
		readSource = rttiReadElem(enableShadows, readSource);
		readSource = rttiReadElem(enableIndirectLighting, readSource);
		readSource = rttiReadElem(overlayOnly, readSource);

		readSource = rttiReadElem(autoExposure.histogramLog2Min, readSource);
		readSource = rttiReadElem(autoExposure.histogramLog2Max, readSource);
		readSource = rttiReadElem(autoExposure.histogramPctLow, readSource);
		readSource = rttiReadElem(autoExposure.histogramPctHigh, readSource);
		readSource = rttiReadElem(autoExposure.minEyeAdaptation, readSource);
		readSource = rttiReadElem(autoExposure.maxEyeAdaptation, readSource);
		readSource = rttiReadElem(autoExposure.eyeAdaptationSpeedUp, readSource);
		readSource = rttiReadElem(autoExposure.eyeAdaptationSpeedDown, readSource);

		readSource = rttiReadElem(tonemapping.filmicCurveShoulderStrength, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveLinearStrength, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveLinearAngle, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveToeStrength, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveToeNumerator, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveToeDenominator, readSource);
		readSource = rttiReadElem(tonemapping.filmicCurveLinearWhitePoint, readSource);

		readSource = rttiReadElem(whiteBalance.temperature, readSource);
		readSource = rttiReadElem(whiteBalance.tint, readSource);

		readSource = rttiReadElem(colorGrading.saturation, readSource);
		readSource = rttiReadElem(colorGrading.contrast, readSource);
		readSource = rttiReadElem(colorGrading.gain, readSource);
		readSource = rttiReadElem(colorGrading.offset, readSource);

		readSource = rttiReadElem(depthOfField.enabled, readSource);
		readSource = rttiReadElem(depthOfField.focalDistance, readSource);
		readSource = rttiReadElem(depthOfField.focalRange, readSource);
		readSource = rttiReadElem(depthOfField.nearTransitionRange, readSource);
		readSource = rttiReadElem(depthOfField.farTransitionRange, readSource);
		readSource = rttiReadElem(depthOfField.nearBlurAmount, readSource);
		readSource = rttiReadElem(depthOfField.farBlurAmount, readSource);

		readSource = rttiReadElem(ambientOcclusion.enabled, readSource);
		readSource = rttiReadElem(ambientOcclusion.radius, readSource);
		readSource = rttiReadElem(ambientOcclusion.bias, readSource);
		readSource = rttiReadElem(ambientOcclusion.fadeRange, readSource);
		readSource = rttiReadElem(ambientOcclusion.fadeDistance, readSource);
		readSource = rttiReadElem(ambientOcclusion.intensity, readSource);
		readSource = rttiReadElem(ambientOcclusion.power, readSource);
		readSource = rttiReadElem(ambientOcclusion.quality, readSource);

		readSource = rttiReadElem(screenSpaceReflections.enabled, readSource);
		readSource = rttiReadElem(screenSpaceReflections.intensity, readSource);
		readSource = rttiReadElem(screenSpaceReflections.maxRoughness, readSource);
		readSource = rttiReadElem(screenSpaceReflections.quality, readSource);
	}
}