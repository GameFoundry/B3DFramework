//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceReflectionsSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptScreenSpaceReflectionsSettings::ScriptScreenSpaceReflectionsSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceReflectionsSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptScreenSpaceReflectionsSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_ScreenSpaceReflectionsSettings", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_ScreenSpaceReflectionsSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_getquality", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_getquality);
		metaData.scriptClass->AddInternalCall("Internal_setquality", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_setquality);
		metaData.scriptClass->AddInternalCall("Internal_getintensity", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_getintensity);
		metaData.scriptClass->AddInternalCall("Internal_setintensity", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_setintensity);
		metaData.scriptClass->AddInternalCall("Internal_getmaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_getmaxRoughness);
		metaData.scriptClass->AddInternalCall("Internal_setmaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::Internal_setmaxRoughness);

	}

	MonoObject* ScriptScreenSpaceReflectionsSettings::create(const SPtr<ScreenSpaceReflectionsSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptScreenSpaceReflectionsSettings>()) ScriptScreenSpaceReflectionsSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptScreenSpaceReflectionsSettings::Internal_ScreenSpaceReflectionsSettings(MonoObject* managedInstance)
	{
		SPtr<ScreenSpaceReflectionsSettings> instance = bs_shared_ptr_new<ScreenSpaceReflectionsSettings>();
		new (bs_alloc<ScriptScreenSpaceReflectionsSettings>())ScriptScreenSpaceReflectionsSettings(managedInstance, instance);
	}

	bool ScriptScreenSpaceReflectionsSettings::Internal_getenabled(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::Internal_setenabled(ScriptScreenSpaceReflectionsSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	uint32_t ScriptScreenSpaceReflectionsSettings::Internal_getquality(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::Internal_setquality(ScriptScreenSpaceReflectionsSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->quality = value;
	}

	float ScriptScreenSpaceReflectionsSettings::Internal_getintensity(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::Internal_setintensity(ScriptScreenSpaceReflectionsSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->intensity = value;
	}

	float ScriptScreenSpaceReflectionsSettings::Internal_getmaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->maxRoughness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::Internal_setmaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->maxRoughness = value;
	}
}
