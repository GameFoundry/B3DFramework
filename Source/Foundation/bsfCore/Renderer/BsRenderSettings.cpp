//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"

namespace bs
{
	RTTITypeBase* AutoExposureSettings::getRTTIStatic()
	{
		return AutoExposureSettingsRTTI::instance();
	}

	RTTITypeBase* AutoExposureSettings::getRTTI() const
	{
		return AutoExposureSettings::getRTTIStatic();
	}

	RTTITypeBase* TonemappingSettings::getRTTIStatic()
	{
		return TonemappingSettingsRTTI::instance();
	}

	RTTITypeBase* TonemappingSettings::getRTTI() const
	{
		return TonemappingSettings::getRTTIStatic();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTIStatic()
	{
		return WhiteBalanceSettingsRTTI::instance();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTI() const
	{
		return WhiteBalanceSettings::getRTTIStatic();
	}

	RTTITypeBase* ColorGradingSettings::getRTTIStatic()
	{
		return ColorGradingSettingsRTTI::instance();
	}

	RTTITypeBase* ColorGradingSettings::getRTTI() const
	{
		return ColorGradingSettings::getRTTIStatic();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTIStatic()
	{
		return AmbientOcclusionSettingsRTTI::instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTI() const
	{
		return AmbientOcclusionSettings::getRTTIStatic();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTIStatic()
	{
		return DepthOfFieldSettingsRTTI::instance();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTI() const
	{
		return DepthOfFieldSettings::getRTTIStatic();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTIStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTI() const
	{
		return ScreenSpaceReflectionsSettings::getRTTIStatic();
	}

	RTTITypeBase* ShadowSettings::getRTTIStatic()
	{
		return ShadowSettingsRTTI::instance();
	}

	RTTITypeBase* ShadowSettings::getRTTI() const
	{
		return ShadowSettings::getRTTIStatic();
	}

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

		bufferSize += rttiGetElemSize(shadowSettings.directionalShadowDistance);
		bufferSize += rttiGetElemSize(shadowSettings.numCascades);
		bufferSize += rttiGetElemSize(shadowSettings.cascadeDistributionExponent);
		bufferSize += rttiGetElemSize(shadowSettings.shadowFilteringQuality);

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

		writeDst = rttiWriteElem(shadowSettings.directionalShadowDistance, writeDst);
		writeDst = rttiWriteElem(shadowSettings.numCascades, writeDst);
		writeDst = rttiWriteElem(shadowSettings.cascadeDistributionExponent, writeDst);
		writeDst = rttiWriteElem(shadowSettings.shadowFilteringQuality, writeDst);
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

		readSource = rttiReadElem(shadowSettings.directionalShadowDistance, readSource);
		readSource = rttiReadElem(shadowSettings.numCascades, readSource);
		readSource = rttiReadElem(shadowSettings.cascadeDistributionExponent, readSource);
		readSource = rttiReadElem(shadowSettings.shadowFilteringQuality, readSource);
	}
}