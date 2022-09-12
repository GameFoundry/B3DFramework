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

	RTTITypeBase* AutoExposureSettings::getRTTIStatic()
	{
		return AutoExposureSettingsRTTI::Instance();
	}

	RTTITypeBase* AutoExposureSettings::getRTTI() const
	{
		return GetRTTIStatic();
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

	RTTITypeBase* TonemappingSettings::getRTTIStatic()
	{
		return TonemappingSettingsRTTI::Instance();
	}

	RTTITypeBase* TonemappingSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void WhiteBalanceSettings::RttiEnumFields(Processor p)
	{
		p(temperature);
		p(tint);
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTIStatic()
	{
		return WhiteBalanceSettingsRTTI::Instance();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void ColorGradingSettings::RttiEnumFields(Processor p)
	{
		p(saturation);
		p(gain);
		p(contrast);
		p(offset);
	}

	RTTITypeBase* ColorGradingSettings::getRTTIStatic()
	{
		return ColorGradingSettingsRTTI::Instance();
	}

	RTTITypeBase* ColorGradingSettings::getRTTI() const
	{
		return GetRTTIStatic();
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

	RTTITypeBase* AmbientOcclusionSettings::getRTTIStatic()
	{
		return AmbientOcclusionSettingsRTTI::Instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <bool Core>
	template <class Processor>
	void TDepthOfFieldSettings<Core>::rttiEnumFields(Processor p)
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

	RTTITypeBase* DepthOfFieldSettings::getRTTIStatic()
	{
		return DepthOfFieldSettingsRTTI::Instance();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void ScreenSpaceReflectionsSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(quality);
		p(intensity);
		p(maxRoughness);
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTIStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::Instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTI() const
	{
		return GetRTTIStatic();
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

	RTTITypeBase* BloomSettings::getRTTIStatic()
	{
		return BloomSettingsRTTI::Instance();
	}

	RTTITypeBase* BloomSettings::getRTTI() const
	{
		return GetRTTIStatic();
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

	RTTITypeBase* ScreenSpaceLensFlareSettings::getRTTIStatic()
	{
		return ScreenSpaceLensFlareSettingsRTTI::Instance();
	}

	RTTITypeBase* ScreenSpaceLensFlareSettings::getRTTI() const
	{
		return GetRTTIStatic();
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

	RTTITypeBase* MotionBlurSettings::getRTTIStatic()
	{
		return MotionBlurSettingsRTTI::Instance();
	}

	RTTITypeBase* MotionBlurSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void TemporalAASettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(jitteredPositionCount);
		p(sharpness);
	}

	RTTITypeBase* TemporalAASettings::getRTTIStatic()
	{
		return TemporalAASettingsRTTI::Instance();
	}

	RTTITypeBase* TemporalAASettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <bool Core>
	template <class Processor>
	void TChromaticAberrationSettings<Core>::rttiEnumFields(Processor p)
	{
		p(enabled);
		p(type);
		p(shiftAmount);
		p(fringeTexture);
	}

	template struct TChromaticAberrationSettings<false>;
	template struct TChromaticAberrationSettings<true>;

	RTTITypeBase* ChromaticAberrationSettings::getRTTIStatic()
	{
		return ChromaticAberrationSettingsRTTI::Instance();
	}

	RTTITypeBase* ChromaticAberrationSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void FilmGrainSettings::RttiEnumFields(Processor p)
	{
		p(enabled);
		p(intensity);
		p(speed);
	}

	RTTITypeBase* FilmGrainSettings::getRTTIStatic()
	{
		return FilmGrainSettingsRTTI::Instance();
	}

	RTTITypeBase* FilmGrainSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <class Processor>
	void ShadowSettings::RttiEnumFields(Processor p)
	{
		p(directionalShadowDistance);
		p(numCascades);
		p(cascadeDistributionExponent);
		p(shadowFilteringQuality);
	}

	RTTITypeBase* ShadowSettings::getRTTIStatic()
	{
		return ShadowSettingsRTTI::Instance();
	}

	RTTITypeBase* ShadowSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}

	template <bool Core>
	template <class Processor>
	void TRenderSettings<Core>::rttiEnumFields(Processor p)
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

	template void TRenderSettings<false>::rttiEnumFields(RttiCoreSyncSize);
	template void TRenderSettings<false>::rttiEnumFields(RttiCoreSyncWriter);

	template void TRenderSettings<true>::rttiEnumFields(RttiCoreSyncSize);
	template void TRenderSettings<true>::rttiEnumFields(RttiCoreSyncReader);

	RTTITypeBase* RenderSettings::getRTTIStatic()
	{
		return RenderSettingsRTTI::Instance();
	}

	RTTITypeBase* RenderSettings::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
