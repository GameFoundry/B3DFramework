//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_BloomSettings", (void*)&ScriptBloomSettings::Internal_BloomSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptBloomSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptBloomSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_getquality", (void*)&ScriptBloomSettings::Internal_getquality);
		metaData.scriptClass->AddInternalCall("Internal_setquality", (void*)&ScriptBloomSettings::Internal_setquality);
		metaData.scriptClass->AddInternalCall("Internal_getthreshold", (void*)&ScriptBloomSettings::Internal_getthreshold);
		metaData.scriptClass->AddInternalCall("Internal_setthreshold", (void*)&ScriptBloomSettings::Internal_setthreshold);
		metaData.scriptClass->AddInternalCall("Internal_getintensity", (void*)&ScriptBloomSettings::Internal_getintensity);
		metaData.scriptClass->AddInternalCall("Internal_setintensity", (void*)&ScriptBloomSettings::Internal_setintensity);
		metaData.scriptClass->AddInternalCall("Internal_gettint", (void*)&ScriptBloomSettings::Internal_gettint);
		metaData.scriptClass->AddInternalCall("Internal_settint", (void*)&ScriptBloomSettings::Internal_settint);
		metaData.scriptClass->AddInternalCall("Internal_getfilterSize", (void*)&ScriptBloomSettings::Internal_getfilterSize);
		metaData.scriptClass->AddInternalCall("Internal_setfilterSize", (void*)&ScriptBloomSettings::Internal_setfilterSize);

	}

	MonoObject* ScriptBloomSettings::create(const SPtr<BloomSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptBloomSettings>()) ScriptBloomSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptBloomSettings::Internal_BloomSettings(MonoObject* managedInstance)
	{
		SPtr<BloomSettings> instance = bs_shared_ptr_new<BloomSettings>();
		new (bs_alloc<ScriptBloomSettings>())ScriptBloomSettings(managedInstance, instance);
	}

	bool ScriptBloomSettings::Internal_getenabled(ScriptBloomSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setenabled(ScriptBloomSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	uint32_t ScriptBloomSettings::Internal_getquality(ScriptBloomSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setquality(ScriptBloomSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->quality = value;
	}

	float ScriptBloomSettings::Internal_getthreshold(ScriptBloomSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setthreshold(ScriptBloomSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->threshold = value;
	}

	float ScriptBloomSettings::Internal_getintensity(ScriptBloomSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setintensity(ScriptBloomSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->intensity = value;
	}

	void ScriptBloomSettings::Internal_gettint(ScriptBloomSettings* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->tint;

		*__output = tmp__output;


	}

	void ScriptBloomSettings::Internal_settint(ScriptBloomSettings* thisPtr, Color* value)
	{
		thisPtr->GetInternal()->tint = *value;
	}

	float ScriptBloomSettings::Internal_getfilterSize(ScriptBloomSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setfilterSize(ScriptBloomSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filterSize = value;
	}
}
