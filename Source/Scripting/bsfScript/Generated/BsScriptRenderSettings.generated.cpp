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
		SPtr<RenderSettings> nativeObject = B3DMakeShared<RenderSettings>();
		new (B3DAllocate<ScriptRenderSettings>())ScriptRenderSettings(managedInstance, nativeObject);
	}

	MonoObject* ScriptRenderSettings::InternalGetDepthOfField(ScriptRenderSettings* self)
	{
		SPtr<DepthOfFieldSettings> tmp__output = B3DMakeShared<DepthOfFieldSettings>();
		*tmp__output = self->GetInternal()->DepthOfField;

		MonoObject* __output;
		__output = ScriptDepthOfFieldSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetDepthOfField(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<DepthOfFieldSettings> tmpvalue;
		ScriptDepthOfFieldSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptDepthOfFieldSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->DepthOfField = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetChromaticAberration(ScriptRenderSettings* self)
	{
		SPtr<ChromaticAberrationSettings> tmp__output = B3DMakeShared<ChromaticAberrationSettings>();
		*tmp__output = self->GetInternal()->ChromaticAberration;

		MonoObject* __output;
		__output = ScriptChromaticAberrationSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetChromaticAberration(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<ChromaticAberrationSettings> tmpvalue;
		ScriptChromaticAberrationSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptChromaticAberrationSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->ChromaticAberration = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableAutoExposure(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableAutoExposure;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableAutoExposure(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableAutoExposure = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetAutoExposure(ScriptRenderSettings* self)
	{
		SPtr<AutoExposureSettings> tmp__output = B3DMakeShared<AutoExposureSettings>();
		*tmp__output = self->GetInternal()->AutoExposure;

		MonoObject* __output;
		__output = ScriptAutoExposureSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAutoExposure(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<AutoExposureSettings> tmpvalue;
		ScriptAutoExposureSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAutoExposureSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->AutoExposure = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableTonemapping(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableTonemapping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableTonemapping(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableTonemapping = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetTonemapping(ScriptRenderSettings* self)
	{
		SPtr<TonemappingSettings> tmp__output = B3DMakeShared<TonemappingSettings>();
		*tmp__output = self->GetInternal()->Tonemapping;

		MonoObject* __output;
		__output = ScriptTonemappingSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTonemapping(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<TonemappingSettings> tmpvalue;
		ScriptTonemappingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTonemappingSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->Tonemapping = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetWhiteBalance(ScriptRenderSettings* self)
	{
		SPtr<WhiteBalanceSettings> tmp__output = B3DMakeShared<WhiteBalanceSettings>();
		*tmp__output = self->GetInternal()->WhiteBalance;

		MonoObject* __output;
		__output = ScriptWhiteBalanceSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetWhiteBalance(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<WhiteBalanceSettings> tmpvalue;
		ScriptWhiteBalanceSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptWhiteBalanceSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->WhiteBalance = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetColorGrading(ScriptRenderSettings* self)
	{
		SPtr<ColorGradingSettings> tmp__output = B3DMakeShared<ColorGradingSettings>();
		*tmp__output = self->GetInternal()->ColorGrading;

		MonoObject* __output;
		__output = ScriptColorGradingSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetColorGrading(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<ColorGradingSettings> tmpvalue;
		ScriptColorGradingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorGradingSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->ColorGrading = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetAmbientOcclusion(ScriptRenderSettings* self)
	{
		SPtr<AmbientOcclusionSettings> tmp__output = B3DMakeShared<AmbientOcclusionSettings>();
		*tmp__output = self->GetInternal()->AmbientOcclusion;

		MonoObject* __output;
		__output = ScriptAmbientOcclusionSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAmbientOcclusion(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<AmbientOcclusionSettings> tmpvalue;
		ScriptAmbientOcclusionSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAmbientOcclusionSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->AmbientOcclusion = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceReflections(ScriptRenderSettings* self)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmp__output = B3DMakeShared<ScreenSpaceReflectionsSettings>();
		*tmp__output = self->GetInternal()->ScreenSpaceReflections;

		MonoObject* __output;
		__output = ScriptScreenSpaceReflectionsSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceReflections(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmpvalue;
		ScriptScreenSpaceReflectionsSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceReflectionsSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->ScreenSpaceReflections = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetBloom(ScriptRenderSettings* self)
	{
		SPtr<BloomSettings> tmp__output = B3DMakeShared<BloomSettings>();
		*tmp__output = self->GetInternal()->Bloom;

		MonoObject* __output;
		__output = ScriptBloomSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetBloom(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<BloomSettings> tmpvalue;
		ScriptBloomSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptBloomSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->Bloom = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceLensFlare(ScriptRenderSettings* self)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmp__output = B3DMakeShared<ScreenSpaceLensFlareSettings>();
		*tmp__output = self->GetInternal()->ScreenSpaceLensFlare;

		MonoObject* __output;
		__output = ScriptScreenSpaceLensFlareSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceLensFlare(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmpvalue;
		ScriptScreenSpaceLensFlareSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceLensFlareSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->ScreenSpaceLensFlare = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetFilmGrain(ScriptRenderSettings* self)
	{
		SPtr<FilmGrainSettings> tmp__output = B3DMakeShared<FilmGrainSettings>();
		*tmp__output = self->GetInternal()->FilmGrain;

		MonoObject* __output;
		__output = ScriptFilmGrainSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetFilmGrain(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<FilmGrainSettings> tmpvalue;
		ScriptFilmGrainSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFilmGrainSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->FilmGrain = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetMotionBlur(ScriptRenderSettings* self)
	{
		SPtr<MotionBlurSettings> tmp__output = B3DMakeShared<MotionBlurSettings>();
		*tmp__output = self->GetInternal()->MotionBlur;

		MonoObject* __output;
		__output = ScriptMotionBlurSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetMotionBlur(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<MotionBlurSettings> tmpvalue;
		ScriptMotionBlurSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptMotionBlurSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->MotionBlur = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetTemporalAa(ScriptRenderSettings* self)
	{
		SPtr<TemporalAASettings> tmp__output = B3DMakeShared<TemporalAASettings>();
		*tmp__output = self->GetInternal()->TemporalAa;

		MonoObject* __output;
		__output = ScriptTemporalAASettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTemporalAa(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<TemporalAASettings> tmpvalue;
		ScriptTemporalAASettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTemporalAASettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->TemporalAa = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableFxaa(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableFxaa;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableFxaa(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableFxaa = value;
	}

	float ScriptRenderSettings::InternalGetExposureScale(ScriptRenderSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->ExposureScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetExposureScale(ScriptRenderSettings* self, float value)
	{
		self->GetInternal()->ExposureScale = value;
	}

	float ScriptRenderSettings::InternalGetGamma(ScriptRenderSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Gamma;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetGamma(ScriptRenderSettings* self, float value)
	{
		self->GetInternal()->Gamma = value;
	}

	bool ScriptRenderSettings::InternalGetEnableHdr(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableHdr;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableHdr(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableHdr = value;
	}

	bool ScriptRenderSettings::InternalGetEnableLighting(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableLighting(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableLighting = value;
	}

	bool ScriptRenderSettings::InternalGetEnableShadows(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableShadows;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableShadows(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableShadows = value;
	}

	bool ScriptRenderSettings::InternalGetEnableVelocityBuffer(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableVelocityBuffer;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableVelocityBuffer(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableVelocityBuffer = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetShadowSettings(ScriptRenderSettings* self)
	{
		SPtr<ShadowSettings> tmp__output = B3DMakeShared<ShadowSettings>();
		*tmp__output = self->GetInternal()->ShadowSettings;

		MonoObject* __output;
		__output = ScriptShadowSettings::Create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetShadowSettings(ScriptRenderSettings* self, MonoObject* value)
	{
		SPtr<ShadowSettings> tmpvalue;
		ScriptShadowSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptShadowSettings::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->ShadowSettings = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableIndirectLighting(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableIndirectLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableIndirectLighting(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableIndirectLighting = value;
	}

	bool ScriptRenderSettings::InternalGetOverlayOnly(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->OverlayOnly;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetOverlayOnly(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->OverlayOnly = value;
	}

	bool ScriptRenderSettings::InternalGetEnableSkybox(ScriptRenderSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->EnableSkybox;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableSkybox(ScriptRenderSettings* self, bool value)
	{
		self->GetInternal()->EnableSkybox = value;
	}

	float ScriptRenderSettings::InternalGetCullDistance(ScriptRenderSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->CullDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetCullDistance(ScriptRenderSettings* self, float value)
	{
		self->GetInternal()->CullDistance = value;
	}
}
