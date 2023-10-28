//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"
#include "BsRenderSettings.implementation.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Image/BsTexture.h"

using namespace bs;

RTTITypeBase* AutoExposureSettings::GetRttiStatic()
{
	return AutoExposureSettingsRTTI::Instance();
}

RTTITypeBase* AutoExposureSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* TonemappingSettings::GetRttiStatic()
{
	return TonemappingSettingsRTTI::Instance();
}

RTTITypeBase* TonemappingSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* WhiteBalanceSettings::GetRttiStatic()
{
	return WhiteBalanceSettingsRTTI::Instance();
}

RTTITypeBase* WhiteBalanceSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* ColorGradingSettings::GetRttiStatic()
{
	return ColorGradingSettingsRTTI::Instance();
}

RTTITypeBase* ColorGradingSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* AmbientOcclusionSettings::GetRttiStatic()
{
	return AmbientOcclusionSettingsRTTI::Instance();
}

RTTITypeBase* AmbientOcclusionSettings::GetRtti() const
{
	return GetRttiStatic();
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

RTTITypeBase* ScreenSpaceReflectionsSettings::GetRttiStatic()
{
	return ScreenSpaceReflectionsSettingsRTTI::Instance();
}

RTTITypeBase* ScreenSpaceReflectionsSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* BloomSettings::GetRttiStatic()
{
	return BloomSettingsRTTI::Instance();
}

RTTITypeBase* BloomSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* ScreenSpaceLensFlareSettings::GetRttiStatic()
{
	return ScreenSpaceLensFlareSettingsRTTI::Instance();
}

RTTITypeBase* ScreenSpaceLensFlareSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* MotionBlurSettings::GetRttiStatic()
{
	return MotionBlurSettingsRTTI::Instance();
}

RTTITypeBase* MotionBlurSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* TemporalAASettings::GetRttiStatic()
{
	return TemporalAASettingsRTTI::Instance();
}

RTTITypeBase* TemporalAASettings::GetRtti() const
{
	return GetRttiStatic();
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

RTTITypeBase* FilmGrainSettings::GetRttiStatic()
{
	return FilmGrainSettingsRTTI::Instance();
}

RTTITypeBase* FilmGrainSettings::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* ShadowSettings::GetRttiStatic()
{
	return ShadowSettingsRTTI::Instance();
}

RTTITypeBase* ShadowSettings::GetRtti() const
{
	return GetRttiStatic();
}

template struct TRenderSettings<false>;
template struct TRenderSettings<true>;

RTTITypeBase* RenderSettings::GetRttiStatic()
{
	return RenderSettingsRTTI::Instance();
}

RTTITypeBase* RenderSettings::GetRtti() const
{
	return GetRttiStatic();
}
