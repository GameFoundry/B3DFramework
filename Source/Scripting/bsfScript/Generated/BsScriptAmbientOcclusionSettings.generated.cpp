//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAmbientOcclusionSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAmbientOcclusionSettings::ScriptAmbientOcclusionSettings(MonoObject* managedInstance, const SPtr<AmbientOcclusionSettings>& value)
		: TScriptReflectable(managedInstance, value)
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
		new(bs_alloc<ScriptAmbientOcclusionSettings>()) ScriptAmbientOcclusionSettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings(MonoObject* managedInstance)
	{
		SPtr<AmbientOcclusionSettings> instance = bs_shared_ptr_new<AmbientOcclusionSettings>();
		new(bs_alloc<ScriptAmbientOcclusionSettings>()) ScriptAmbientOcclusionSettings(managedInstance, instance);
	}

	bool ScriptAmbientOcclusionSettings::InternalGetEnabled(ScriptAmbientOcclusionSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetEnabled(ScriptAmbientOcclusionSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetRadius(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Radius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetRadius(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Radius = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetBias(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Bias;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetBias(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Bias = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeDistance(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FadeDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeDistance(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FadeDistance = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeRange(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FadeRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeRange(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FadeRange = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetIntensity(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetIntensity(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Intensity = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetPower(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Power;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetPower(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Power = value;
	}

	uint32_t ScriptAmbientOcclusionSettings::InternalGetQuality(ScriptAmbientOcclusionSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetQuality(ScriptAmbientOcclusionSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->Quality = value;
	}
} // namespace bs
