//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBloomSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptBloomSettings::ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptBloomSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_BloomSettings", (void*)&ScriptBloomSettings::Internal_BloomSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptBloomSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptBloomSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getquality", (void*)&ScriptBloomSettings::Internal_getquality);
		metaData.scriptClass->addInternalCall("Internal_setquality", (void*)&ScriptBloomSettings::Internal_setquality);
		metaData.scriptClass->addInternalCall("Internal_getthreshold", (void*)&ScriptBloomSettings::Internal_getthreshold);
		metaData.scriptClass->addInternalCall("Internal_setthreshold", (void*)&ScriptBloomSettings::Internal_setthreshold);
		metaData.scriptClass->addInternalCall("Internal_getintensity", (void*)&ScriptBloomSettings::Internal_getintensity);
		metaData.scriptClass->addInternalCall("Internal_setintensity", (void*)&ScriptBloomSettings::Internal_setintensity);
		metaData.scriptClass->addInternalCall("Internal_gettint", (void*)&ScriptBloomSettings::Internal_gettint);
		metaData.scriptClass->addInternalCall("Internal_settint", (void*)&ScriptBloomSettings::Internal_settint);

	}

	MonoObject* ScriptBloomSettings::create(const SPtr<BloomSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
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
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setenabled(ScriptBloomSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	uint32_t ScriptBloomSettings::Internal_getquality(ScriptBloomSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setquality(ScriptBloomSettings* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->quality = value;
	}

	float ScriptBloomSettings::Internal_getthreshold(ScriptBloomSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setthreshold(ScriptBloomSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->threshold = value;
	}

	float ScriptBloomSettings::Internal_getintensity(ScriptBloomSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::Internal_setintensity(ScriptBloomSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->intensity = value;
	}

	void ScriptBloomSettings::Internal_gettint(ScriptBloomSettings* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->getInternal()->tint;

		*__output = tmp__output;


	}

	void ScriptBloomSettings::Internal_settint(ScriptBloomSettings* thisPtr, Color* value)
	{
		thisPtr->getInternal()->tint = *value;
	}
}
