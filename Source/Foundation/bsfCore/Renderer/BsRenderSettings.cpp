//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	template <class Processor>
	void AutoExposureSettings::rttiEnumFields(Processor p)
	{
		p(histogramLog2Min);
		p(histogramLog2Max);
		p(histogramPctLow);
		p(histogramPctHigh);
		p(minEyeAdaptation);
		p(maxEyeAdaptation);
		p(eyeAdaptationSpeedUp);
		p(eyeAdaptationSpeedDown);
	}

	RTTITypeBase* AutoExposureSettings::getRTTIStatic()
	{
		return AutoExposureSettingsRTTI::instance();
	}

	RTTITypeBase* AutoExposureSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void TonemappingSettings::rttiEnumFields(Processor p)
	{
		p(filmicCurveShoulderStrength);
		p(filmicCurveLinearStrength);
		p(filmicCurveLinearAngle);
		p(filmicCurveToeStrength);
		p(filmicCurveToeNumerator);
		p(filmicCurveToeDenominator);
		p(filmicCurveLinearWhitePoint);
	}

	RTTITypeBase* TonemappingSettings::getRTTIStatic()
	{
		return TonemappingSettingsRTTI::instance();
	}

	RTTITypeBase* TonemappingSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void WhiteBalanceSettings::rttiEnumFields(Processor p)
	{
		p(temperature);
		p(tint);
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTIStatic()
	{
		return WhiteBalanceSettingsRTTI::instance();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ColorGradingSettings::rttiEnumFields(Processor p)
	{
		p(saturation);
		p(gain);
		p(contrast);
		p(offset);
	}

	RTTITypeBase* ColorGradingSettings::getRTTIStatic()
	{
		return ColorGradingSettingsRTTI::instance();
	}

	RTTITypeBase* ColorGradingSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void AmbientOcclusionSettings::rttiEnumFields(Processor p)
	{
		p(enabled);
		p(radius);
		p(bias);
		p(fadeDistance);
		p(fadeRange);
		p(intensity);
		p(power);
		p(quality);
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTIStatic()
	{
		return AmbientOcclusionSettingsRTTI::instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void DepthOfFieldSettings::rttiEnumFields(Processor p)
	{
		p(enabled);
		p(focalDistance);
		p(focalRange);
		p(nearTransitionRange);
		p(farTransitionRange);
		p(nearBlurAmount);
		p(farBlurAmount);
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTIStatic()
	{
		return DepthOfFieldSettingsRTTI::instance();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ScreenSpaceReflectionsSettings::rttiEnumFields(Processor p)
	{
		p(enabled);
		p(quality);
		p(intensity);
		p(maxRoughness);
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTIStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void BloomSettings::rttiEnumFields(Processor p)
	{
		p(enabled);
		p(quality);
		p(threshold);
		p(intensity);
		p(tint);
	}

	RTTITypeBase* BloomSettings::getRTTIStatic()
	{
		return BloomSettingsRTTI::instance();
	}

	RTTITypeBase* BloomSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ShadowSettings::rttiEnumFields(Processor p)
	{
		p(directionalShadowDistance);
		p(numCascades);
		p(cascadeDistributionExponent);
		p(shadowFilteringQuality);
	}

	RTTITypeBase* ShadowSettings::getRTTIStatic()
	{
		return ShadowSettingsRTTI::instance();
	}

	RTTITypeBase* ShadowSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void RenderSettings::rttiEnumFields(Processor p)
	{
		p(enableAutoExposure);
		p(autoExposure);
		p(enableTonemapping);
		p(tonemapping);
		p(whiteBalance);
		p(colorGrading);
		p(depthOfField);
		p(ambientOcclusion);
		p(screenSpaceReflections);
		p(bloom);
		p(exposureScale);
		p(gamma);
		p(enableFXAA);
		p(enableHDR);
		p(enableLighting);
		p(enableShadows);
		p(enableIndirectLighting);
		p(overlayOnly);
		p(enableSkybox);
		p(cullDistance);
	}

	template void RenderSettings::rttiEnumFields(RttiCoreSyncSize);
	template void RenderSettings::rttiEnumFields(RttiCoreSyncWriter);
	template void RenderSettings::rttiEnumFields(RttiCoreSyncReader);

	RTTITypeBase* RenderSettings::getRTTIStatic()
	{
		return RenderSettingsRTTI::instance();
	}

	RTTITypeBase* RenderSettings::getRTTI() const
	{
		return getRTTIStatic();
	}
}
