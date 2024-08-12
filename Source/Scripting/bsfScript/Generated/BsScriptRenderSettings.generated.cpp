//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRenderSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptDepthOfFieldSettings.generated.h"
#include "BsScriptWhiteBalanceSettings.generated.h"
#include "BsScriptChromaticAberrationSettings.generated.h"
#include "BsScriptAutoExposureSettings.generated.h"
#include "BsScriptTonemappingSettings.generated.h"
#include "BsScriptScreenSpaceReflectionsSettings.generated.h"
#include "BsScriptColorGradingSettings.generated.h"
#include "BsScriptAmbientOcclusionSettings.generated.h"
#include "BsScriptBloomSettings.generated.h"
#include "BsScriptScreenSpaceLensFlareSettings.generated.h"
#include "BsScriptFilmGrainSettings.generated.h"
#include "BsScriptMotionBlurSettings.generated.h"
#include "BsScriptTemporalAASettings.generated.h"
#include "BsScriptShadowSettings.generated.h"

namespace bs
{
	ScriptRenderSettings::ScriptRenderSettings(MonoObject* managedInstance, const SPtr<RenderSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptRenderSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_RenderSettings", (void*)&ScriptRenderSettings::InternalRenderSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetDepthOfField", (void*)&ScriptRenderSettings::InternalGetDepthOfField);
		metaData.ScriptClass->AddInternalCall("Internal_SetDepthOfField", (void*)&ScriptRenderSettings::InternalSetDepthOfField);
		metaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberration", (void*)&ScriptRenderSettings::InternalGetChromaticAberration);
		metaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberration", (void*)&ScriptRenderSettings::InternalSetChromaticAberration);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableAutoExposure", (void*)&ScriptRenderSettings::InternalGetEnableAutoExposure);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableAutoExposure", (void*)&ScriptRenderSettings::InternalSetEnableAutoExposure);
		metaData.ScriptClass->AddInternalCall("Internal_GetAutoExposure", (void*)&ScriptRenderSettings::InternalGetAutoExposure);
		metaData.ScriptClass->AddInternalCall("Internal_SetAutoExposure", (void*)&ScriptRenderSettings::InternalSetAutoExposure);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableTonemapping", (void*)&ScriptRenderSettings::InternalGetEnableTonemapping);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableTonemapping", (void*)&ScriptRenderSettings::InternalSetEnableTonemapping);
		metaData.ScriptClass->AddInternalCall("Internal_GetTonemapping", (void*)&ScriptRenderSettings::InternalGetTonemapping);
		metaData.ScriptClass->AddInternalCall("Internal_SetTonemapping", (void*)&ScriptRenderSettings::InternalSetTonemapping);
		metaData.ScriptClass->AddInternalCall("Internal_GetWhiteBalance", (void*)&ScriptRenderSettings::InternalGetWhiteBalance);
		metaData.ScriptClass->AddInternalCall("Internal_SetWhiteBalance", (void*)&ScriptRenderSettings::InternalSetWhiteBalance);
		metaData.ScriptClass->AddInternalCall("Internal_GetColorGrading", (void*)&ScriptRenderSettings::InternalGetColorGrading);
		metaData.ScriptClass->AddInternalCall("Internal_SetColorGrading", (void*)&ScriptRenderSettings::InternalSetColorGrading);
		metaData.ScriptClass->AddInternalCall("Internal_GetAmbientOcclusion", (void*)&ScriptRenderSettings::InternalGetAmbientOcclusion);
		metaData.ScriptClass->AddInternalCall("Internal_SetAmbientOcclusion", (void*)&ScriptRenderSettings::InternalSetAmbientOcclusion);
		metaData.ScriptClass->AddInternalCall("Internal_GetScreenSpaceReflections", (void*)&ScriptRenderSettings::InternalGetScreenSpaceReflections);
		metaData.ScriptClass->AddInternalCall("Internal_SetScreenSpaceReflections", (void*)&ScriptRenderSettings::InternalSetScreenSpaceReflections);
		metaData.ScriptClass->AddInternalCall("Internal_GetBloom", (void*)&ScriptRenderSettings::InternalGetBloom);
		metaData.ScriptClass->AddInternalCall("Internal_SetBloom", (void*)&ScriptRenderSettings::InternalSetBloom);
		metaData.ScriptClass->AddInternalCall("Internal_GetScreenSpaceLensFlare", (void*)&ScriptRenderSettings::InternalGetScreenSpaceLensFlare);
		metaData.ScriptClass->AddInternalCall("Internal_SetScreenSpaceLensFlare", (void*)&ScriptRenderSettings::InternalSetScreenSpaceLensFlare);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmGrain", (void*)&ScriptRenderSettings::InternalGetFilmGrain);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmGrain", (void*)&ScriptRenderSettings::InternalSetFilmGrain);
		metaData.ScriptClass->AddInternalCall("Internal_GetMotionBlur", (void*)&ScriptRenderSettings::InternalGetMotionBlur);
		metaData.ScriptClass->AddInternalCall("Internal_SetMotionBlur", (void*)&ScriptRenderSettings::InternalSetMotionBlur);
		metaData.ScriptClass->AddInternalCall("Internal_GetTemporalAa", (void*)&ScriptRenderSettings::InternalGetTemporalAa);
		metaData.ScriptClass->AddInternalCall("Internal_SetTemporalAa", (void*)&ScriptRenderSettings::InternalSetTemporalAa);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableFxaa", (void*)&ScriptRenderSettings::InternalGetEnableFxaa);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableFxaa", (void*)&ScriptRenderSettings::InternalSetEnableFxaa);
		metaData.ScriptClass->AddInternalCall("Internal_GetExposureScale", (void*)&ScriptRenderSettings::InternalGetExposureScale);
		metaData.ScriptClass->AddInternalCall("Internal_SetExposureScale", (void*)&ScriptRenderSettings::InternalSetExposureScale);
		metaData.ScriptClass->AddInternalCall("Internal_GetGamma", (void*)&ScriptRenderSettings::InternalGetGamma);
		metaData.ScriptClass->AddInternalCall("Internal_SetGamma", (void*)&ScriptRenderSettings::InternalSetGamma);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableHdr", (void*)&ScriptRenderSettings::InternalGetEnableHdr);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableHdr", (void*)&ScriptRenderSettings::InternalSetEnableHdr);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableLighting", (void*)&ScriptRenderSettings::InternalGetEnableLighting);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableLighting", (void*)&ScriptRenderSettings::InternalSetEnableLighting);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableShadows", (void*)&ScriptRenderSettings::InternalGetEnableShadows);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableShadows", (void*)&ScriptRenderSettings::InternalSetEnableShadows);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableVelocityBuffer", (void*)&ScriptRenderSettings::InternalGetEnableVelocityBuffer);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableVelocityBuffer", (void*)&ScriptRenderSettings::InternalSetEnableVelocityBuffer);
		metaData.ScriptClass->AddInternalCall("Internal_GetShadowSettings", (void*)&ScriptRenderSettings::InternalGetShadowSettings);
		metaData.ScriptClass->AddInternalCall("Internal_SetShadowSettings", (void*)&ScriptRenderSettings::InternalSetShadowSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableIndirectLighting", (void*)&ScriptRenderSettings::InternalGetEnableIndirectLighting);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableIndirectLighting", (void*)&ScriptRenderSettings::InternalSetEnableIndirectLighting);
		metaData.ScriptClass->AddInternalCall("Internal_GetOverlayOnly", (void*)&ScriptRenderSettings::InternalGetOverlayOnly);
		metaData.ScriptClass->AddInternalCall("Internal_SetOverlayOnly", (void*)&ScriptRenderSettings::InternalSetOverlayOnly);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableSkybox", (void*)&ScriptRenderSettings::InternalGetEnableSkybox);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableSkybox", (void*)&ScriptRenderSettings::InternalSetEnableSkybox);
		metaData.ScriptClass->AddInternalCall("Internal_GetCullDistance", (void*)&ScriptRenderSettings::InternalGetCullDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetCullDistance", (void*)&ScriptRenderSettings::InternalSetCullDistance);

	}

	MonoObject* ScriptRenderSettings::Create(const SPtr<RenderSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptRenderSettings>()) ScriptRenderSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptRenderSettings::InternalRenderSettings(MonoObject* managedInstance)
	{
		SPtr<RenderSettings> instance = B3DMakeShared<RenderSettings>();
		new (B3DAllocate<ScriptRenderSettings>())ScriptRenderSettings(managedInstance, instance);
	}

	MonoObject* ScriptRenderSettings::InternalGetDepthOfField(ScriptRenderSettings* thisPtr)
	{
		SPtr<DepthOfFieldSettings> tmp__output = B3DMakeShared<DepthOfFieldSettings>();
		*tmp__output = thisPtr->GetInternal()->DepthOfField;

		MonoObject* __output;
		__output = ScriptDepthOfFieldSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetDepthOfField(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<DepthOfFieldSettings> tmpvalue;
		ScriptDepthOfFieldSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptDepthOfFieldSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->DepthOfField = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetChromaticAberration(ScriptRenderSettings* thisPtr)
	{
		SPtr<ChromaticAberrationSettings> tmp__output = B3DMakeShared<ChromaticAberrationSettings>();
		*tmp__output = thisPtr->GetInternal()->ChromaticAberration;

		MonoObject* __output;
		__output = ScriptChromaticAberrationSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetChromaticAberration(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ChromaticAberrationSettings> tmpvalue;
		ScriptChromaticAberrationSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptChromaticAberrationSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->ChromaticAberration = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableAutoExposure(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableAutoExposure;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableAutoExposure(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableAutoExposure = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetAutoExposure(ScriptRenderSettings* thisPtr)
	{
		SPtr<AutoExposureSettings> tmp__output = B3DMakeShared<AutoExposureSettings>();
		*tmp__output = thisPtr->GetInternal()->AutoExposure;

		MonoObject* __output;
		__output = ScriptAutoExposureSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAutoExposure(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<AutoExposureSettings> tmpvalue;
		ScriptAutoExposureSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAutoExposureSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->AutoExposure = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableTonemapping(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableTonemapping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableTonemapping(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableTonemapping = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetTonemapping(ScriptRenderSettings* thisPtr)
	{
		SPtr<TonemappingSettings> tmp__output = B3DMakeShared<TonemappingSettings>();
		*tmp__output = thisPtr->GetInternal()->Tonemapping;

		MonoObject* __output;
		__output = ScriptTonemappingSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTonemapping(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<TonemappingSettings> tmpvalue;
		ScriptTonemappingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTonemappingSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->Tonemapping = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetWhiteBalance(ScriptRenderSettings* thisPtr)
	{
		SPtr<WhiteBalanceSettings> tmp__output = B3DMakeShared<WhiteBalanceSettings>();
		*tmp__output = thisPtr->GetInternal()->WhiteBalance;

		MonoObject* __output;
		__output = ScriptWhiteBalanceSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetWhiteBalance(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<WhiteBalanceSettings> tmpvalue;
		ScriptWhiteBalanceSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptWhiteBalanceSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->WhiteBalance = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetColorGrading(ScriptRenderSettings* thisPtr)
	{
		SPtr<ColorGradingSettings> tmp__output = B3DMakeShared<ColorGradingSettings>();
		*tmp__output = thisPtr->GetInternal()->ColorGrading;

		MonoObject* __output;
		__output = ScriptColorGradingSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetColorGrading(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ColorGradingSettings> tmpvalue;
		ScriptColorGradingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorGradingSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->ColorGrading = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetAmbientOcclusion(ScriptRenderSettings* thisPtr)
	{
		SPtr<AmbientOcclusionSettings> tmp__output = B3DMakeShared<AmbientOcclusionSettings>();
		*tmp__output = thisPtr->GetInternal()->AmbientOcclusion;

		MonoObject* __output;
		__output = ScriptAmbientOcclusionSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAmbientOcclusion(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<AmbientOcclusionSettings> tmpvalue;
		ScriptAmbientOcclusionSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAmbientOcclusionSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->AmbientOcclusion = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceReflections(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmp__output = B3DMakeShared<ScreenSpaceReflectionsSettings>();
		*tmp__output = thisPtr->GetInternal()->ScreenSpaceReflections;

		MonoObject* __output;
		__output = ScriptScreenSpaceReflectionsSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceReflections(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmpvalue;
		ScriptScreenSpaceReflectionsSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceReflectionsSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->ScreenSpaceReflections = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetBloom(ScriptRenderSettings* thisPtr)
	{
		SPtr<BloomSettings> tmp__output = B3DMakeShared<BloomSettings>();
		*tmp__output = thisPtr->GetInternal()->Bloom;

		MonoObject* __output;
		__output = ScriptBloomSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetBloom(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<BloomSettings> tmpvalue;
		ScriptBloomSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptBloomSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->Bloom = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceLensFlare(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmp__output = B3DMakeShared<ScreenSpaceLensFlareSettings>();
		*tmp__output = thisPtr->GetInternal()->ScreenSpaceLensFlare;

		MonoObject* __output;
		__output = ScriptScreenSpaceLensFlareSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceLensFlare(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmpvalue;
		ScriptScreenSpaceLensFlareSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceLensFlareSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->ScreenSpaceLensFlare = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetFilmGrain(ScriptRenderSettings* thisPtr)
	{
		SPtr<FilmGrainSettings> tmp__output = B3DMakeShared<FilmGrainSettings>();
		*tmp__output = thisPtr->GetInternal()->FilmGrain;

		MonoObject* __output;
		__output = ScriptFilmGrainSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetFilmGrain(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<FilmGrainSettings> tmpvalue;
		ScriptFilmGrainSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFilmGrainSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->FilmGrain = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetMotionBlur(ScriptRenderSettings* thisPtr)
	{
		SPtr<MotionBlurSettings> tmp__output = B3DMakeShared<MotionBlurSettings>();
		*tmp__output = thisPtr->GetInternal()->MotionBlur;

		MonoObject* __output;
		__output = ScriptMotionBlurSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetMotionBlur(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<MotionBlurSettings> tmpvalue;
		ScriptMotionBlurSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptMotionBlurSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->MotionBlur = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetTemporalAa(ScriptRenderSettings* thisPtr)
	{
		SPtr<TemporalAASettings> tmp__output = B3DMakeShared<TemporalAASettings>();
		*tmp__output = thisPtr->GetInternal()->TemporalAa;

		MonoObject* __output;
		__output = ScriptTemporalAASettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTemporalAa(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<TemporalAASettings> tmpvalue;
		ScriptTemporalAASettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTemporalAASettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->TemporalAa = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableFxaa(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableFxaa;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableFxaa(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableFxaa = value;
	}

	float ScriptRenderSettings::InternalGetExposureScale(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->ExposureScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetExposureScale(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->ExposureScale = value;
	}

	float ScriptRenderSettings::InternalGetGamma(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Gamma;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetGamma(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Gamma = value;
	}

	bool ScriptRenderSettings::InternalGetEnableHdr(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableHdr;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableHdr(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableHdr = value;
	}

	bool ScriptRenderSettings::InternalGetEnableLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableLighting = value;
	}

	bool ScriptRenderSettings::InternalGetEnableShadows(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableShadows;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableShadows(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableShadows = value;
	}

	bool ScriptRenderSettings::InternalGetEnableVelocityBuffer(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableVelocityBuffer;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableVelocityBuffer(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableVelocityBuffer = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetShadowSettings(ScriptRenderSettings* thisPtr)
	{
		SPtr<ShadowSettings> tmp__output = B3DMakeShared<ShadowSettings>();
		*tmp__output = thisPtr->GetInternal()->ShadowSettings;

		MonoObject* __output;
		__output = ScriptShadowSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetShadowSettings(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ShadowSettings> tmpvalue;
		ScriptShadowSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptShadowSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->ShadowSettings = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableIndirectLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableIndirectLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableIndirectLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableIndirectLighting = value;
	}

	bool ScriptRenderSettings::InternalGetOverlayOnly(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->OverlayOnly;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetOverlayOnly(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->OverlayOnly = value;
	}

	bool ScriptRenderSettings::InternalGetEnableSkybox(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->EnableSkybox;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableSkybox(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->EnableSkybox = value;
	}

	float ScriptRenderSettings::InternalGetCullDistance(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->CullDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetCullDistance(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->CullDistance = value;
	}
}
