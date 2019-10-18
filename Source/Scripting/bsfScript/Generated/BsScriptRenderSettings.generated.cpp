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

	void ScriptRenderSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_RenderSettings", (void*)&ScriptRenderSettings::Internal_RenderSettings);
		metaData.scriptClass->addInternalCall("Internal_getdepthOfField", (void*)&ScriptRenderSettings::Internal_getdepthOfField);
		metaData.scriptClass->addInternalCall("Internal_setdepthOfField", (void*)&ScriptRenderSettings::Internal_setdepthOfField);
		metaData.scriptClass->addInternalCall("Internal_getchromaticAberration", (void*)&ScriptRenderSettings::Internal_getchromaticAberration);
		metaData.scriptClass->addInternalCall("Internal_setchromaticAberration", (void*)&ScriptRenderSettings::Internal_setchromaticAberration);
		metaData.scriptClass->addInternalCall("Internal_getenableAutoExposure", (void*)&ScriptRenderSettings::Internal_getenableAutoExposure);
		metaData.scriptClass->addInternalCall("Internal_setenableAutoExposure", (void*)&ScriptRenderSettings::Internal_setenableAutoExposure);
		metaData.scriptClass->addInternalCall("Internal_getautoExposure", (void*)&ScriptRenderSettings::Internal_getautoExposure);
		metaData.scriptClass->addInternalCall("Internal_setautoExposure", (void*)&ScriptRenderSettings::Internal_setautoExposure);
		metaData.scriptClass->addInternalCall("Internal_getenableTonemapping", (void*)&ScriptRenderSettings::Internal_getenableTonemapping);
		metaData.scriptClass->addInternalCall("Internal_setenableTonemapping", (void*)&ScriptRenderSettings::Internal_setenableTonemapping);
		metaData.scriptClass->addInternalCall("Internal_gettonemapping", (void*)&ScriptRenderSettings::Internal_gettonemapping);
		metaData.scriptClass->addInternalCall("Internal_settonemapping", (void*)&ScriptRenderSettings::Internal_settonemapping);
		metaData.scriptClass->addInternalCall("Internal_getwhiteBalance", (void*)&ScriptRenderSettings::Internal_getwhiteBalance);
		metaData.scriptClass->addInternalCall("Internal_setwhiteBalance", (void*)&ScriptRenderSettings::Internal_setwhiteBalance);
		metaData.scriptClass->addInternalCall("Internal_getcolorGrading", (void*)&ScriptRenderSettings::Internal_getcolorGrading);
		metaData.scriptClass->addInternalCall("Internal_setcolorGrading", (void*)&ScriptRenderSettings::Internal_setcolorGrading);
		metaData.scriptClass->addInternalCall("Internal_getambientOcclusion", (void*)&ScriptRenderSettings::Internal_getambientOcclusion);
		metaData.scriptClass->addInternalCall("Internal_setambientOcclusion", (void*)&ScriptRenderSettings::Internal_setambientOcclusion);
		metaData.scriptClass->addInternalCall("Internal_getscreenSpaceReflections", (void*)&ScriptRenderSettings::Internal_getscreenSpaceReflections);
		metaData.scriptClass->addInternalCall("Internal_setscreenSpaceReflections", (void*)&ScriptRenderSettings::Internal_setscreenSpaceReflections);
		metaData.scriptClass->addInternalCall("Internal_getbloom", (void*)&ScriptRenderSettings::Internal_getbloom);
		metaData.scriptClass->addInternalCall("Internal_setbloom", (void*)&ScriptRenderSettings::Internal_setbloom);
		metaData.scriptClass->addInternalCall("Internal_getscreenSpaceLensFlare", (void*)&ScriptRenderSettings::Internal_getscreenSpaceLensFlare);
		metaData.scriptClass->addInternalCall("Internal_setscreenSpaceLensFlare", (void*)&ScriptRenderSettings::Internal_setscreenSpaceLensFlare);
		metaData.scriptClass->addInternalCall("Internal_getfilmGrain", (void*)&ScriptRenderSettings::Internal_getfilmGrain);
		metaData.scriptClass->addInternalCall("Internal_setfilmGrain", (void*)&ScriptRenderSettings::Internal_setfilmGrain);
		metaData.scriptClass->addInternalCall("Internal_getmotionBlur", (void*)&ScriptRenderSettings::Internal_getmotionBlur);
		metaData.scriptClass->addInternalCall("Internal_setmotionBlur", (void*)&ScriptRenderSettings::Internal_setmotionBlur);
		metaData.scriptClass->addInternalCall("Internal_gettemporalAA", (void*)&ScriptRenderSettings::Internal_gettemporalAA);
		metaData.scriptClass->addInternalCall("Internal_settemporalAA", (void*)&ScriptRenderSettings::Internal_settemporalAA);
		metaData.scriptClass->addInternalCall("Internal_getenableFXAA", (void*)&ScriptRenderSettings::Internal_getenableFXAA);
		metaData.scriptClass->addInternalCall("Internal_setenableFXAA", (void*)&ScriptRenderSettings::Internal_setenableFXAA);
		metaData.scriptClass->addInternalCall("Internal_getexposureScale", (void*)&ScriptRenderSettings::Internal_getexposureScale);
		metaData.scriptClass->addInternalCall("Internal_setexposureScale", (void*)&ScriptRenderSettings::Internal_setexposureScale);
		metaData.scriptClass->addInternalCall("Internal_getgamma", (void*)&ScriptRenderSettings::Internal_getgamma);
		metaData.scriptClass->addInternalCall("Internal_setgamma", (void*)&ScriptRenderSettings::Internal_setgamma);
		metaData.scriptClass->addInternalCall("Internal_getenableHDR", (void*)&ScriptRenderSettings::Internal_getenableHDR);
		metaData.scriptClass->addInternalCall("Internal_setenableHDR", (void*)&ScriptRenderSettings::Internal_setenableHDR);
		metaData.scriptClass->addInternalCall("Internal_getenableLighting", (void*)&ScriptRenderSettings::Internal_getenableLighting);
		metaData.scriptClass->addInternalCall("Internal_setenableLighting", (void*)&ScriptRenderSettings::Internal_setenableLighting);
		metaData.scriptClass->addInternalCall("Internal_getenableShadows", (void*)&ScriptRenderSettings::Internal_getenableShadows);
		metaData.scriptClass->addInternalCall("Internal_setenableShadows", (void*)&ScriptRenderSettings::Internal_setenableShadows);
		metaData.scriptClass->addInternalCall("Internal_getenableVelocityBuffer", (void*)&ScriptRenderSettings::Internal_getenableVelocityBuffer);
		metaData.scriptClass->addInternalCall("Internal_setenableVelocityBuffer", (void*)&ScriptRenderSettings::Internal_setenableVelocityBuffer);
		metaData.scriptClass->addInternalCall("Internal_getshadowSettings", (void*)&ScriptRenderSettings::Internal_getshadowSettings);
		metaData.scriptClass->addInternalCall("Internal_setshadowSettings", (void*)&ScriptRenderSettings::Internal_setshadowSettings);
		metaData.scriptClass->addInternalCall("Internal_getenableIndirectLighting", (void*)&ScriptRenderSettings::Internal_getenableIndirectLighting);
		metaData.scriptClass->addInternalCall("Internal_setenableIndirectLighting", (void*)&ScriptRenderSettings::Internal_setenableIndirectLighting);
		metaData.scriptClass->addInternalCall("Internal_getoverlayOnly", (void*)&ScriptRenderSettings::Internal_getoverlayOnly);
		metaData.scriptClass->addInternalCall("Internal_setoverlayOnly", (void*)&ScriptRenderSettings::Internal_setoverlayOnly);
		metaData.scriptClass->addInternalCall("Internal_getenableSkybox", (void*)&ScriptRenderSettings::Internal_getenableSkybox);
		metaData.scriptClass->addInternalCall("Internal_setenableSkybox", (void*)&ScriptRenderSettings::Internal_setenableSkybox);
		metaData.scriptClass->addInternalCall("Internal_getcullDistance", (void*)&ScriptRenderSettings::Internal_getcullDistance);
		metaData.scriptClass->addInternalCall("Internal_setcullDistance", (void*)&ScriptRenderSettings::Internal_setcullDistance);

	}

	MonoObject* ScriptRenderSettings::create(const SPtr<RenderSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
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
		*tmp__output = thisPtr->getInternal()->depthOfField;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->depthOfField = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getchromaticAberration(ScriptRenderSettings* thisPtr)
	{
		SPtr<ChromaticAberrationSettings> tmp__output = bs_shared_ptr_new<ChromaticAberrationSettings>();
		*tmp__output = thisPtr->getInternal()->chromaticAberration;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->chromaticAberration = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableAutoExposure(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableAutoExposure;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableAutoExposure(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableAutoExposure = value;
	}

	MonoObject* ScriptRenderSettings::Internal_getautoExposure(ScriptRenderSettings* thisPtr)
	{
		SPtr<AutoExposureSettings> tmp__output = bs_shared_ptr_new<AutoExposureSettings>();
		*tmp__output = thisPtr->getInternal()->autoExposure;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->autoExposure = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableTonemapping(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableTonemapping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableTonemapping(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableTonemapping = value;
	}

	MonoObject* ScriptRenderSettings::Internal_gettonemapping(ScriptRenderSettings* thisPtr)
	{
		SPtr<TonemappingSettings> tmp__output = bs_shared_ptr_new<TonemappingSettings>();
		*tmp__output = thisPtr->getInternal()->tonemapping;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->tonemapping = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getwhiteBalance(ScriptRenderSettings* thisPtr)
	{
		SPtr<WhiteBalanceSettings> tmp__output = bs_shared_ptr_new<WhiteBalanceSettings>();
		*tmp__output = thisPtr->getInternal()->whiteBalance;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->whiteBalance = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getcolorGrading(ScriptRenderSettings* thisPtr)
	{
		SPtr<ColorGradingSettings> tmp__output = bs_shared_ptr_new<ColorGradingSettings>();
		*tmp__output = thisPtr->getInternal()->colorGrading;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->colorGrading = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getambientOcclusion(ScriptRenderSettings* thisPtr)
	{
		SPtr<AmbientOcclusionSettings> tmp__output = bs_shared_ptr_new<AmbientOcclusionSettings>();
		*tmp__output = thisPtr->getInternal()->ambientOcclusion;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->ambientOcclusion = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getscreenSpaceReflections(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceReflectionsSettings> tmp__output = bs_shared_ptr_new<ScreenSpaceReflectionsSettings>();
		*tmp__output = thisPtr->getInternal()->screenSpaceReflections;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->screenSpaceReflections = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getbloom(ScriptRenderSettings* thisPtr)
	{
		SPtr<BloomSettings> tmp__output = bs_shared_ptr_new<BloomSettings>();
		*tmp__output = thisPtr->getInternal()->bloom;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->bloom = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getscreenSpaceLensFlare(ScriptRenderSettings* thisPtr)
	{
		SPtr<ScreenSpaceLensFlareSettings> tmp__output = bs_shared_ptr_new<ScreenSpaceLensFlareSettings>();
		*tmp__output = thisPtr->getInternal()->screenSpaceLensFlare;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->screenSpaceLensFlare = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getfilmGrain(ScriptRenderSettings* thisPtr)
	{
		SPtr<FilmGrainSettings> tmp__output = bs_shared_ptr_new<FilmGrainSettings>();
		*tmp__output = thisPtr->getInternal()->filmGrain;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->filmGrain = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_getmotionBlur(ScriptRenderSettings* thisPtr)
	{
		SPtr<MotionBlurSettings> tmp__output = bs_shared_ptr_new<MotionBlurSettings>();
		*tmp__output = thisPtr->getInternal()->motionBlur;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->motionBlur = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::Internal_gettemporalAA(ScriptRenderSettings* thisPtr)
	{
		SPtr<TemporalAASettings> tmp__output = bs_shared_ptr_new<TemporalAASettings>();
		*tmp__output = thisPtr->getInternal()->temporalAA;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->temporalAA = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableFXAA(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableFXAA;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableFXAA(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableFXAA = value;
	}

	float ScriptRenderSettings::Internal_getexposureScale(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->exposureScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setexposureScale(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->exposureScale = value;
	}

	float ScriptRenderSettings::Internal_getgamma(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->gamma;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setgamma(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->gamma = value;
	}

	bool ScriptRenderSettings::Internal_getenableHDR(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableHDR;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableHDR(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableHDR = value;
	}

	bool ScriptRenderSettings::Internal_getenableLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableLighting = value;
	}

	bool ScriptRenderSettings::Internal_getenableShadows(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableShadows;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableShadows(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableShadows = value;
	}

	bool ScriptRenderSettings::Internal_getenableVelocityBuffer(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableVelocityBuffer;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableVelocityBuffer(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableVelocityBuffer = value;
	}

	MonoObject* ScriptRenderSettings::Internal_getshadowSettings(ScriptRenderSettings* thisPtr)
	{
		SPtr<ShadowSettings> tmp__output = bs_shared_ptr_new<ShadowSettings>();
		*tmp__output = thisPtr->getInternal()->shadowSettings;

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
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->shadowSettings = *tmpvalue;
	}

	bool ScriptRenderSettings::Internal_getenableIndirectLighting(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableIndirectLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableIndirectLighting(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableIndirectLighting = value;
	}

	bool ScriptRenderSettings::Internal_getoverlayOnly(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->overlayOnly;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setoverlayOnly(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->overlayOnly = value;
	}

	bool ScriptRenderSettings::Internal_getenableSkybox(ScriptRenderSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enableSkybox;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setenableSkybox(ScriptRenderSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enableSkybox = value;
	}

	float ScriptRenderSettings::Internal_getcullDistance(ScriptRenderSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->cullDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::Internal_setcullDistance(ScriptRenderSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->cullDistance = value;
	}
}
