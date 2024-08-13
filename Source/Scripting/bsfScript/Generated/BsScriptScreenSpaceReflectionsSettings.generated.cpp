//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceReflectionsSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptScreenSpaceReflectionsSettings::ScriptScreenSpaceReflectionsSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceReflectionsSettings>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptScreenSpaceReflectionsSettings>()) ScriptScreenSpaceReflectionsSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptScreenSpaceReflectionsSettings::InternalScreenSpaceReflectionsSettings(MonoObject* managedInstance)
	{
		SPtr<ScreenSpaceReflectionsSettings> nativeObject = B3DMakeShared<ScreenSpaceReflectionsSettings>();
		new (B3DAllocate<ScriptScreenSpaceReflectionsSettings>())ScriptScreenSpaceReflectionsSettings(managedInstance, nativeObject);
	}

	bool ScriptScreenSpaceReflectionsSettings::InternalGetEnabled(ScriptScreenSpaceReflectionsSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetEnabled(ScriptScreenSpaceReflectionsSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	uint32_t ScriptScreenSpaceReflectionsSettings::InternalGetQuality(ScriptScreenSpaceReflectionsSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetQuality(ScriptScreenSpaceReflectionsSettings* self, uint32_t value)
	{
		self->GetInternal()->Quality = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetIntensity(ScriptScreenSpaceReflectionsSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetIntensity(ScriptScreenSpaceReflectionsSettings* self, float value)
	{
		self->GetInternal()->Intensity = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->MaxRoughness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self, float value)
	{
		self->GetInternal()->MaxRoughness = value;
	}
}
