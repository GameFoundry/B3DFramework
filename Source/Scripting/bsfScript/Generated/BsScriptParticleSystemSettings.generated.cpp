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
	MonoObject* ScriptParticleSystemSettings::InternalGetMaterial(ScriptParticleSystemSettings* self)
	{
		TResourceHandle<Material> tmp__output;
		tmp__output = self->GetInternal()->Material;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaterial(ScriptParticleSystemSettings* self, MonoObject* value)
	{
		TResourceHandle<Material> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Material>(scriptObjectWrappervalue->GetHandle());
		self->GetInternal()->Material = tmpvalue;
	}

	MonoObject* ScriptParticleSystemSettings::InternalGetMesh(ScriptParticleSystemSettings* self)
	{
		TResourceHandle<Mesh> tmp__output;
		tmp__output = self->GetInternal()->Mesh;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMesh(ScriptParticleSystemSettings* self, MonoObject* value)
	{
		TResourceHandle<Mesh> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Mesh>(scriptObjectWrappervalue->GetHandle());
		self->GetInternal()->Mesh = tmpvalue;
	}

	ParticleSimulationSpace ScriptParticleSystemSettings::InternalGetSimulationSpace(ScriptParticleSystemSettings* self)
	{
		ParticleSimulationSpace tmp__output;
		tmp__output = self->GetInternal()->SimulationSpace;

		ParticleSimulationSpace __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSimulationSpace(ScriptParticleSystemSettings* self, ParticleSimulationSpace value)
	{
		self->GetInternal()->SimulationSpace = value;
	}

	ParticleOrientation ScriptParticleSystemSettings::InternalGetOrientation(ScriptParticleSystemSettings* self)
	{
		ParticleOrientation tmp__output;
		tmp__output = self->GetInternal()->Orientation;

		ParticleOrientation __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientation(ScriptParticleSystemSettings* self, ParticleOrientation value)
	{
		self->GetInternal()->Orientation = value;
	}

	float ScriptParticleSystemSettings::InternalGetDuration(ScriptParticleSystemSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Duration;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetDuration(ScriptParticleSystemSettings* self, float value)
	{
		self->GetInternal()->Duration = value;
	}

	bool ScriptParticleSystemSettings::InternalGetIsLooping(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->IsLooping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetIsLooping(ScriptParticleSystemSettings* self, bool value)
	{
		self->GetInternal()->IsLooping = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetMaxParticles(ScriptParticleSystemSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->MaxParticles;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaxParticles(ScriptParticleSystemSettings* self, uint32_t value)
	{
		self->GetInternal()->MaxParticles = value;
	}

	bool ScriptParticleSystemSettings::InternalGetGpuSimulation(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->GpuSimulation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetGpuSimulation(ScriptParticleSystemSettings* self, bool value)
	{
		self->GetInternal()->GpuSimulation = value;
	}

	ParticleRenderMode ScriptParticleSystemSettings::InternalGetRenderMode(ScriptParticleSystemSettings* self)
	{
		ParticleRenderMode tmp__output;
		tmp__output = self->GetInternal()->RenderMode;

		ParticleRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetRenderMode(ScriptParticleSystemSettings* self, ParticleRenderMode value)
	{
		self->GetInternal()->RenderMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetOrientationLockY(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->OrientationLockY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientationLockY(ScriptParticleSystemSettings* self, bool value)
	{
		self->GetInternal()->OrientationLockY = value;
	}

	void ScriptParticleSystemSettings::InternalGetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->OrientationPlaneNormal;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::InternalSetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->OrientationPlaneNormal = *value;
	}

	ParticleSortMode ScriptParticleSystemSettings::InternalGetSortMode(ScriptParticleSystemSettings* self)
	{
		ParticleSortMode tmp__output;
		tmp__output = self->GetInternal()->SortMode;

		ParticleSortMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSortMode(ScriptParticleSystemSettings* self, ParticleSortMode value)
	{
		self->GetInternal()->SortMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticSeed(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->UseAutomaticSeed;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticSeed(ScriptParticleSystemSettings* self, bool value)
	{
		self->GetInternal()->UseAutomaticSeed = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetManualSeed(ScriptParticleSystemSettings* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->ManualSeed;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetManualSeed(ScriptParticleSystemSettings* self, uint32_t value)
	{
		self->GetInternal()->ManualSeed = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticBounds(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->UseAutomaticBounds;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticBounds(ScriptParticleSystemSettings* self, bool value)
	{
		self->GetInternal()->UseAutomaticBounds = value;
	}

	void ScriptParticleSystemSettings::InternalGetCustomBounds(ScriptParticleSystemSettings* self, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = self->GetInternal()->CustomBounds;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::InternalSetCustomBounds(ScriptParticleSystemSettings* self, AABox* value)
	{
		self->GetInternal()->CustomBounds = *value;
	}
}
