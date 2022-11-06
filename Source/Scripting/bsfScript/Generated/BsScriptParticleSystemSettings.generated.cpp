//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleSystemSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptParticleSystemSettings::ScriptParticleSystemSettings(MonoObject* managedInstance, const SPtr<ParticleSystemSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleSystemSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptParticleSystemSettings::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptParticleSystemSettings::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptParticleSystemSettings::InternalGetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptParticleSystemSettings::InternalSetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_GetSimulationSpace", (void*)&ScriptParticleSystemSettings::InternalGetSimulationSpace);
		metaData.ScriptClass->AddInternalCall("Internal_SetSimulationSpace", (void*)&ScriptParticleSystemSettings::InternalSetSimulationSpace);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrientation", (void*)&ScriptParticleSystemSettings::InternalGetOrientation);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrientation", (void*)&ScriptParticleSystemSettings::InternalSetOrientation);
		metaData.ScriptClass->AddInternalCall("Internal_GetDuration", (void*)&ScriptParticleSystemSettings::InternalGetDuration);
		metaData.ScriptClass->AddInternalCall("Internal_SetDuration", (void*)&ScriptParticleSystemSettings::InternalSetDuration);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsLooping", (void*)&ScriptParticleSystemSettings::InternalGetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsLooping", (void*)&ScriptParticleSystemSettings::InternalSetIsLooping);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxParticles", (void*)&ScriptParticleSystemSettings::InternalGetMaxParticles);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxParticles", (void*)&ScriptParticleSystemSettings::InternalSetMaxParticles);
		metaData.ScriptClass->AddInternalCall("Internal_GetGpuSimulation", (void*)&ScriptParticleSystemSettings::InternalGetGpuSimulation);
		metaData.ScriptClass->AddInternalCall("Internal_SetGpuSimulation", (void*)&ScriptParticleSystemSettings::InternalSetGpuSimulation);
		metaData.ScriptClass->AddInternalCall("Internal_GetRenderMode", (void*)&ScriptParticleSystemSettings::InternalGetRenderMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetRenderMode", (void*)&ScriptParticleSystemSettings::InternalSetRenderMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrientationLockY", (void*)&ScriptParticleSystemSettings::InternalGetOrientationLockY);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrientationLockY", (void*)&ScriptParticleSystemSettings::InternalSetOrientationLockY);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrientationPlaneNormal", (void*)&ScriptParticleSystemSettings::InternalGetOrientationPlaneNormal);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrientationPlaneNormal", (void*)&ScriptParticleSystemSettings::InternalSetOrientationPlaneNormal);
		metaData.ScriptClass->AddInternalCall("Internal_GetSortMode", (void*)&ScriptParticleSystemSettings::InternalGetSortMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetSortMode", (void*)&ScriptParticleSystemSettings::InternalSetSortMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseAutomaticSeed", (void*)&ScriptParticleSystemSettings::InternalGetUseAutomaticSeed);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseAutomaticSeed", (void*)&ScriptParticleSystemSettings::InternalSetUseAutomaticSeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetManualSeed", (void*)&ScriptParticleSystemSettings::InternalGetManualSeed);
		metaData.ScriptClass->AddInternalCall("Internal_SetManualSeed", (void*)&ScriptParticleSystemSettings::InternalSetManualSeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseAutomaticBounds", (void*)&ScriptParticleSystemSettings::InternalGetUseAutomaticBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseAutomaticBounds", (void*)&ScriptParticleSystemSettings::InternalSetUseAutomaticBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetCustomBounds", (void*)&ScriptParticleSystemSettings::InternalGetCustomBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetCustomBounds", (void*)&ScriptParticleSystemSettings::InternalSetCustomBounds);

	}

	MonoObject* ScriptParticleSystemSettings::Create(const SPtr<ParticleSystemSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleSystemSettings>()) ScriptParticleSystemSettings(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptParticleSystemSettings::InternalGetMaterial(ScriptParticleSystemSettings* thisPtr)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetInternal()->Material;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaterial(ScriptParticleSystemSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Material> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Material>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->Material = tmpvalue;
	}

	MonoObject* ScriptParticleSystemSettings::InternalGetMesh(ScriptParticleSystemSettings* thisPtr)
	{
		ResourceHandle<Mesh> tmp__output;
		tmp__output = thisPtr->GetInternal()->Mesh;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMesh(ScriptParticleSystemSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Mesh> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::ToNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Mesh>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->Mesh = tmpvalue;
	}

	ParticleSimulationSpace ScriptParticleSystemSettings::InternalGetSimulationSpace(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleSimulationSpace tmp__output;
		tmp__output = thisPtr->GetInternal()->SimulationSpace;

		ParticleSimulationSpace __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSimulationSpace(ScriptParticleSystemSettings* thisPtr, ParticleSimulationSpace value)
	{
		thisPtr->GetInternal()->SimulationSpace = value;
	}

	ParticleOrientation ScriptParticleSystemSettings::InternalGetOrientation(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleOrientation tmp__output;
		tmp__output = thisPtr->GetInternal()->Orientation;

		ParticleOrientation __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientation(ScriptParticleSystemSettings* thisPtr, ParticleOrientation value)
	{
		thisPtr->GetInternal()->Orientation = value;
	}

	float ScriptParticleSystemSettings::InternalGetDuration(ScriptParticleSystemSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Duration;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetDuration(ScriptParticleSystemSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Duration = value;
	}

	bool ScriptParticleSystemSettings::InternalGetIsLooping(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->IsLooping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetIsLooping(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->IsLooping = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetMaxParticles(ScriptParticleSystemSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->MaxParticles;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaxParticles(ScriptParticleSystemSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->MaxParticles = value;
	}

	bool ScriptParticleSystemSettings::InternalGetGpuSimulation(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->GpuSimulation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetGpuSimulation(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->GpuSimulation = value;
	}

	ParticleRenderMode ScriptParticleSystemSettings::InternalGetRenderMode(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleRenderMode tmp__output;
		tmp__output = thisPtr->GetInternal()->RenderMode;

		ParticleRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetRenderMode(ScriptParticleSystemSettings* thisPtr, ParticleRenderMode value)
	{
		thisPtr->GetInternal()->RenderMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetOrientationLockY(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->OrientationLockY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientationLockY(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->OrientationLockY = value;
	}

	void ScriptParticleSystemSettings::InternalGetOrientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->OrientationPlaneNormal;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::InternalSetOrientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->OrientationPlaneNormal = *value;
	}

	ParticleSortMode ScriptParticleSystemSettings::InternalGetSortMode(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleSortMode tmp__output;
		tmp__output = thisPtr->GetInternal()->SortMode;

		ParticleSortMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSortMode(ScriptParticleSystemSettings* thisPtr, ParticleSortMode value)
	{
		thisPtr->GetInternal()->SortMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticSeed(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->UseAutomaticSeed;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticSeed(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->UseAutomaticSeed = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetManualSeed(ScriptParticleSystemSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->ManualSeed;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetManualSeed(ScriptParticleSystemSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->ManualSeed = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticBounds(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->UseAutomaticBounds;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticBounds(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->UseAutomaticBounds = value;
	}

	void ScriptParticleSystemSettings::InternalGetCustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetInternal()->CustomBounds;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::InternalSetCustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* value)
	{
		thisPtr->GetInternal()->CustomBounds = *value;
	}
}
