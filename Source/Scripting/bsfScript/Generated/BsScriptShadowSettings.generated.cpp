//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		new (B3DAllocate<ScriptShadowSettings>()) ScriptShadowSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptShadowSettings::InternalShadowSettings(MonoObject* managedInstance)
	{
		SPtr<ShadowSettings> nativeObject = B3DMakeShared<ShadowSettings>();
		new (B3DAllocate<ScriptShadowSettings>())ScriptShadowSettings(managedInstance, nativeObject);
	}

	float ScriptShadowSettings::InternalGetDirectionalShadowDistance(ScriptShadowSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->DirectionalShadowDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetDirectionalShadowDistance(ScriptShadowSettings* self, float value)
	{
		self->GetInternal()->DirectionalShadowDistance = value;
	}

	uint32_t ScriptShadowSettings::InternalGetNumCascades(ScriptShadowSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->NumCascades;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetNumCascades(ScriptShadowSettings* self, uint32_t value)
	{
		self->GetInternal()->NumCascades = value;
	}

	float ScriptShadowSettings::InternalGetCascadeDistributionExponent(ScriptShadowSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->CascadeDistributionExponent;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetCascadeDistributionExponent(ScriptShadowSettings* self, float value)
	{
		self->GetInternal()->CascadeDistributionExponent = value;
	}

	uint32_t ScriptShadowSettings::InternalGetShadowFilteringQuality(ScriptShadowSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->ShadowFilteringQuality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShadowSettings::InternalSetShadowFilteringQuality(ScriptShadowSettings* self, uint32_t value)
	{
		self->GetInternal()->ShadowFilteringQuality = value;
	}
}
