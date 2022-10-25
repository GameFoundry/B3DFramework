//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptWhiteBalanceSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptWhiteBalanceSettings::ScriptWhiteBalanceSettings(MonoObject* managedInstance, const SPtr<WhiteBalanceSettings>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptWhiteBalanceSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_WhiteBalanceSettings", (void*)&ScriptWhiteBalanceSettings::InternalWhiteBalanceSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetTemperature", (void*)&ScriptWhiteBalanceSettings::InternalGetTemperature);
		metaData.ScriptClass->AddInternalCall("Internal_SetTemperature", (void*)&ScriptWhiteBalanceSettings::InternalSetTemperature);
		metaData.ScriptClass->AddInternalCall("Internal_GetTint", (void*)&ScriptWhiteBalanceSettings::InternalGetTint);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptWhiteBalanceSettings::InternalSetTint);
	}

	MonoObject* ScriptWhiteBalanceSettings::Create(const SPtr<WhiteBalanceSettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptWhiteBalanceSettings>()) ScriptWhiteBalanceSettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptWhiteBalanceSettings::InternalWhiteBalanceSettings(MonoObject* managedInstance)
	{
		SPtr<WhiteBalanceSettings> instance = bs_shared_ptr_new<WhiteBalanceSettings>();
		new(bs_alloc<ScriptWhiteBalanceSettings>()) ScriptWhiteBalanceSettings(managedInstance, instance);
	}

	float ScriptWhiteBalanceSettings::InternalGetTemperature(ScriptWhiteBalanceSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Temperature;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTemperature(ScriptWhiteBalanceSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Temperature = value;
	}

	float ScriptWhiteBalanceSettings::InternalGetTint(ScriptWhiteBalanceSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Tint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTint(ScriptWhiteBalanceSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Tint = value;
	}
} // namespace bs
