//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRenderSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptDepthOfFieldSettings.generated.h"
#include "B3DScriptWhiteBalanceSettings.generated.h"
#include "B3DScriptChromaticAberrationSettings.generated.h"
#include "B3DScriptAutoExposureSettings.generated.h"
#include "B3DScriptTonemappingSettings.generated.h"
#include "B3DScriptScreenSpaceReflectionsSettings.generated.h"
#include "B3DScriptColorGradingSettings.generated.h"
#include "B3DScriptAmbientOcclusionSettings.generated.h"
#include "B3DScriptBloomSettings.generated.h"
#include "B3DScriptScreenSpaceLensFlareSettings.generated.h"
#include "B3DScriptFilmGrainSettings.generated.h"
#include "B3DScriptMotionBlurSettings.generated.h"
#include "B3DScriptTemporalAASettings.generated.h"
#include "B3DScriptShadowSettings.generated.h"

namespace b3d
{
	ScriptRenderSettings::ScriptRenderSettings(const TShared<RenderSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRenderSettings::~ScriptRenderSettings()
	{
		UnregisterEvents();
	}

	void ScriptRenderSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RenderSettings", (void*)&ScriptRenderSettings::InternalRenderSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepthOfField", (void*)&ScriptRenderSettings::InternalGetDepthOfField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDepthOfField", (void*)&ScriptRenderSettings::InternalSetDepthOfField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberration", (void*)&ScriptRenderSettings::InternalGetChromaticAberration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberration", (void*)&ScriptRenderSettings::InternalSetChromaticAberration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableAutoExposure", (void*)&ScriptRenderSettings::InternalGetEnableAutoExposure);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableAutoExposure", (void*)&ScriptRenderSettings::InternalSetEnableAutoExposure);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAutoExposure", (void*)&ScriptRenderSettings::InternalGetAutoExposure);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAutoExposure", (void*)&ScriptRenderSettings::InternalSetAutoExposure);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableTonemapping", (void*)&ScriptRenderSettings::InternalGetEnableTonemapping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableTonemapping", (void*)&ScriptRenderSettings::InternalSetEnableTonemapping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTonemapping", (void*)&ScriptRenderSettings::InternalGetTonemapping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTonemapping", (void*)&ScriptRenderSettings::InternalSetTonemapping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWhiteBalance", (void*)&ScriptRenderSettings::InternalGetWhiteBalance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWhiteBalance", (void*)&ScriptRenderSettings::InternalSetWhiteBalance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColorGrading", (void*)&ScriptRenderSettings::InternalGetColorGrading);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColorGrading", (void*)&ScriptRenderSettings::InternalSetColorGrading);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAmbientOcclusion", (void*)&ScriptRenderSettings::InternalGetAmbientOcclusion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAmbientOcclusion", (void*)&ScriptRenderSettings::InternalSetAmbientOcclusion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScreenSpaceReflections", (void*)&ScriptRenderSettings::InternalGetScreenSpaceReflections);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScreenSpaceReflections", (void*)&ScriptRenderSettings::InternalSetScreenSpaceReflections);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBloom", (void*)&ScriptRenderSettings::InternalGetBloom);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBloom", (void*)&ScriptRenderSettings::InternalSetBloom);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScreenSpaceLensFlare", (void*)&ScriptRenderSettings::InternalGetScreenSpaceLensFlare);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScreenSpaceLensFlare", (void*)&ScriptRenderSettings::InternalSetScreenSpaceLensFlare);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmGrain", (void*)&ScriptRenderSettings::InternalGetFilmGrain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmGrain", (void*)&ScriptRenderSettings::InternalSetFilmGrain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMotionBlur", (void*)&ScriptRenderSettings::InternalGetMotionBlur);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMotionBlur", (void*)&ScriptRenderSettings::InternalSetMotionBlur);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTemporalAa", (void*)&ScriptRenderSettings::InternalGetTemporalAa);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTemporalAa", (void*)&ScriptRenderSettings::InternalSetTemporalAa);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableFxaa", (void*)&ScriptRenderSettings::InternalGetEnableFxaa);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableFxaa", (void*)&ScriptRenderSettings::InternalSetEnableFxaa);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetExposureScale", (void*)&ScriptRenderSettings::InternalGetExposureScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetExposureScale", (void*)&ScriptRenderSettings::InternalSetExposureScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGamma", (void*)&ScriptRenderSettings::InternalGetGamma);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGamma", (void*)&ScriptRenderSettings::InternalSetGamma);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableHdr", (void*)&ScriptRenderSettings::InternalGetEnableHdr);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableHdr", (void*)&ScriptRenderSettings::InternalSetEnableHdr);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableLighting", (void*)&ScriptRenderSettings::InternalGetEnableLighting);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableLighting", (void*)&ScriptRenderSettings::InternalSetEnableLighting);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableShadows", (void*)&ScriptRenderSettings::InternalGetEnableShadows);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableShadows", (void*)&ScriptRenderSettings::InternalSetEnableShadows);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableVelocityBuffer", (void*)&ScriptRenderSettings::InternalGetEnableVelocityBuffer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableVelocityBuffer", (void*)&ScriptRenderSettings::InternalSetEnableVelocityBuffer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShadowSettings", (void*)&ScriptRenderSettings::InternalGetShadowSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShadowSettings", (void*)&ScriptRenderSettings::InternalSetShadowSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableIndirectLighting", (void*)&ScriptRenderSettings::InternalGetEnableIndirectLighting);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableIndirectLighting", (void*)&ScriptRenderSettings::InternalSetEnableIndirectLighting);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOverlayOnly", (void*)&ScriptRenderSettings::InternalGetOverlayOnly);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOverlayOnly", (void*)&ScriptRenderSettings::InternalSetOverlayOnly);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableSkybox", (void*)&ScriptRenderSettings::InternalGetEnableSkybox);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableSkybox", (void*)&ScriptRenderSettings::InternalSetEnableSkybox);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCullDistance", (void*)&ScriptRenderSettings::InternalGetCullDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCullDistance", (void*)&ScriptRenderSettings::InternalSetCullDistance);

	}

	MonoObject* ScriptRenderSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptRenderSettings::InternalRenderSettings(MonoObject* scriptObject)
	{
		TShared<RenderSettings> nativeObject = B3DMakeShared<RenderSettings>();
		ScriptObjectWrapper::Create<ScriptRenderSettings>(nativeObject, scriptObject);
	}

	MonoObject* ScriptRenderSettings::InternalGetDepthOfField(ScriptRenderSettings* self)
	{
		TShared<DepthOfFieldSettings> tmp__output = B3DMakeShared<DepthOfFieldSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->DepthOfField;

		MonoObject* __output;
		__output = ScriptDepthOfFieldSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetDepthOfField(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<DepthOfFieldSettings> tmpvalue;
		ScriptDepthOfFieldSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptDepthOfFieldSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<DepthOfFieldSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->DepthOfField = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetChromaticAberration(ScriptRenderSettings* self)
	{
		TShared<ChromaticAberrationSettings> tmp__output = B3DMakeShared<ChromaticAberrationSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ChromaticAberration;

		MonoObject* __output;
		__output = ScriptChromaticAberrationSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetChromaticAberration(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ChromaticAberrationSettings> tmpvalue;
		ScriptChromaticAberrationSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptChromaticAberrationSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ChromaticAberrationSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->ChromaticAberration = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableAutoExposure(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableAutoExposure;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableAutoExposure(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableAutoExposure = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetAutoExposure(ScriptRenderSettings* self)
	{
		TShared<AutoExposureSettings> tmp__output = B3DMakeShared<AutoExposureSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->AutoExposure;

		MonoObject* __output;
		__output = ScriptAutoExposureSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAutoExposure(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<AutoExposureSettings> tmpvalue;
		ScriptAutoExposureSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAutoExposureSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<AutoExposureSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->AutoExposure = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableTonemapping(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableTonemapping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableTonemapping(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableTonemapping = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetTonemapping(ScriptRenderSettings* self)
	{
		TShared<TonemappingSettings> tmp__output = B3DMakeShared<TonemappingSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->Tonemapping;

		MonoObject* __output;
		__output = ScriptTonemappingSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTonemapping(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TonemappingSettings> tmpvalue;
		ScriptTonemappingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTonemappingSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TonemappingSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->Tonemapping = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetWhiteBalance(ScriptRenderSettings* self)
	{
		TShared<WhiteBalanceSettings> tmp__output = B3DMakeShared<WhiteBalanceSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->WhiteBalance;

		MonoObject* __output;
		__output = ScriptWhiteBalanceSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetWhiteBalance(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<WhiteBalanceSettings> tmpvalue;
		ScriptWhiteBalanceSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptWhiteBalanceSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<WhiteBalanceSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->WhiteBalance = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetColorGrading(ScriptRenderSettings* self)
	{
		TShared<ColorGradingSettings> tmp__output = B3DMakeShared<ColorGradingSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ColorGrading;

		MonoObject* __output;
		__output = ScriptColorGradingSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetColorGrading(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ColorGradingSettings> tmpvalue;
		ScriptColorGradingSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorGradingSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ColorGradingSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->ColorGrading = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetAmbientOcclusion(ScriptRenderSettings* self)
	{
		TShared<AmbientOcclusionSettings> tmp__output = B3DMakeShared<AmbientOcclusionSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->AmbientOcclusion;

		MonoObject* __output;
		__output = ScriptAmbientOcclusionSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetAmbientOcclusion(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<AmbientOcclusionSettings> tmpvalue;
		ScriptAmbientOcclusionSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptAmbientOcclusionSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<AmbientOcclusionSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->AmbientOcclusion = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceReflections(ScriptRenderSettings* self)
	{
		TShared<ScreenSpaceReflectionsSettings> tmp__output = B3DMakeShared<ScreenSpaceReflectionsSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ScreenSpaceReflections;

		MonoObject* __output;
		__output = ScriptScreenSpaceReflectionsSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceReflections(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ScreenSpaceReflectionsSettings> tmpvalue;
		ScriptScreenSpaceReflectionsSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceReflectionsSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ScreenSpaceReflectionsSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->ScreenSpaceReflections = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetBloom(ScriptRenderSettings* self)
	{
		TShared<BloomSettings> tmp__output = B3DMakeShared<BloomSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->Bloom;

		MonoObject* __output;
		__output = ScriptBloomSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetBloom(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<BloomSettings> tmpvalue;
		ScriptBloomSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptBloomSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<BloomSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->Bloom = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetScreenSpaceLensFlare(ScriptRenderSettings* self)
	{
		TShared<ScreenSpaceLensFlareSettings> tmp__output = B3DMakeShared<ScreenSpaceLensFlareSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ScreenSpaceLensFlare;

		MonoObject* __output;
		__output = ScriptScreenSpaceLensFlareSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetScreenSpaceLensFlare(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ScreenSpaceLensFlareSettings> tmpvalue;
		ScriptScreenSpaceLensFlareSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptScreenSpaceLensFlareSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ScreenSpaceLensFlareSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->ScreenSpaceLensFlare = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetFilmGrain(ScriptRenderSettings* self)
	{
		TShared<FilmGrainSettings> tmp__output = B3DMakeShared<FilmGrainSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->FilmGrain;

		MonoObject* __output;
		__output = ScriptFilmGrainSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetFilmGrain(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<FilmGrainSettings> tmpvalue;
		ScriptFilmGrainSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptFilmGrainSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<FilmGrainSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->FilmGrain = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetMotionBlur(ScriptRenderSettings* self)
	{
		TShared<MotionBlurSettings> tmp__output = B3DMakeShared<MotionBlurSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->MotionBlur;

		MonoObject* __output;
		__output = ScriptMotionBlurSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetMotionBlur(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<MotionBlurSettings> tmpvalue;
		ScriptMotionBlurSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptMotionBlurSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<MotionBlurSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->MotionBlur = *tmpvalue;
	}

	MonoObject* ScriptRenderSettings::InternalGetTemporalAa(ScriptRenderSettings* self)
	{
		TShared<TemporalAASettings> tmp__output = B3DMakeShared<TemporalAASettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->TemporalAa;

		MonoObject* __output;
		__output = ScriptTemporalAASettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetTemporalAa(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TemporalAASettings> tmpvalue;
		ScriptTemporalAASettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptTemporalAASettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TemporalAASettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->TemporalAa = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableFxaa(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableFxaa;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableFxaa(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableFxaa = value;
	}

	float ScriptRenderSettings::InternalGetExposureScale(ScriptRenderSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ExposureScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetExposureScale(ScriptRenderSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->ExposureScale = value;
	}

	float ScriptRenderSettings::InternalGetGamma(ScriptRenderSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->Gamma;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetGamma(ScriptRenderSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->Gamma = value;
	}

	bool ScriptRenderSettings::InternalGetEnableHdr(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableHdr;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableHdr(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableHdr = value;
	}

	bool ScriptRenderSettings::InternalGetEnableLighting(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableLighting(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableLighting = value;
	}

	bool ScriptRenderSettings::InternalGetEnableShadows(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableShadows;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableShadows(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableShadows = value;
	}

	bool ScriptRenderSettings::InternalGetEnableVelocityBuffer(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableVelocityBuffer;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableVelocityBuffer(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableVelocityBuffer = value;
	}

	MonoObject* ScriptRenderSettings::InternalGetShadowSettings(ScriptRenderSettings* self)
	{
		TShared<ShadowSettings> tmp__output = B3DMakeShared<ShadowSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->ShadowSettings;

		MonoObject* __output;
		__output = ScriptShadowSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptRenderSettings::InternalSetShadowSettings(ScriptRenderSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ShadowSettings> tmpvalue;
		ScriptShadowSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptShadowSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ShadowSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<RenderSettings*>(self->GetNativeObject())->ShadowSettings = *tmpvalue;
	}

	bool ScriptRenderSettings::InternalGetEnableIndirectLighting(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableIndirectLighting;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableIndirectLighting(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableIndirectLighting = value;
	}

	bool ScriptRenderSettings::InternalGetOverlayOnly(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->OverlayOnly;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetOverlayOnly(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->OverlayOnly = value;
	}

	bool ScriptRenderSettings::InternalGetEnableSkybox(ScriptRenderSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->EnableSkybox;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetEnableSkybox(ScriptRenderSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->EnableSkybox = value;
	}

	float ScriptRenderSettings::InternalGetCullDistance(ScriptRenderSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<RenderSettings*>(self->GetNativeObject())->CullDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderSettings::InternalSetCullDistance(ScriptRenderSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<RenderSettings*>(self->GetNativeObject())->CullDistance = value;
	}
}
