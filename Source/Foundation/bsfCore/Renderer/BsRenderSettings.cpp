//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "Image/BsTexture.h"

namespace bs
{
	template <class Processor>
	void AutoExposureSettings::RttiEnumFields(Processor p)
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

	RTTITypeBase* AutoExposureSettings::GetRttiStatic()
	{
		return AutoExposureSettingsRTTI::Instance();
	}

	RTTITypeBase* AutoExposureSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void TonemappingSettings::RttiEnumFields(Processor p)
	{
		p(filmicCurveShoulderStrength);
		p(filmicCurveLinearStrength);
		p(filmicCurveLinearAngle);
		p(filmicCurveToeStrength);
		p(filmicCurveToeNumerator);
		p(filmicCurveToeDenominator);
		p(filmicCurveLinearWhitePoint);
	}

	RTTITypeBase* TonemappingSettings::GetRttiStatic()
	{
		return TonemappingSettingsRTTI::Instance();
	}

	RTTITypeBase* TonemappingSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void WhiteBalanceSettings::RttiEnumFields(Processor p)
	{
		p(temperature);
		p(tint);
	}

	RTTITypeBase* WhiteBalanceSettings::GetRttiStatic()
	{
		return WhiteBalanceSettingsRTTI::Instance();
	}

	RTTITypeBase* WhiteBalanceSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void ColorGradingSettings::RttiEnumFields(Processor p)
	{
		p(saturation);
		p(gain);
		p(contrast);
		p(offset);
	}

	RTTITypeBase* ColorGradingSettings::GetRttiStatic()
	{
		return ColorGradingSettingsRTTI::Instance();
	}

	RTTITypeBase* ColorGradingSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void AmbientOcclusionSettings::RttiEnumFields(Processor p)
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

	RTTITypeBase* AmbientOcclusionSettings::GetRttiStatic()
	{
		return AmbientOcclusionSettingsRTTI::Instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <bool Core>
	template <class Processor>
	void TDepthOfFieldSettings<Core>::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(focalDistance);
		p(focalRange);
		p(nearTransitionRange);
		p(farTransitionRange);
		p(nearBlurAmount);
		p(farBlurAmount);
		p(type);
		p(maxBokehSize);
		p(bokehShape);
		p(adaptiveColorThreshold);
		p(adaptiveRadiusThreshold);
		p(focalLength);
		p(apertureSize);
		p(sensorSize);
		p(bokehOcclusion);
		p(occlusionDepthRange);
	}

	template struct TDepthOfFieldSettings<false>;
	template struct TDepthOfFieldSettings<true>;

	RTTITypeBase* DepthOfFieldSettings::GetRttiStatic()
	{
		return DepthOfFieldSettingsRTTI::Instance();
	}

	RTTITypeBase* DepthOfFieldSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void ScreenSpaceReflectionsSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(quality);
		p(intensity);
		p(maxRoughness);
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::GetRttiStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::Instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void BloomSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(quality);
		p(threshold);
		p(intensity);
		p(tint);
		p(filterSize);
	}

	RTTITypeBase* BloomSettings::GetRttiStatic()
	{
		return BloomSettingsRTTI::Instance();
	}

	RTTITypeBase* BloomSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void ScreenSpaceLensFlareSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(downsampleCount);
		p(threshold);
		p(ghostCount);
		p(ghostSpacing);
		p(brightness);
		p(filterSize);
		p(halo);
		p(haloRadius);
		p(haloThickness);
		p(haloThreshold);
		p(haloAspectRatio);
		p(chromaticAberration);
		p(chromaticAberrationOffset);
		p(bicubicUpsampling);
	}

	RTTITypeBase* ScreenSpaceLensFlareSettings::GetRttiStatic()
	{
		return ScreenSpaceLensFlareSettingsRTTI::Instance();
	}

	RTTITypeBase* ScreenSpaceLensFlareSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void MotionBlurSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(domain);
		p(filter);
		p(quality);
		p(maximumRadius);
	}

	RTTITypeBase* MotionBlurSettings::GetRttiStatic()
	{
		return MotionBlurSettingsRTTI::Instance();
	}

	RTTITypeBase* MotionBlurSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void TemporalAASettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(jitteredPositionCount);
		p(sharpness);
	}

	RTTITypeBase* TemporalAASettings::GetRttiStatic()
	{
		return TemporalAASettingsRTTI::Instance();
	}

	RTTITypeBase* TemporalAASettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <bool Core>
	template <class Processor>
	void TChromaticAberrationSettings<Core>::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(type);
		p(shiftAmount);
		p(fringeTexture);
	}

	template struct TChromaticAberrationSettings<false>;
	template struct TChromaticAberrationSettings<true>;

	RTTITypeBase* ChromaticAberrationSettings::GetRttiStatic()
	{
		return ChromaticAberrationSettingsRTTI::Instance();
	}

	RTTITypeBase* ChromaticAberrationSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void FilmGrainSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(intensity);
		p(speed);
	}

	RTTITypeBase* FilmGrainSettings::GetRttiStatic()
	{
		return FilmGrainSettingsRTTI::Instance();
	}

	RTTITypeBase* FilmGrainSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <class Processor>
	void ShadowSettings::RttiEnumFields(Processor p)
	{
		p(directionalShadowDistance);
		p(numCascades);
		p(cascadeDistributionExponent);
		p(shadowFilteringQuality);
	}

	RTTITypeBase* ShadowSettings::GetRttiStatic()
	{
		return ShadowSettingsRTTI::Instance();
	}

	RTTITypeBase* ShadowSettings::GetRtti() const
	{
		return GetRttiStatic();
	}

	template <bool Core>
	template <class Processor>
	void TRenderSettings<Core>::RttiEnumFields(Processor p)
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
		p(screenSpaceLensFlare);
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
		p(motionBlur);
		p(filmGrain);
		p(chromaticAberration);
		p(temporalAA);
		p(enableVelocityBuffer);
	}

	template struct TRenderSettings<false>;
	template struct TRenderSettings<true>;

	template void TRenderSettings<false>::RttiEnumFields(RttiCoreSyncSize);
	template void TRenderSettings<false>::RttiEnumFields(RttiCoreSyncWriter);

	template void TRenderSettings<true>::RttiEnumFields(RttiCoreSyncSize);
	template void TRenderSettings<true>::RttiEnumFields(RttiCoreSyncReader);

	RTTITypeBase* RenderSettings::GetRttiStatic()
	{
		return RenderSettingsRTTI::Instance();
	}

	RTTITypeBase* RenderSettings::GetRtti() const
	{
		return GetRttiStatic();
	}
}
