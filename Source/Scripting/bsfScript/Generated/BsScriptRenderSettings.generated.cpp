//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
#include "BsScriptBloomSettings.generated.h"
#include "BsScriptAmbientOcclusionSettings.generated.h"
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
		metaData.scriptClass->AddInternalCall("Internal_RenderSettings", (void*)&ScriptRenderSettings::Internal_RenderSettings);
		metaData.scriptClass->AddInternalCall("Internal_getdepthOfField", (void*)&ScriptRenderSettings::Internal_getdepthOfField);
		metaData.scriptClass->AddInternalCall("Internal_setdepthOfField", (void*)&ScriptRenderSettings::Internal_setdepthOfField);
		metaData.scriptClass->AddInternalCall("Internal_getchromaticAberration", (void*)&ScriptRenderSettings::Internal_getchromaticAberration);
		metaData.scriptClass->AddInternalCall("Internal_setchromaticAberration", (void*)&ScriptRenderSettings::Internal_setchromaticAberration);
		metaData.scriptClass->AddInternalCall("Internal_getenableAutoExposure", (void*)&ScriptRenderSettings::Internal_getenableAutoExposure);
		metaData.scriptClass->AddInternalCall("Internal_setenableAutoExposure", (void*)&ScriptRenderSettings::Internal_setenableAutoExposure);
		metaData.scriptClass->AddInternalCall("Internal_getautoExposure", (void*)&ScriptRenderSettings::Internal_getautoExposure);
		metaData.scriptClass->AddInternalCall("Internal_setautoExposure", (void*)&ScriptRenderSettings::Internal_setautoExposure);
		metaData.scriptClass->AddInternalCall("Internal_getenableTonemapping", (void*)&ScriptRenderSettings::Internal_getenableTonemapping);
		metaData.scriptClass->AddInternalCall("Internal_setenableTonemapping", (void*)&ScriptRenderSettings::Internal_setenableTonemapping);
		metaData.scriptClass->AddInternalCall("Internal_gettonemapping", (void*)&ScriptRenderSettings::Internal_gettonemapping);
		metaData.scriptClass->AddInternalCall("Internal_settonemapping", (void*)&ScriptRenderSettings::Internal_settonemapping);
		metaData.scriptClass->AddInternalCall("Internal_getwhiteBalance", (void*)&ScriptRenderSettings::Internal_getwhiteBalance);
		metaData.scriptClass->AddInternalCall("Internal_setwhiteBalance", (void*)&ScriptRenderSettings::Internal_setwhiteBalance);
		metaData.scriptClass->AddInternalCall("Internal_getcolorGrading", (void*)&ScriptRenderSettings::Internal_getcolorGrading);
		metaData.scriptClass->AddInternalCall("Internal_setcolorGrading", (void*)&ScriptRenderSettings::Internal_setcolorGrading);
		metaData.scriptClass->AddInternalCall("Internal_getambientOcclusion", (void*)&ScriptRenderSettings::Internal_getambientOcclusion);
		metaData.scriptClass->AddInternalCall("Internal_setambientOcclusion", (void*)&ScriptRenderSettings::Internal_setambientOcclusion);
		metaData.scriptClass->AddInternalCall("Internal_getscreenSpaceReflections", (void*)&ScriptRenderSettings::Internal_getscreenSpaceReflections);
		metaData.scriptClass->AddInternalCall("Internal_setscreenSpaceReflections", (void*)&ScriptRenderSettings::Internal_setscreenSpaceReflections);
		metaData.scriptClass->AddInternalCall("Internal_getbloom", (void*)&ScriptRenderSettings::Internal_getbloom);
		metaData.scriptClass->AddInternalCall("Internal_setbloom", (void*)&ScriptRenderSettings::Internal_setbloom);
		metaData.scriptClass->AddInternalCall("Internal_getscreenSpaceLensFlare", (void*)&ScriptRenderSettings::Internal_getscreenSpaceLensFlare);
		metaData.scriptClass->AddInternalCall("Internal_setscreenSpaceLensFlare", (void*)&ScriptRenderSettings::Internal_setscreenSpaceLensFlare);
		metaData.scriptClass->AddInternalCall("Internal_getfilmGrain", (void*)&ScriptRenderSettings::Internal_getfilmGrain);
		metaData.scriptClass->AddInternalCall("Internal_setfilmGrain", (void*)&ScriptRenderSettings::Internal_setfilmGrain);
		metaData.scriptClass->AddInternalCall("Internal_getmotionBlur", (void*)&ScriptRenderSettings::Internal_getmotionBlur);
		metaData.scriptClass->AddInternalCall("Internal_setmotionBlur", (void*)&ScriptRenderSettings::Internal_setmotionBlur);
		metaData.scriptClass->AddInternalCall("Internal_gettemporalAA", (void*)&ScriptRenderSettings::Internal_gettemporalAA);
		metaData.scriptClass->AddInternalCall("Internal_settemporalAA", (void*)&ScriptRenderSettings::Internal_settemporalAA);
		metaData.scriptClass->AddInternalCall("Internal_getenableFXAA", (void*)&ScriptRenderSettings::Internal_getenableFXAA);
		metaData.scriptClass->AddInternalCall("Internal_setenableFXAA", (void*)&ScriptRenderSettings::Internal_setenableFXAA);
		metaData.scriptClass->AddInternalCall("Internal_getexposureScale", (void*)&ScriptRenderSettings::Internal_getexposureScale);
		metaData.scriptClass->AddInternalCall("Internal_setexposureScale", (void*)&ScriptRenderSettings::Internal_setexposureScale);
		metaData.scriptClass->AddInternalCall("Internal_getgamma", (void*)&ScriptRenderSettings::Internal_getgamma);
		metaData.scriptClass->AddInternalCall("Internal_setgamma", (void*)&ScriptRenderSettings::Internal_setgamma);
		metaData.scriptClass->AddInternalCall("Internal_getenableHDR", (void*)&ScriptRenderSettings::Internal_getenableHDR);
		metaData.scriptClass->AddInternalCall("Internal_setenableHDR", (void*)&ScriptRenderSettings::Internal_setenableHDR);
		metaData.scriptClass->AddInternalCall("Internal_getenableLighting", (void*)&ScriptRenderSettings::Internal_getenableLighting);
		metaData.scriptClass->AddInternalCall("Internal_setenableLighting", (void*)&ScriptRenderSettings::Internal_setenableLighting);
		metaData.scriptClass->AddInternalCall("Internal_getenableShadows", (void*)&ScriptRenderSettings::Internal_getenableShadows);
		metaData.scriptClass->AddInternalCall("Internal_setenableShadows", (void*)&ScriptRenderSettings::Internal_setenableShadows);
		metaData.scriptClass->AddInternalCall("Internal_getenableVelocityBuffer", (void*)&ScriptRenderSettings::Internal_getenableVelocityBuffer);
		metaData.scriptClass->AddInternalCall("Internal_setenableVelocityBuffer", (void*)&ScriptRenderSettings::Internal_setenableVelocityBuffer);
		metaData.scriptClass->AddInternalCall("Internal_getshadowSettings", (void*)&ScriptRenderSettings::Internal_getshadowSettings);
		metaData.scriptClass->AddInternalCall("Internal_setshadowSettings", (void*)&ScriptRenderSettings::Internal_setshadowSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenableIndirectLighting", (void*)&ScriptRenderSettings::Internal_getenableIndirectLighting);
		metaData.scriptClass->AddInternalCall("Internal_setenableIndirectLighting", (void*)&ScriptRenderSettings::Internal_setenableIndirectLighting);
		metaData.scriptClass->AddInternalCall("Internal_getoverlayOnly", (void*)&ScriptRenderSettings::Internal_getoverlayOnly);
		metaData.scriptClass->AddInternalCall("Internal_setoverlayOnly", (void*)&ScriptRenderSettings::Internal_setoverlayOnly);
		metaData.scriptClass->AddInternalCall("Internal_getenableSkybox", (void*)&ScriptRenderSettings::Internal_getenableSkybox);
		metaData.scriptClass->AddInternalCall("Internal_setenableSkybox", (void*)&ScriptRenderSettings::Internal_setenableSkybox);
		metaData.scriptClass->AddInternalCall("Internal_getcullDistance", (void*)&ScriptRenderSettings::Internal_getcullDistance);
		metaData.scriptClass->AddInternalCall("Internal_setcullDistance", (void*)&ScriptRenderSettings::Internal_setcullDistance);

	}

	MonoObject* ScriptRenderSettings::create(const SPtr<RenderSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptRenderSettings>()) ScriptRenderSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptRenderSettings::Internal_RenderSettings(MonoObject* managedInstance)
	{
		SPtr<RenderSettings> instance = bs_shared_ptr_new<RenderSettings>();
		new (bs_alloc<ScriptRenderSettings>())ScriptRenderSettings(managedInstance, instance);
	}

	MonoObject* ScriptRenderSettings::Internal_getdepthOfField(ScriptRenderSettings* thisPtr)
	{
		SPtr<DepthOfFieldSettings> tmp__output = bs_shared_ptr_new<DepthOfFieldSettings>();
		*tmp__output = thisPtr->GetInternal()->depthOfField;

		MonoObject* __output;
		__output = ScriptDepthOfFieldSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setdepthOfField(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<DepthOfFieldSettings> tmpvalue;
		ScriptDepthOfFieldSettings* scriptvalue;
		scriptvalue = ScriptDepthOfFieldSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->depthOfField = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getchromaticAberration(ScriptRenderSettings* thisPtr)
	{
		SPtr<ChromaticAberrationSettings> tmp__output = bs_shared_ptr_new<ChromaticAberrationSettings>();
		*tmp__output = thisPtr->GetInternal()->chromaticAberration;

		MonoObject* __output;
		__output = ScriptChromaticAberrationSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setchromaticAberration(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ChromaticAberrationSettings> tmpvalue;
		ScriptChromaticAberrationSettings* scriptvalue;
		scriptvalue = ScriptChromaticAberrationSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->chromaticAberration = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableAutoExposure(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableAutoExposure;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableAutoExposure(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableAutoExposure = value;
	}

	MonoObject* ScriptRenderSettings::Internal_getautoExposure(ScriptRenderSettings* thisPtr)
	{
		SPtr<AutoExposureSettings> tmp__output = bs_shared_ptr_new<AutoExposureSettings>();
		*tmp__output = thisPtr->GetInternal()->autoExposure;

		MonoObject* __output;
		__output = ScriptAutoExposureSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setautoExposure(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<AutoExposureSettings> tmpvalue;
		ScriptAutoExposureSettings* scriptvalue;
		scriptvalue = ScriptAutoExposureSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->autoExposure = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableTonemapping(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableTonemapping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableTonemapping(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableTonemapping = value;
	}

	MonoObject* ScriptRenderSettings::Internal_gettonemapping(ScriptRenderSettings* thisPtr)
	{
		SPtr<TonemappingSettings> tmp__output = bs_shared_ptr_new<TonemappingSettings>();
		*tmp__output = thisPtr->GetInternal()->tonemapping;

		MonoObject* __output;
		__output = ScriptTonemappingSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_settonemapping(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<TonemappingSettings> tmpvalue;
		ScriptTonemappingSettings* scriptvalue;
		scriptvalue = ScriptTonemappingSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->tonemapping = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getwhiteBalance(ScriptRenderSettings* thisPtr)
	{
		SPtr<WhiteBalanceSettings> tmp__output = bs_shared_ptr_new<WhiteBalanceSettings>();
		*tmp__output = thisPtr->GetInternal()->whiteBalance;

		MonoObject* __output;
		__output = ScriptWhiteBalanceSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setwhiteBalance(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<WhiteBalanceSettings> tmpvalue;
		ScriptWhiteBalanceSettings* scriptvalue;
		scriptvalue = ScriptWhiteBalanceSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->whiteBalance = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getcolorGrading(ScriptRenderSettings* thisPtr)
	{
		SPtr<ColorGradingSettings> tmp__output = bs_shared_ptr_new<ColorGradingSettings>();
		*tmp__output = thisPtr->GetInternal()->colorGrading;

		MonoObject* __output;
		__output = ScriptColorGradingSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setcolorGrading(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ColorGradingSettings> tmpvalue;
		ScriptColorGradingSettings* scriptvalue;
		scriptvalue = ScriptColorGradingSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->colorGrading = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getambientOcclusion(ScriptRenderSettings* thisPtr)
	{
		SPtr<AmbientOcclusionSettings> tmp__output = bs_shared_ptr_new<AmbientOcclusionSettings>();
		*tmp__output = thisPtr->GetInternal()->ambientOcclusion;

		MonoObject* __output;
		__output = ScriptAmbientOcclusionSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setambientOcclusion(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<AmbientOcclusionSettings> tmpvalue;
		ScriptAmbientOcclusionSettings* scriptvalue;
		scriptvalue = ScriptAmbientOcclusionSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->ambientOcclusion = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getscreenSpaceReflections(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmp__output = bs_shared_ptr_new<ScreenSpaceReflectionsSettings>();
		*tmp__output = thisPtr->GetInternal()->screenSpaceReflections;

		MonoObject* __output;
		__output = ScriptScreenSpaceReflectionsSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setscreenSpaceReflections(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmpvalue;
		ScriptScreenSpaceReflectionsSettings* scriptvalue;
		scriptvalue = ScriptScreenSpaceReflectionsSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->screenSpaceReflections = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getbloom(ScriptRenderSettings* thisPtr)
	{
		SPtr<BloomSettings> tmp__output = bs_shared_ptr_new<BloomSettings>();
		*tmp__output = thisPtr->GetInternal()->bloom;

		MonoObject* __output;
		__output = ScriptBloomSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setbloom(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<BloomSettings> tmpvalue;
		ScriptBloomSettings* scriptvalue;
		scriptvalue = ScriptBloomSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->bloom = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getscreenSpaceLensFlare(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmp__output = bs_shared_ptr_new<ScreenSpaceLensFlareSettings>();
		*tmp__output = thisPtr->GetInternal()->screenSpaceLensFlare;

		MonoObject* __output;
		__output = ScriptScreenSpaceLensFlareSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setscreenSpaceLensFlare(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmpvalue;
		ScriptScreenSpaceLensFlareSettings* scriptvalue;
		scriptvalue = ScriptScreenSpaceLensFlareSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->screenSpaceLensFlare = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getfilmGrain(ScriptRenderSettings* thisPtr)
	{
		SPtr<FilmGrainSettings> tmp__output = bs_shared_ptr_new<FilmGrainSettings>();
		*tmp__output = thisPtr->GetInternal()->filmGrain;

		MonoObject* __output;
		__output = ScriptFilmGrainSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setfilmGrain(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<FilmGrainSettings> tmpvalue;
		ScriptFilmGrainSettings* scriptvalue;
		scriptvalue = ScriptFilmGrainSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->filmGrain = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getmotionBlur(ScriptRenderSettings* thisPtr)
	{
		SPtr<MotionBlurSettings> tmp__output = bs_shared_ptr_new<MotionBlurSettings>();
		*tmp__output = thisPtr->GetInternal()->motionBlur;

		MonoObject* __output;
		__output = ScriptMotionBlurSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setmotionBlur(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<MotionBlurSettings> tmpvalue;
		ScriptMotionBlurSettings* scriptvalue;
		scriptvalue = ScriptMotionBlurSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->motionBlur = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_gettemporalAA(ScriptRenderSettings* thisPtr)
	{
		SPtr<TemporalAASettings> tmp__output = bs_shared_ptr_new<TemporalAASettings>();
		*tmp__output = thisPtr->GetInternal()->temporalAA;

		MonoObject* __output;
		__output = ScriptTemporalAASettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_settemporalAA(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<TemporalAASettings> tmpvalue;
		ScriptTemporalAASettings* scriptvalue;
		scriptvalue = ScriptTemporalAASettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->temporalAA = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableFXAA(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableFXAA;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableFXAA(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableFXAA = value;
	}

	float ScriptRenderSettings::Internal_getexposureScale(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->exposureScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setexposureScale(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->exposureScale = value;
	}

	float ScriptRenderSettings::Internal_getgamma(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->gamma;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setgamma(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->gamma = value;
	}

	bool ScriptRenderSettings::Internal_getenableHDR(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableHDR;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableHDR(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableHDR = value;
	}

	bool ScriptRenderSettings::Internal_getenableLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableLighting = value;
	}

	bool ScriptRenderSettings::Internal_getenableShadows(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableShadows;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableShadows(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableShadows = value;
	}

	bool ScriptRenderSettings::Internal_getenableVelocityBuffer(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableVelocityBuffer;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableVelocityBuffer(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableVelocityBuffer = value;
	}

	MonoObject* ScriptRenderSettings::Internal_getshadowSettings(ScriptRenderSettings* thisPtr)
	{
		SPtr<ShadowSettings> tmp__output = bs_shared_ptr_new<ShadowSettings>();
		*tmp__output = thisPtr->GetInternal()->shadowSettings;

		MonoObject* __output;
		__output = ScriptShadowSettings::create(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::Internal_setshadowSettings(ScriptRenderSettings* thisPtr, MonoObject* value)
	{
		SPtr<ShadowSettings> tmpvalue;
		ScriptShadowSettings* scriptvalue;
		scriptvalue = ScriptShadowSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->shadowSettings = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableIndirectLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableIndirectLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableIndirectLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableIndirectLighting = value;
	}

	bool ScriptRenderSettings::Internal_getoverlayOnly(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->overlayOnly;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setoverlayOnly(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->overlayOnly = value;
	}

	bool ScriptRenderSettings::Internal_getenableSkybox(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enableSkybox;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableSkybox(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enableSkybox = value;
	}

	float ScriptRenderSettings::Internal_getcullDistance(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->cullDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setcullDistance(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->cullDistance = value;
	}
}
