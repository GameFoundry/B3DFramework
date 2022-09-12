//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShadowSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShadowSettings::ScriptShadowSettings(MonoObject* managedInstance, const SPtr<ShadowSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptShadowSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_ShadowSettings", (void*)&ScriptShadowSettings::Internal_ShadowSettings);
		metaData.scriptClass->AddInternalCall("Internal_getdirectionalShadowDistance", (void*)&ScriptShadowSettings::Internal_getdirectionalShadowDistance);
		metaData.scriptClass->AddInternalCall("Internal_setdirectionalShadowDistance", (void*)&ScriptShadowSettings::Internal_setdirectionalShadowDistance);
		metaData.scriptClass->AddInternalCall("Internal_getnumCascades", (void*)&ScriptShadowSettings::Internal_getnumCascades);
		metaData.scriptClass->AddInternalCall("Internal_setnumCascades", (void*)&ScriptShadowSettings::Internal_setnumCascades);
		metaData.scriptClass->AddInternalCall("Internal_getcascadeDistributionExponent", (void*)&ScriptShadowSettings::Internal_getcascadeDistributionExponent);
		metaData.scriptClass->AddInternalCall("Internal_setcascadeDistributionExponent", (void*)&ScriptShadowSettings::Internal_setcascadeDistributionExponent);
		metaData.scriptClass->AddInternalCall("Internal_getshadowFilteringQuality", (void*)&ScriptShadowSettings::Internal_getshadowFilteringQuality);
		metaData.scriptClass->AddInternalCall("Internal_setshadowFilteringQuality", (void*)&ScriptShadowSettings::Internal_setshadowFilteringQuality);

	}

	MonoObject* ScriptShadowSettings::create(const SPtr<ShadowSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptShadowSettings>()) ScriptShadowSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptShadowSettings::Internal_ShadowSettings(MonoObject* managedInstance)
	{
		SPtr<ShadowSettings> instance = bs_shared_ptr_new<ShadowSettings>();
		new (bs_alloc<ScriptShadowSettings>())ScriptShadowSettings(managedInstance, instance);
	}

	float ScriptShadowSettings::Internal_getdirectionalShadowDistance(ScriptShadowSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->directionalShadowDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::Internal_setdirectionalShadowDistance(ScriptShadowSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->directionalShadowDistance = value;
	}

	uint32_t ScriptShadowSettings::Internal_getnumCascades(ScriptShadowSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->numCascades;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::Internal_setnumCascades(ScriptShadowSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->numCascades = value;
	}

	float ScriptShadowSettings::Internal_getcascadeDistributionExponent(ScriptShadowSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->cascadeDistributionExponent;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::Internal_setcascadeDistributionExponent(ScriptShadowSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->cascadeDistributionExponent = value;
	}

	uint32_t ScriptShadowSettings::Internal_getshadowFilteringQuality(ScriptShadowSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->shadowFilteringQuality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::Internal_setshadowFilteringQuality(ScriptShadowSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->shadowFilteringQuality = value;
	}
}
