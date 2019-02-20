//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptWhiteBalanceSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_WhiteBalanceSettings", (void*)&ScriptWhiteBalanceSettings::Internal_WhiteBalanceSettings);
		metaData.scriptClass->addInternalCall("Internal_gettemperature", (void*)&ScriptWhiteBalanceSettings::Internal_gettemperature);
		metaData.scriptClass->addInternalCall("Internal_settemperature", (void*)&ScriptWhiteBalanceSettings::Internal_settemperature);
		metaData.scriptClass->addInternalCall("Internal_gettint", (void*)&ScriptWhiteBalanceSettings::Internal_gettint);
		metaData.scriptClass->addInternalCall("Internal_settint", (void*)&ScriptWhiteBalanceSettings::Internal_settint);

	}

	MonoObject* ScriptWhiteBalanceSettings::create(const SPtr<WhiteBalanceSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptWhiteBalanceSettings>()) ScriptWhiteBalanceSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptWhiteBalanceSettings::Internal_WhiteBalanceSettings(MonoObject* managedInstance)
	{
		SPtr<WhiteBalanceSettings> instance = bs_shared_ptr_new<WhiteBalanceSettings>();
		new (bs_alloc<ScriptWhiteBalanceSettings>())ScriptWhiteBalanceSettings(managedInstance, instance);
	}

	float ScriptWhiteBalanceSettings::Internal_gettemperature(ScriptWhiteBalanceSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->temperature;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::Internal_settemperature(ScriptWhiteBalanceSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->temperature = value;
	}

	float ScriptWhiteBalanceSettings::Internal_gettint(ScriptWhiteBalanceSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->tint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::Internal_settint(ScriptWhiteBalanceSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->tint = value;
	}
}
