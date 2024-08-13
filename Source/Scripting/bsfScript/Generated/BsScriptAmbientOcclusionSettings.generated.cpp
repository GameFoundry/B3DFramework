//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAmbientOcclusionSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAmbientOcclusionSettings::ScriptAmbientOcclusionSettings(MonoObject* managedInstance, const SPtr<AmbientOcclusionSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptAmbientOcclusionSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_AmbientOcclusionSettings", (void*)&ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptAmbientOcclusionSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptAmbientOcclusionSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptAmbientOcclusionSettings::InternalGetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptAmbientOcclusionSettings::InternalSetRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetBias", (void*)&ScriptAmbientOcclusionSettings::InternalGetBias);
		metaData.ScriptClass->AddInternalCall("Internal_SetBias", (void*)&ScriptAmbientOcclusionSettings::InternalSetBias);
		metaData.ScriptClass->AddInternalCall("Internal_GetFadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalGetFadeDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetFadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalSetFadeDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetFadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalGetFadeRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetFadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalSetFadeRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptAmbientOcclusionSettings::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptAmbientOcclusionSettings::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetPower", (void*)&ScriptAmbientOcclusionSettings::InternalGetPower);
		metaData.ScriptClass->AddInternalCall("Internal_SetPower", (void*)&ScriptAmbientOcclusionSettings::InternalSetPower);
		metaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptAmbientOcclusionSettings::InternalGetQuality);
		metaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptAmbientOcclusionSettings::InternalSetQuality);

	}

	MonoObject* ScriptAmbientOcclusionSettings::Create(const SPtr<AmbientOcclusionSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAmbientOcclusionSettings>()) ScriptAmbientOcclusionSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings(MonoObject* managedInstance)
	{
		SPtr<AmbientOcclusionSettings> nativeObject = B3DMakeShared<AmbientOcclusionSettings>();
		new (B3DAllocate<ScriptAmbientOcclusionSettings>())ScriptAmbientOcclusionSettings(managedInstance, nativeObject);
	}

	bool ScriptAmbientOcclusionSettings::InternalGetEnabled(ScriptAmbientOcclusionSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetEnabled(ScriptAmbientOcclusionSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetRadius(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Radius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetRadius(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->Radius = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetBias(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Bias;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetBias(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->Bias = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeDistance(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FadeDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeDistance(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->FadeDistance = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeRange(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FadeRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeRange(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->FadeRange = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetIntensity(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetIntensity(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->Intensity = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetPower(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Power;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetPower(ScriptAmbientOcclusionSettings* self, float value)
	{
		self->GetInternal()->Power = value;
	}

	uint32_t ScriptAmbientOcclusionSettings::InternalGetQuality(ScriptAmbientOcclusionSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetQuality(ScriptAmbientOcclusionSettings* self, uint32_t value)
	{
		self->GetInternal()->Quality = value;
	}
}
