//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRenderSettings.h"
#include "RTTI/B3DRenderSettingsRTTI.h"
#include "B3DRenderSettings.implementation.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Image/B3DTexture.h"

using namespace b3d;

RTTIType* AutoExposureSettings::GetRttiStatic()
{
	return AutoExposureSettingsRTTI::Instance();
}

RTTIType* AutoExposureSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* TonemappingSettings::GetRttiStatic()
{
	return TonemappingSettingsRTTI::Instance();
}

RTTIType* TonemappingSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* WhiteBalanceSettings::GetRttiStatic()
{
	return WhiteBalanceSettingsRTTI::Instance();
}

RTTIType* WhiteBalanceSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ColorGradingSettings::GetRttiStatic()
{
	return ColorGradingSettingsRTTI::Instance();
}

RTTIType* ColorGradingSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* AmbientOcclusionSettings::GetRttiStatic()
{
	return AmbientOcclusionSettingsRTTI::Instance();
}

RTTIType* AmbientOcclusionSettings::GetRtti() const
{
	return GetRttiStatic();
}

template struct TDepthOfFieldSettings<false>;
template struct TDepthOfFieldSettings<true>;

RTTIType* DepthOfFieldSettings::GetRttiStatic()
{
	return DepthOfFieldSettingsRTTI::Instance();
}

RTTIType* DepthOfFieldSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ScreenSpaceReflectionsSettings::GetRttiStatic()
{
	return ScreenSpaceReflectionsSettingsRTTI::Instance();
}

RTTIType* ScreenSpaceReflectionsSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* BloomSettings::GetRttiStatic()
{
	return BloomSettingsRTTI::Instance();
}

RTTIType* BloomSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ScreenSpaceLensFlareSettings::GetRttiStatic()
{
	return ScreenSpaceLensFlareSettingsRTTI::Instance();
}

RTTIType* ScreenSpaceLensFlareSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* MotionBlurSettings::GetRttiStatic()
{
	return MotionBlurSettingsRTTI::Instance();
}

RTTIType* MotionBlurSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* TemporalAASettings::GetRttiStatic()
{
	return TemporalAASettingsRTTI::Instance();
}

RTTIType* TemporalAASettings::GetRtti() const
{
	return GetRttiStatic();
}

template struct TChromaticAberrationSettings<false>;
template struct TChromaticAberrationSettings<true>;

RTTIType* ChromaticAberrationSettings::GetRttiStatic()
{
	return ChromaticAberrationSettingsRTTI::Instance();
}

RTTIType* ChromaticAberrationSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* FilmGrainSettings::GetRttiStatic()
{
	return FilmGrainSettingsRTTI::Instance();
}

RTTIType* FilmGrainSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ShadowSettings::GetRttiStatic()
{
	return ShadowSettingsRTTI::Instance();
}

RTTIType* ShadowSettings::GetRtti() const
{
	return GetRttiStatic();
}

template struct TRenderSettings<false>;
template struct TRenderSettings<true>;

RTTIType* RenderSettings::GetRttiStatic()
{
	return RenderSettingsRTTI::Instance();
}

RTTIType* RenderSettings::GetRtti() const
{
	return GetRttiStatic();
}
