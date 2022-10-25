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
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleGpuSimulationSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetVectorField", (void*)&ScriptParticleGpuSimulationSettings::InternalGetVectorField);
		metaData.ScriptClass->AddInternalCall("Internal_SetVectorField", (void*)&ScriptParticleGpuSimulationSettings::InternalSetVectorField);
		metaData.ScriptClass->AddInternalCall("Internal_GetColorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalGetColorOverLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_SetColorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalSetColorOverLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_GetSizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalGetSizeScaleOverLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_SetSizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalSetSizeScaleOverLifetime);
		metaData.ScriptClass->AddInternalCall("Internal_GetAcceleration", (void*)&ScriptParticleGpuSimulationSettings::InternalGetAcceleration);
		metaData.ScriptClass->AddInternalCall("Internal_SetAcceleration", (void*)&ScriptParticleGpuSimulationSettings::InternalSetAcceleration);
		metaData.ScriptClass->AddInternalCall("Internal_GetDrag", (void*)&ScriptParticleGpuSimulationSettings::InternalGetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_SetDrag", (void*)&ScriptParticleGpuSimulationSettings::InternalSetDrag);
		metaData.ScriptClass->AddInternalCall("Internal_GetDepthCollision", (void*)&ScriptParticleGpuSimulationSettings::InternalGetDepthCollision);
		metaData.ScriptClass->AddInternalCall("Internal_SetDepthCollision", (void*)&ScriptParticleGpuSimulationSettings::InternalSetDepthCollision);
	}

	MonoObject* ScriptParticleGpuSimulationSettings::Create(const SPtr<ParticleGpuSimulationSettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptParticleGpuSimulationSettings>()) ScriptParticleGpuSimulationSettings(managedInstance, value);
		return managedInstance;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetVectorField(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<ParticleVectorFieldSettings> tmp__output = bs_shared_ptr_new<ParticleVectorFieldSettings>();
		*tmp__output = thisPtr->GetInternal()->VectorField;

		MonoObject* __output;
		__output = ScriptParticleVectorFieldSettings::Create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetVectorField(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<ParticleVectorFieldSettings> tmpvalue;
		ScriptParticleVectorFieldSettings* scriptvalue;
		scriptvalue = ScriptParticleVectorFieldSettings::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->VectorField = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetColorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<TColorDistribution<ColorGradient>> tmp__output = bs_shared_ptr_new<TColorDistribution<ColorGradient>>();
		*tmp__output = thisPtr->GetInternal()->ColorOverLifetime;

		MonoObject* __output;
		__output = ScriptTColorDistributionColorGradient::Create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetColorOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptTColorDistributionColorGradient* scriptvalue;
		scriptvalue = ScriptTColorDistributionColorGradient::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->ColorOverLifetime = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<TDistribution<Vector2>> tmp__output = bs_shared_ptr_new<TDistribution<Vector2>>();
		*tmp__output = thisPtr->GetInternal()->SizeScaleOverLifetime;

		MonoObject* __output;
		__output = ScriptTDistributionVector2::Create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<Vector2>> tmpvalue;
		ScriptTDistributionVector2* scriptvalue;
		scriptvalue = ScriptTDistributionVector2::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->SizeScaleOverLifetime = *tmpvalue;
	}

	void ScriptParticleGpuSimulationSettings::InternalGetAcceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->Acceleration;

		*__output = tmp__output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetAcceleration(ScriptParticleGpuSimulationSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->Acceleration = *value;
	}

	float ScriptParticleGpuSimulationSettings::InternalGetDrag(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Drag;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetDrag(ScriptParticleGpuSimulationSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Drag = value;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetDepthCollision(ScriptParticleGpuSimulationSettings* thisPtr)
	{
		SPtr<ParticleDepthCollisionSettings> tmp__output = bs_shared_ptr_new<ParticleDepthCollisionSettings>();
		*tmp__output = thisPtr->GetInternal()->DepthCollision;

		MonoObject* __output;
		__output = ScriptParticleDepthCollisionSettings::Create(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetDepthCollision(ScriptParticleGpuSimulationSettings* thisPtr, MonoObject* value)
	{
		SPtr<ParticleDepthCollisionSettings> tmpvalue;
		ScriptParticleDepthCollisionSettings* scriptvalue;
		scriptvalue = ScriptParticleDepthCollisionSettings::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->GetInternal();
		thisPtr->GetInternal()->DepthCollision = *tmpvalue;
	}
} // namespace bs
