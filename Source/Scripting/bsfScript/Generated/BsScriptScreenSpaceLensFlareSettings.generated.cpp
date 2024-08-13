//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceLensFlareSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptScreenSpaceLensFlareSettings::ScriptScreenSpaceLensFlareSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceLensFlareSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptScreenSpaceLensFlareSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ScreenSpaceLensFlareSettings", (void*)&ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetDownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetGhostCount", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing);
		metaData.ScriptClass->AddInternalCall("Internal_SetGhostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing);
		metaData.ScriptClass->AddInternalCall("Internal_GetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBrightness);
		metaData.ScriptClass->AddInternalCall("Internal_SetBrightness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBrightness);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHalo);
		metaData.ScriptClass->AddInternalCall("Internal_SetHalo", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHalo);
		metaData.ScriptClass->AddInternalCall("Internal_GetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetHaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness);
		metaData.ScriptClass->AddInternalCall("Internal_SetHaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness);
		metaData.ScriptClass->AddInternalCall("Internal_GetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetHaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_SetHaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration);
		metaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration);
		metaData.ScriptClass->AddInternalCall("Internal_GetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetChromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling);
		metaData.ScriptClass->AddInternalCall("Internal_SetBicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling);

	}

	MonoObject* ScriptScreenSpaceLensFlareSettings::Create(const SPtr<ScreenSpaceLensFlareSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptScreenSpaceLensFlareSettings>()) ScriptScreenSpaceLensFlareSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptScreenSpaceLensFlareSettings::InternalScreenSpaceLensFlareSettings(MonoObject* managedInstance)
	{
		SPtr<ScreenSpaceLensFlareSettings> nativeObject = B3DMakeShared<ScreenSpaceLensFlareSettings>();
		new (B3DAllocate<ScriptScreenSpaceLensFlareSettings>())ScriptScreenSpaceLensFlareSettings(managedInstance, nativeObject);
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetEnabled(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetEnabled(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->DownsampleCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value)
	{
		self->GetInternal()->DownsampleCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetThreshold(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetThreshold(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->Threshold = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::InternalGetGhostCount(ScriptScreenSpaceLensFlareSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->GhostCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetGhostCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value)
	{
		self->GetInternal()->GhostCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->GhostSpacing;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->GhostSpacing = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetBrightness(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Brightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetBrightness(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->Brightness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetFilterSize(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetFilterSize(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->FilterSize = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetHalo(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Halo;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHalo(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		self->GetInternal()->Halo = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloRadius(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HaloRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloRadius(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->HaloRadius = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThickness(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HaloThickness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThickness(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->HaloThickness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HaloThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->HaloThreshold = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HaloAspectRatio;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->HaloAspectRatio = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->ChromaticAberration;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		self->GetInternal()->ChromaticAberration = value;
	}

	float ScriptScreenSpaceLensFlareSettings::InternalGetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->ChromaticAberrationOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self, float value)
	{
		self->GetInternal()->ChromaticAberrationOffset = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::InternalGetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->BicubicUpsampling;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::InternalSetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self, bool value)
	{
		self->GetInternal()->BicubicUpsampling = value;
	}
}
