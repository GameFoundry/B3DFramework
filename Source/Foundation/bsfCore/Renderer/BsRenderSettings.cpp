//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "Image/BsTexture.h"

using namespace bs;

template <class Processor>
void AutoExposureSettings::RttiEnumFields(Processor p)
{
	p(HistogramLog2Min);
	p(HistogramLog2Max);
	p(HistogramPctLow);
	p(HistogramPctHigh);
	p(MinEyeAdaptation);
	p(MaxEyeAdaptation);
	p(EyeAdaptationSpeedUp);
	p(EyeAdaptationSpeedDown);
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
	p(FilmicCurveShoulderStrength);
	p(FilmicCurveLinearStrength);
	p(FilmicCurveLinearAngle);
	p(FilmicCurveToeStrength);
	p(FilmicCurveToeNumerator);
	p(FilmicCurveToeDenominator);
	p(FilmicCurveLinearWhitePoint);
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
	p(Temperature);
	p(Tint);
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
	p(Saturation);
	p(Gain);
	p(Contrast);
	p(Offset);
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
	p(Enabled);
	p(Radius);
	p(Bias);
	p(FadeDistance);
	p(FadeRange);
	p(Intensity);
	p(Power);
	p(Quality);
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
	p(Enabled);
	p(FocalDistance);
	p(FocalRange);
	p(NearTransitionRange);
	p(FarTransitionRange);
	p(NearBlurAmount);
	p(FarBlurAmount);
	p(Type);
	p(MaxBokehSize);
	p(BokehShape);
	p(AdaptiveColorThreshold);
	p(AdaptiveRadiusThreshold);
	p(FocalLength);
	p(ApertureSize);
	p(SensorSize);
	p(BokehOcclusion);
	p(OcclusionDepthRange);
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
	p(Enabled);
	p(Quality);
	p(Intensity);
	p(MaxRoughness);
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
	p(Enabled);
	p(Quality);
	p(Threshold);
	p(Intensity);
	p(Tint);
	p(FilterSize);
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
	p(Enabled);
	p(DownsampleCount);
	p(Threshold);
	p(GhostCount);
	p(GhostSpacing);
	p(Brightness);
	p(FilterSize);
	p(Halo);
	p(HaloRadius);
	p(HaloThickness);
	p(HaloThreshold);
	p(HaloAspectRatio);
	p(ChromaticAberration);
	p(ChromaticAberrationOffset);
	p(BicubicUpsampling);
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
	p(Enabled);
	p(Domain);
	p(Filter);
	p(Quality);
	p(MaximumRadius);
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
	p(Enabled);
	p(JitteredPositionCount);
	p(Sharpness);
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
	p(Enabled);
	p(Type);
	p(ShiftAmount);
	p(FringeTexture);
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
	p(Enabled);
	p(Intensity);
	p(Speed);
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
	p(EnableAutoExposure);
	p(AutoExposure);
	p(EnableTonemapping);
	p(Tonemapping);
	p(WhiteBalance);
	p(ColorGrading);
	p(DepthOfField);
	p(AmbientOcclusion);
	p(ScreenSpaceReflections);
	p(Bloom);
	p(ScreenSpaceLensFlare);
	p(ExposureScale);
	p(Gamma);
	p(EnableFxaa);
	p(EnableHdr);
	p(EnableLighting);
	p(EnableShadows);
	p(EnableIndirectLighting);
	p(OverlayOnly);
	p(EnableSkybox);
	p(CullDistance);
	p(MotionBlur);
	p(FilmGrain);
	p(ChromaticAberration);
	p(TemporalAa);
	p(EnableVelocityBuffer);
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
