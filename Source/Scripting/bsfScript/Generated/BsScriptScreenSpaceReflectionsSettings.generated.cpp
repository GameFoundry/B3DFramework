//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceReflectionsSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptScreenSpaceReflectionsSettings::ScriptScreenSpaceReflectionsSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceReflectionsSettings>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptScreenSpaceReflectionsSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ScreenSpaceReflectionsSettings", (void*)&ScriptScreenSpaceReflectionsSettings::InternalScreenSpaceReflectionsSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetQuality);
		metaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetQuality);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetMaxRoughness);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetMaxRoughness);
	}

	MonoObject* ScriptScreenSpaceReflectionsSettings::Create(const SPtr<ScreenSpaceReflectionsSettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptScreenSpaceReflectionsSettings>()) ScriptScreenSpaceReflectionsSettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalScreenSpaceReflectionsSettings(MonoObject* managedInstance)
	{
		SPtr<ScreenSpaceReflectionsSettings> instance = bs_shared_ptr_new<ScreenSpaceReflectionsSettings>();
		new(bs_alloc<ScriptScreenSpaceReflectionsSettings>()) ScriptScreenSpaceReflectionsSettings(managedInstance, instance);
	}

	bool ScriptScreenSpaceReflectionsSettings::InternalGetEnabled(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetEnabled(ScriptScreenSpaceReflectionsSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	uint32_t ScriptScreenSpaceReflectionsSettings::InternalGetQuality(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetQuality(ScriptScreenSpaceReflectionsSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->Quality = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetIntensity(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetIntensity(ScriptScreenSpaceReflectionsSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Intensity = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetMaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->MaxRoughness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetMaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->MaxRoughness = value;
	}
} // namespace bs
