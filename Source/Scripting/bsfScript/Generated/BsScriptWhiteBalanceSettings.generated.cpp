//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptWhiteBalanceSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptWhiteBalanceSettings::ScriptWhiteBalanceSettings(MonoObject* managedInstance, const SPtr<WhiteBalanceSettings>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptWhiteBalanceSettings>()) ScriptWhiteBalanceSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptWhiteBalanceSettings::InternalWhiteBalanceSettings(MonoObject* managedInstance)
	{
		SPtr<WhiteBalanceSettings> nativeObject = B3DMakeShared<WhiteBalanceSettings>();
		new (B3DAllocate<ScriptWhiteBalanceSettings>())ScriptWhiteBalanceSettings(managedInstance, nativeObject);
	}

	float ScriptWhiteBalanceSettings::InternalGetTemperature(ScriptWhiteBalanceSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Temperature;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTemperature(ScriptWhiteBalanceSettings* self, float value)
	{
		self->GetInternal()->Temperature = value;
	}

	float ScriptWhiteBalanceSettings::InternalGetTint(ScriptWhiteBalanceSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Tint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTint(ScriptWhiteBalanceSettings* self, float value)
	{
		self->GetInternal()->Tint = value;
	}
}
