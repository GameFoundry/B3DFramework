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
		metaData.scriptClass->AddInternalCall("Internal_AmbientOcclusionSettings", (void*)&ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptAmbientOcclusionSettings::InternalGetenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptAmbientOcclusionSettings::InternalSetenabled);
		metaData.scriptClass->AddInternalCall("Internal_getradius", (void*)&ScriptAmbientOcclusionSettings::InternalGetradius);
		metaData.scriptClass->AddInternalCall("Internal_setradius", (void*)&ScriptAmbientOcclusionSettings::InternalSetradius);
		metaData.scriptClass->AddInternalCall("Internal_getbias", (void*)&ScriptAmbientOcclusionSettings::InternalGetbias);
		metaData.scriptClass->AddInternalCall("Internal_setbias", (void*)&ScriptAmbientOcclusionSettings::InternalSetbias);
		metaData.scriptClass->AddInternalCall("Internal_getfadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalGetfadeDistance);
		metaData.scriptClass->AddInternalCall("Internal_setfadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalSetfadeDistance);
		metaData.scriptClass->AddInternalCall("Internal_getfadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalGetfadeRange);
		metaData.scriptClass->AddInternalCall("Internal_setfadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalSetfadeRange);
		metaData.scriptClass->AddInternalCall("Internal_getintensity", (void*)&ScriptAmbientOcclusionSettings::InternalGetintensity);
		metaData.scriptClass->AddInternalCall("Internal_setintensity", (void*)&ScriptAmbientOcclusionSettings::InternalSetintensity);
		metaData.scriptClass->AddInternalCall("Internal_getpower", (void*)&ScriptAmbientOcclusionSettings::InternalGetpower);
		metaData.scriptClass->AddInternalCall("Internal_setpower", (void*)&ScriptAmbientOcclusionSettings::InternalSetpower);
		metaData.scriptClass->AddInternalCall("Internal_getquality", (void*)&ScriptAmbientOcclusionSettings::InternalGetquality);
		metaData.scriptClass->AddInternalCall("Internal_setquality", (void*)&ScriptAmbientOcclusionSettings::InternalSetquality);

	}

	MonoObject* ScriptAmbientOcclusionSettings::Create(const SPtr<AmbientOcclusionSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptAmbientOcclusionSettings>()) ScriptAmbientOcclusionSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings(MonoObject* managedInstance)
	{
		SPtr<AmbientOcclusionSettings> instance = bs_shared_ptr_new<AmbientOcclusionSettings>();
		new (bs_alloc<ScriptAmbientOcclusionSettings>())ScriptAmbientOcclusionSettings(managedInstance, instance);
	}

	bool ScriptAmbientOcclusionSettings::InternalGetenabled(ScriptAmbientOcclusionSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetenabled(ScriptAmbientOcclusionSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetradius(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->radius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetradius(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->radius = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetbias(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->bias;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetbias(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->bias = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetfadeDistance(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->fadeDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetfadeDistance(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->fadeDistance = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetfadeRange(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->fadeRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetfadeRange(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->fadeRange = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetintensity(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetintensity(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->intensity = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetpower(ScriptAmbientOcclusionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->power;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetpower(ScriptAmbientOcclusionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->power = value;
	}

	uint32_t ScriptAmbientOcclusionSettings::InternalGetquality(ScriptAmbientOcclusionSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetquality(ScriptAmbientOcclusionSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->quality = value;
	}
}
