//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleVectorFieldSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTDistribution.generated.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptParticleVectorFieldSettings::ScriptParticleVectorFieldSettings(MonoObject* managedInstance, const SPtr<ParticleVectorFieldSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleVectorFieldSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getvectorField", (void*)&ScriptParticleVectorFieldSettings::Internal_getvectorField);
		metaData.scriptClass->AddInternalCall("Internal_setvectorField", (void*)&ScriptParticleVectorFieldSettings::Internal_setvectorField);
		metaData.scriptClass->AddInternalCall("Internal_getintensity", (void*)&ScriptParticleVectorFieldSettings::Internal_getintensity);
		metaData.scriptClass->AddInternalCall("Internal_setintensity", (void*)&ScriptParticleVectorFieldSettings::Internal_setintensity);
		metaData.scriptClass->AddInternalCall("Internal_gettightness", (void*)&ScriptParticleVectorFieldSettings::Internal_gettightness);
		metaData.scriptClass->AddInternalCall("Internal_settightness", (void*)&ScriptParticleVectorFieldSettings::Internal_settightness);
		metaData.scriptClass->AddInternalCall("Internal_getscale", (void*)&ScriptParticleVectorFieldSettings::Internal_getscale);
		metaData.scriptClass->AddInternalCall("Internal_setscale", (void*)&ScriptParticleVectorFieldSettings::Internal_setscale);
		metaData.scriptClass->AddInternalCall("Internal_getoffset", (void*)&ScriptParticleVectorFieldSettings::Internal_getoffset);
		metaData.scriptClass->AddInternalCall("Internal_setoffset", (void*)&ScriptParticleVectorFieldSettings::Internal_setoffset);
		metaData.scriptClass->AddInternalCall("Internal_getrotation", (void*)&ScriptParticleVectorFieldSettings::Internal_getrotation);
		metaData.scriptClass->AddInternalCall("Internal_setrotation", (void*)&ScriptParticleVectorFieldSettings::Internal_setrotation);
		metaData.scriptClass->AddInternalCall("Internal_getrotationRate", (void*)&ScriptParticleVectorFieldSettings::Internal_getrotationRate);
		metaData.scriptClass->AddInternalCall("Internal_setrotationRate", (void*)&ScriptParticleVectorFieldSettings::Internal_setrotationRate);
		metaData.scriptClass->AddInternalCall("Internal_gettilingX", (void*)&ScriptParticleVectorFieldSettings::Internal_gettilingX);
		metaData.scriptClass->AddInternalCall("Internal_settilingX", (void*)&ScriptParticleVectorFieldSettings::Internal_settilingX);
		metaData.scriptClass->AddInternalCall("Internal_gettilingY", (void*)&ScriptParticleVectorFieldSettings::Internal_gettilingY);
		metaData.scriptClass->AddInternalCall("Internal_settilingY", (void*)&ScriptParticleVectorFieldSettings::Internal_settilingY);
		metaData.scriptClass->AddInternalCall("Internal_gettilingZ", (void*)&ScriptParticleVectorFieldSettings::Internal_gettilingZ);
		metaData.scriptClass->AddInternalCall("Internal_settilingZ", (void*)&ScriptParticleVectorFieldSettings::Internal_settilingZ);

	}

	MonoObject* ScriptParticleVectorFieldSettings::create(const SPtr<ParticleVectorFieldSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleVectorFieldSettings>()) ScriptParticleVectorFieldSettings(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptParticleVectorFieldSettings::Internal_getvectorField(ScriptParticleVectorFieldSettings* thisPtr)
	{
		ResourceHandle<VectorField> tmp__output;
		tmp__output = thisPtr->GetInternal()->vectorField;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_setvectorField(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<VectorField> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<VectorField>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->vectorField = tmpvalue;
	}

	float ScriptParticleVectorFieldSettings::Internal_getintensity(ScriptParticleVectorFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_setintensity(ScriptParticleVectorFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->intensity = value;
	}

	float ScriptParticleVectorFieldSettings::Internal_gettightness(ScriptParticleVectorFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->tightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_settightness(ScriptParticleVectorFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->tightness = value;
	}

	void ScriptParticleVectorFieldSettings::Internal_getscale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->scale;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::Internal_setscale(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->scale = *value;
	}

	void ScriptParticleVectorFieldSettings::Internal_getoffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->offset;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::Internal_setoffset(ScriptParticleVectorFieldSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->offset = *value;
	}

	void ScriptParticleVectorFieldSettings::Internal_getrotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetInternal()->rotation;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::Internal_setrotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* value)
	{
		thisPtr->GetInternal()->rotation = *value;
	}

	MonoObject* ScriptParticleVectorFieldSettings::Internal_getrotationRate(ScriptParticleVectorFieldSettings* thisPtr)
	{
		SPtr<TDistribution<Vector3>> tmp__output = bs_shared_ptr_new<TDistribution<Vector3>>();
		*tmp__output = thisPtr->GetInternal()->rotationRate;

		MonoObject* __output;
		__output = ScriptTDistributionVector3::create(tmp__output);

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_setrotationRate(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<Vector3>> tmpvalue;
		ScriptTDistributionVector3* scriptvalue;
		scriptvalue = ScriptTDistributionVector3::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->rotationRate = *tmpvalue;
	}

	bool ScriptParticleVectorFieldSettings::Internal_gettilingX(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->tilingX;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_settilingX(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->tilingX = value;
	}

	bool ScriptParticleVectorFieldSettings::Internal_gettilingY(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->tilingY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_settilingY(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->tilingY = value;
	}

	bool ScriptParticleVectorFieldSettings::Internal_gettilingZ(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->tilingZ;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::Internal_settilingZ(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->tilingZ = value;
	}
}
