//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradingSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptColorGradingSettings::ScriptColorGradingSettings(MonoObject* managedInstance, const SPtr<ColorGradingSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptColorGradingSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getsaturation", (void*)&ScriptColorGradingSettings::Internal_getsaturation);
		metaData.scriptClass->AddInternalCall("Internal_setsaturation", (void*)&ScriptColorGradingSettings::Internal_setsaturation);
		metaData.scriptClass->AddInternalCall("Internal_getcontrast", (void*)&ScriptColorGradingSettings::Internal_getcontrast);
		metaData.scriptClass->AddInternalCall("Internal_setcontrast", (void*)&ScriptColorGradingSettings::Internal_setcontrast);
		metaData.scriptClass->AddInternalCall("Internal_getgain", (void*)&ScriptColorGradingSettings::Internal_getgain);
		metaData.scriptClass->AddInternalCall("Internal_setgain", (void*)&ScriptColorGradingSettings::Internal_setgain);
		metaData.scriptClass->AddInternalCall("Internal_getoffset", (void*)&ScriptColorGradingSettings::Internal_getoffset);
		metaData.scriptClass->AddInternalCall("Internal_setoffset", (void*)&ScriptColorGradingSettings::Internal_setoffset);

	}

	MonoObject* ScriptColorGradingSettings::create(const SPtr<ColorGradingSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptColorGradingSettings>()) ScriptColorGradingSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradingSettings::Internal_getsaturation(ScriptColorGradingSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->saturation;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::Internal_setsaturation(ScriptColorGradingSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->saturation = *value;
	}

	void ScriptColorGradingSettings::Internal_getcontrast(ScriptColorGradingSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->contrast;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::Internal_setcontrast(ScriptColorGradingSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->contrast = *value;
	}

	void ScriptColorGradingSettings::Internal_getgain(ScriptColorGradingSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->gain;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::Internal_setgain(ScriptColorGradingSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->gain = *value;
	}

	void ScriptColorGradingSettings::Internal_getoffset(ScriptColorGradingSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->offset;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::Internal_setoffset(ScriptColorGradingSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->offset = *value;
	}
}
