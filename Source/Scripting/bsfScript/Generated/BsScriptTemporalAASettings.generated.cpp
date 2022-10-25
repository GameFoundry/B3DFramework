//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTemporalAASettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTemporalAASettings::ScriptTemporalAASettings(MonoObject* managedInstance, const SPtr<TemporalAASettings>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptTemporalAASettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TemporalAASettings", (void*)&ScriptTemporalAASettings::InternalTemporalAASettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptTemporalAASettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptTemporalAASettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetJitteredPositionCount", (void*)&ScriptTemporalAASettings::InternalGetJitteredPositionCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetJitteredPositionCount", (void*)&ScriptTemporalAASettings::InternalSetJitteredPositionCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetSharpness", (void*)&ScriptTemporalAASettings::InternalGetSharpness);
		metaData.ScriptClass->AddInternalCall("Internal_SetSharpness", (void*)&ScriptTemporalAASettings::InternalSetSharpness);
	}

	MonoObject* ScriptTemporalAASettings::Create(const SPtr<TemporalAASettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptTemporalAASettings>()) ScriptTemporalAASettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptTemporalAASettings::InternalTemporalAASettings(MonoObject* managedInstance)
	{
		SPtr<TemporalAASettings> instance = bs_shared_ptr_new<TemporalAASettings>();
		new(bs_alloc<ScriptTemporalAASettings>()) ScriptTemporalAASettings(managedInstance, instance);
	}

	bool ScriptTemporalAASettings::InternalGetEnabled(ScriptTemporalAASettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetEnabled(ScriptTemporalAASettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	uint32_t ScriptTemporalAASettings::InternalGetJitteredPositionCount(ScriptTemporalAASettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->JitteredPositionCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetJitteredPositionCount(ScriptTemporalAASettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->JitteredPositionCount = value;
	}

	float ScriptTemporalAASettings::InternalGetSharpness(ScriptTemporalAASettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Sharpness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetSharpness(ScriptTemporalAASettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Sharpness = value;
	}
} // namespace bs
