//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBloomSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptBloomSettings::ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptBloomSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_BloomSettings", (void*)&ScriptBloomSettings::InternalBloomSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptBloomSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptBloomSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptBloomSettings::InternalGetQuality);
		metaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptBloomSettings::InternalSetQuality);
		metaData.ScriptClass->AddInternalCall("Internal_GetThreshold", (void*)&ScriptBloomSettings::InternalGetThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetThreshold", (void*)&ScriptBloomSettings::InternalSetThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptBloomSettings::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptBloomSettings::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetTint", (void*)&ScriptBloomSettings::InternalGetTint);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptBloomSettings::InternalSetTint);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilterSize", (void*)&ScriptBloomSettings::InternalGetFilterSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilterSize", (void*)&ScriptBloomSettings::InternalSetFilterSize);

	}

	MonoObject* ScriptBloomSettings::Create(const SPtr<BloomSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptBloomSettings>()) ScriptBloomSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptBloomSettings::InternalBloomSettings(MonoObject* managedInstance)
	{
		SPtr<BloomSettings> nativeObject = B3DMakeShared<BloomSettings>();
		new (B3DAllocate<ScriptBloomSettings>())ScriptBloomSettings(managedInstance, nativeObject);
	}

	bool ScriptBloomSettings::InternalGetEnabled(ScriptBloomSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetEnabled(ScriptBloomSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	uint32_t ScriptBloomSettings::InternalGetQuality(ScriptBloomSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetQuality(ScriptBloomSettings* self, uint32_t value)
	{
		self->GetInternal()->Quality = value;
	}

	float ScriptBloomSettings::InternalGetThreshold(ScriptBloomSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetThreshold(ScriptBloomSettings* self, float value)
	{
		self->GetInternal()->Threshold = value;
	}

	float ScriptBloomSettings::InternalGetIntensity(ScriptBloomSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetIntensity(ScriptBloomSettings* self, float value)
	{
		self->GetInternal()->Intensity = value;
	}

	void ScriptBloomSettings::InternalGetTint(ScriptBloomSettings* self, Color* __output)
	{
		Color tmp__output;
		tmp__output = self->GetInternal()->Tint;

		*__output = tmp__output;


	}

	void ScriptBloomSettings::InternalSetTint(ScriptBloomSettings* self, Color* value)
	{
		self->GetInternal()->Tint = *value;
	}

	float ScriptBloomSettings::InternalGetFilterSize(ScriptBloomSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetFilterSize(ScriptBloomSettings* self, float value)
	{
		self->GetInternal()->FilterSize = value;
	}
}
