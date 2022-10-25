//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShadowSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptShadowSettings::ScriptShadowSettings(MonoObject* managedInstance, const SPtr<ShadowSettings>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptShadowSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ShadowSettings", (void*)&ScriptShadowSettings::InternalShadowSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetDirectionalShadowDistance", (void*)&ScriptShadowSettings::InternalGetDirectionalShadowDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetDirectionalShadowDistance", (void*)&ScriptShadowSettings::InternalSetDirectionalShadowDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumCascades", (void*)&ScriptShadowSettings::InternalGetNumCascades);
		metaData.ScriptClass->AddInternalCall("Internal_SetNumCascades", (void*)&ScriptShadowSettings::InternalSetNumCascades);
		metaData.ScriptClass->AddInternalCall("Internal_GetCascadeDistributionExponent", (void*)&ScriptShadowSettings::InternalGetCascadeDistributionExponent);
		metaData.ScriptClass->AddInternalCall("Internal_SetCascadeDistributionExponent", (void*)&ScriptShadowSettings::InternalSetCascadeDistributionExponent);
		metaData.ScriptClass->AddInternalCall("Internal_GetShadowFilteringQuality", (void*)&ScriptShadowSettings::InternalGetShadowFilteringQuality);
		metaData.ScriptClass->AddInternalCall("Internal_SetShadowFilteringQuality", (void*)&ScriptShadowSettings::InternalSetShadowFilteringQuality);
	}

	MonoObject* ScriptShadowSettings::Create(const SPtr<ShadowSettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptShadowSettings>()) ScriptShadowSettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptShadowSettings::InternalShadowSettings(MonoObject* managedInstance)
	{
		SPtr<ShadowSettings> instance = bs_shared_ptr_new<ShadowSettings>();
		new(bs_alloc<ScriptShadowSettings>()) ScriptShadowSettings(managedInstance, instance);
	}

	float ScriptShadowSettings::InternalGetDirectionalShadowDistance(ScriptShadowSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->DirectionalShadowDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetDirectionalShadowDistance(ScriptShadowSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->DirectionalShadowDistance = value;
	}

	uint32_t ScriptShadowSettings::InternalGetNumCascades(ScriptShadowSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->NumCascades;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetNumCascades(ScriptShadowSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->NumCascades = value;
	}

	float ScriptShadowSettings::InternalGetCascadeDistributionExponent(ScriptShadowSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->CascadeDistributionExponent;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetCascadeDistributionExponent(ScriptShadowSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->CascadeDistributionExponent = value;
	}

	uint32_t ScriptShadowSettings::InternalGetShadowFilteringQuality(ScriptShadowSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->ShadowFilteringQuality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetShadowFilteringQuality(ScriptShadowSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->ShadowFilteringQuality = value;
	}
} // namespace bs
