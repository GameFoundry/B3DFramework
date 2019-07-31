//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleGpuSimulationSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleDepthCollisionSettings.generated.h"
#include "BsScriptParticleVectorFieldSettings.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTColorDistribution.generated.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleGpuSimulationSettings::ScriptParticleGpuSimulationSettings(MonoObject* managedInstance, const SPtr<ParticleGpuSimulationSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleGpuSimulationSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getvectorField", (void*)&ScriptParticleGpuSimulationSettings::Internal_getvectorField);
		metaData.scriptClass->addInternalCall("Internal_setvectorField", (void*)&ScriptParticleGpuSimulationSettings::Internal_setvectorField);
		metaData.scriptClass->addInternalCall("Internal_getcolorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::Internal_getcolorOverLifetime);
		metaData.scriptClass->addInternalCall("Internal_setcolorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::Internal_setcolorOverLifetime);
		metaData.scriptClass->addInternalCall("Internal_getsizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::Internal_getsizeScaleOverLifetime);
		metaData.scriptClass->addInternalCall("Internal_setsizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::Internal_setsizeScaleOverLifetime);
		metaData.scriptClass->addInternalCall("Internal_getacceleration", (void*)&ScriptParticleGpuSimulationSettings::Internal_getacceleration);
		metaData.scriptClass->addInternalCall("Internal_setacceleration", (void*)&ScriptParticleGpuSimulationSettings::Internal_setacceleration);
		metaData.scriptClass->addInternalCall("Internal_getdrag", (void*)&ScriptParticleGpuSimulationSettings::Internal_getdrag);
		metaData.scriptClass->addInternalCall("Internal_setdrag", (void*)&ScriptParticleGpuSimulationSettings::Internal_setdrag);
		metaData.scriptClass->addInternalCall("Internal_getdepthCollision", (void*)&ScriptParticleGpuSimulationSettings::Internal_getdepthCollision);
		metaData.scriptClass->addInternalCall("Internal_setdepthCollision", (void*)&ScriptParticleGpuSimulationSettings::Internal_setdepthCollision);

	}

	MonoObject* ScriptParticleGpuSimulationSettings::create(const SPtr<ParticleGpuSimulationSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleGpuSimulationSettings>()) ScriptParticleGpuSimulationSettings(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptParticleGpuSimulationSettings::Internal_getvectorField(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<ParticleVectorFieldSettings> tmp__output = bs_shared_ptr_new<ParticleVectorFieldSettings>();
		*tmp__output = thisPtr->getInternal()->vectorField;

		MonoObject* __output;
		__output = ScriptParticleVectorFieldSettings::create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::Internal_setvectorField(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<ParticleVectorFieldSettings> tmpvalue;
		ScriptParticleVectorFieldSettings* scriptvalue;
		scriptvalue = ScriptParticleVectorFieldSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->vectorField = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::Internal_getcolorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<TColorDistribution<ColorGradient>> tmp__output = bs_shared_ptr_new<TColorDistribution<ColorGradient>>();
		*tmp__output = thisPtr->getInternal()->colorOverLifetime;

		MonoObject* __output;
		__output = ScriptTColorDistributionColorGradient::create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::Internal_setcolorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptTColorDistributionColorGradient* scriptvalue;
		scriptvalue = ScriptTColorDistributionColorGradient::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->colorOverLifetime = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::Internal_getsizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<TDistribution<Vector2>> tmp__output = bs_shared_ptr_new<TDistribution<Vector2>>();
		*tmp__output = thisPtr->getInternal()->sizeScaleOverLifetime;

		MonoObject* __output;
		__output = ScriptTDistributionVector2::create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::Internal_setsizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<Vector2>> tmpvalue;
		ScriptTDistributionVector2* scriptvalue;
		scriptvalue = ScriptTDistributionVector2::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->sizeScaleOverLifetime = *tmpvalue;
	}

	void ScriptParticleGpuSimulationSettings::Internal_getacceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->acceleration;

		*__output = tmp__output;


	}

	void ScriptParticleGpuSimulationSettings::Internal_setacceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* value)
	{
		thisPtr->getInternal()->acceleration = *value;
	}

	float ScriptParticleGpuSimulationSettings::Internal_getdrag(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->drag;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::Internal_setdrag(ScriptParticleGpuSimulationSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->drag = value;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::Internal_getdepthCollision(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<ParticleDepthCollisionSettings> tmp__output = bs_shared_ptr_new<ParticleDepthCollisionSettings>();
		*tmp__output = thisPtr->getInternal()->depthCollision;

		MonoObject* __output;
		__output = ScriptParticleDepthCollisionSettings::create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::Internal_setdepthCollision(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<ParticleDepthCollisionSettings> tmpvalue;
		ScriptParticleDepthCollisionSettings* scriptvalue;
		scriptvalue = ScriptParticleDepthCollisionSettings::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getInternal();
		thisPtr->getInternal()->depthCollision = *tmpvalue;
	}
}
