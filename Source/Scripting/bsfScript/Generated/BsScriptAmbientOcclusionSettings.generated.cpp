//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAmbientOcclusionSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAmbientOcclusionSettings::ScriptAmbientOcclusionSettings(MonoObject* managedInstance, const SPtr<AmbientOcclusionSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptAmbientOcclusionSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_AmbientOcclusionSettings", (void*)&ScriptAmbientOcclusionSettings::Internal_AmbientOcclusionSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptAmbientOcclusionSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptAmbientOcclusionSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getradius", (void*)&ScriptAmbientOcclusionSettings::Internal_getradius);
		metaData.scriptClass->addInternalCall("Internal_setradius", (void*)&ScriptAmbientOcclusionSettings::Internal_setradius);
		metaData.scriptClass->addInternalCall("Internal_getbias", (void*)&ScriptAmbientOcclusionSettings::Internal_getbias);
		metaData.scriptClass->addInternalCall("Internal_setbias", (void*)&ScriptAmbientOcclusionSettings::Internal_setbias);
		metaData.scriptClass->addInternalCall("Internal_getfadeDistance", (void*)&ScriptAmbientOcclusionSettings::Internal_getfadeDistance);
		metaData.scriptClass->addInternalCall("Internal_setfadeDistance", (void*)&ScriptAmbientOcclusionSettings::Internal_setfadeDistance);
		metaData.scriptClass->addInternalCall("Internal_getfadeRange", (void*)&ScriptAmbientOcclusionSettings::Internal_getfadeRange);
		metaData.scriptClass->addInternalCall("Internal_setfadeRange", (void*)&ScriptAmbientOcclusionSettings::Internal_setfadeRange);
		metaData.scriptClass->addInternalCall("Internal_getintensity", (void*)&ScriptAmbientOcclusionSettings::Internal_getintensity);
		metaData.scriptClass->addInternalCall("Internal_setintensity", (void*)&ScriptAmbientOcclusionSettings::Internal_setintensity);
		metaData.scriptClass->addInternalCall("Internal_getpower", (void*)&ScriptAmbientOcclusionSettings::Internal_getpower);
		metaData.scriptClass->addInternalCall("Internal_setpower", (void*)&ScriptAmbientOcclusionSettings::Internal_setpower);
		metaData.scriptClass->addInternalCall("Internal_getquality", (void*)&ScriptAmbientOcclusionSettings::Internal_getquality);
		metaData.scriptClass->addInternalCall("Internal_setquality", (void*)&ScriptAmbientOcclusionSettings::Internal_setquality);

	}

	MonoObject* ScriptAmbientOcclusionSettings::create(const SPtr<AmbientOcclusionSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptAmbientOcclusionSettings>()) ScriptAmbientOcclusionSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptAmbientOcclusionSettings::Internal_AmbientOcclusionSettings(MonoObject* managedInstance)
	{
		SPtr<AmbientOcclusionSettings> instance = bs_shared_ptr_new<AmbientOcclusionSettings>();
		new (bs_alloc<ScriptAmbientOcclusionSettings>())ScriptAmbientOcclusionSettings(managedInstance, instance);
	}

	bool ScriptAmbientOcclusionSettings::Internal_getenabled(ScriptAmbientOcclusionSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setenabled(ScriptAmbientOcclusionSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getradius(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->radius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setradius(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->radius = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getbias(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->bias;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setbias(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->bias = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getfadeDistance(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->fadeDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setfadeDistance(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->fadeDistance = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getfadeRange(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->fadeRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setfadeRange(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->fadeRange = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getintensity(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setintensity(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->intensity = value;
	}

	float ScriptAmbientOcclusionSettings::Internal_getpower(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->power;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setpower(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->power = value;
	}

	uint32_t ScriptAmbientOcclusionSettings::Internal_getquality(ScriptAmbientOcclusionSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::Internal_setquality(ScriptAmbientOcclusionSettings* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->quality = value;
	}
}
