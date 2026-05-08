//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptScreenSpaceLensFlareSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptScreenSpaceLensFlareSettings::ScriptScreenSpaceLensFlareSettings(const TShared<ScreenSpaceLensFlareSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptScreenSpaceLensFlareSettings::~ScriptScreenSpaceLensFlareSettings()
	{
		UnregisterEvents();
	}

	void ScriptScreenSpaceLensFlareSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenSpaceLensFlareSettings", (void*)&ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBrightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBrightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHalo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHalo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling);

	}

	MonoObject* ScriptScreenSpaceLensFlareSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings(MonoObject* scriptObject)
	{
		TShared<ScreenSpaceLensFlareSettings> nativeObject = B3DMakeShared<ScreenSpaceLensFlareSettings>();
		ScriptObjectWrapper::Create<ScriptScreenSpaceLensFlareSettings>(nativeObject, scriptObject);
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetEnabled(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetEnabled(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Enabled = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->DownsampleCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->DownsampleCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetThreshold(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetThreshold(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Threshold = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount(ScriptScreenSpaceLensFlareSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->GhostCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->GhostCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->GhostSpacing;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->GhostSpacing = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetBrightness(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Brightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetBrightness(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Brightness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->FilterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->FilterSize = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetHalo(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Halo;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHalo(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->Halo = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloRadius = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloThickness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloThickness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloThreshold = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloAspectRatio;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->HaloAspectRatio = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->ChromaticAberration;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->ChromaticAberration = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->ChromaticAberrationOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->ChromaticAberrationOffset = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->BicubicUpsampling;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceLensFlareSettings*>(self->GetNativeObject())->BicubicUpsampling = value;
	}
}
