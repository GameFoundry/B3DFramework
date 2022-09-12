//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_getmaterial", (void*)&ScriptParticleSystemSettings::Internal_getmaterial);
		metaData.scriptClass->AddInternalCall("Internal_setmaterial", (void*)&ScriptParticleSystemSettings::Internal_setmaterial);
		metaData.scriptClass->AddInternalCall("Internal_getmesh", (void*)&ScriptParticleSystemSettings::Internal_getmesh);
		metaData.scriptClass->AddInternalCall("Internal_setmesh", (void*)&ScriptParticleSystemSettings::Internal_setmesh);
		metaData.scriptClass->AddInternalCall("Internal_getsimulationSpace", (void*)&ScriptParticleSystemSettings::Internal_getsimulationSpace);
		metaData.scriptClass->AddInternalCall("Internal_setsimulationSpace", (void*)&ScriptParticleSystemSettings::Internal_setsimulationSpace);
		metaData.scriptClass->AddInternalCall("Internal_getorientation", (void*)&ScriptParticleSystemSettings::Internal_getorientation);
		metaData.scriptClass->AddInternalCall("Internal_setorientation", (void*)&ScriptParticleSystemSettings::Internal_setorientation);
		metaData.scriptClass->AddInternalCall("Internal_getduration", (void*)&ScriptParticleSystemSettings::Internal_getduration);
		metaData.scriptClass->AddInternalCall("Internal_setduration", (void*)&ScriptParticleSystemSettings::Internal_setduration);
		metaData.scriptClass->AddInternalCall("Internal_getisLooping", (void*)&ScriptParticleSystemSettings::Internal_getisLooping);
		metaData.scriptClass->AddInternalCall("Internal_setisLooping", (void*)&ScriptParticleSystemSettings::Internal_setisLooping);
		metaData.scriptClass->AddInternalCall("Internal_getmaxParticles", (void*)&ScriptParticleSystemSettings::Internal_getmaxParticles);
		metaData.scriptClass->AddInternalCall("Internal_setmaxParticles", (void*)&ScriptParticleSystemSettings::Internal_setmaxParticles);
		metaData.scriptClass->AddInternalCall("Internal_getgpuSimulation", (void*)&ScriptParticleSystemSettings::Internal_getgpuSimulation);
		metaData.scriptClass->AddInternalCall("Internal_setgpuSimulation", (void*)&ScriptParticleSystemSettings::Internal_setgpuSimulation);
		metaData.scriptClass->AddInternalCall("Internal_getrenderMode", (void*)&ScriptParticleSystemSettings::Internal_getrenderMode);
		metaData.scriptClass->AddInternalCall("Internal_setrenderMode", (void*)&ScriptParticleSystemSettings::Internal_setrenderMode);
		metaData.scriptClass->AddInternalCall("Internal_getorientationLockY", (void*)&ScriptParticleSystemSettings::Internal_getorientationLockY);
		metaData.scriptClass->AddInternalCall("Internal_setorientationLockY", (void*)&ScriptParticleSystemSettings::Internal_setorientationLockY);
		metaData.scriptClass->AddInternalCall("Internal_getorientationPlaneNormal", (void*)&ScriptParticleSystemSettings::Internal_getorientationPlaneNormal);
		metaData.scriptClass->AddInternalCall("Internal_setorientationPlaneNormal", (void*)&ScriptParticleSystemSettings::Internal_setorientationPlaneNormal);
		metaData.scriptClass->AddInternalCall("Internal_getsortMode", (void*)&ScriptParticleSystemSettings::Internal_getsortMode);
		metaData.scriptClass->AddInternalCall("Internal_setsortMode", (void*)&ScriptParticleSystemSettings::Internal_setsortMode);
		metaData.scriptClass->AddInternalCall("Internal_getuseAutomaticSeed", (void*)&ScriptParticleSystemSettings::Internal_getuseAutomaticSeed);
		metaData.scriptClass->AddInternalCall("Internal_setuseAutomaticSeed", (void*)&ScriptParticleSystemSettings::Internal_setuseAutomaticSeed);
		metaData.scriptClass->AddInternalCall("Internal_getmanualSeed", (void*)&ScriptParticleSystemSettings::Internal_getmanualSeed);
		metaData.scriptClass->AddInternalCall("Internal_setmanualSeed", (void*)&ScriptParticleSystemSettings::Internal_setmanualSeed);
		metaData.scriptClass->AddInternalCall("Internal_getuseAutomaticBounds", (void*)&ScriptParticleSystemSettings::Internal_getuseAutomaticBounds);
		metaData.scriptClass->AddInternalCall("Internal_setuseAutomaticBounds", (void*)&ScriptParticleSystemSettings::Internal_setuseAutomaticBounds);
		metaData.scriptClass->AddInternalCall("Internal_getcustomBounds", (void*)&ScriptParticleSystemSettings::Internal_getcustomBounds);
		metaData.scriptClass->AddInternalCall("Internal_setcustomBounds", (void*)&ScriptParticleSystemSettings::Internal_setcustomBounds);

	}

	MonoObject* ScriptParticleSystemSettings::create(const SPtr<ParticleSystemSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleSystemSettings>()) ScriptParticleSystemSettings(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptParticleSystemSettings::Internal_getmaterial(ScriptParticleSystemSettings* thisPtr)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetInternal()->material;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setmaterial(ScriptParticleSystemSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Material> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Material>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->material = tmpvalue;
	}

	MonoObject* ScriptParticleSystemSettings::Internal_getmesh(ScriptParticleSystemSettings* thisPtr)
	{
		ResourceHandle<Mesh> tmp__output;
		tmp__output = thisPtr->GetInternal()->mesh;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setmesh(ScriptParticleSystemSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Mesh> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Mesh>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->mesh = tmpvalue;
	}

	ParticleSimulationSpace ScriptParticleSystemSettings::Internal_getsimulationSpace(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleSimulationSpace tmp__output;
		tmp__output = thisPtr->GetInternal()->simulationSpace;

		ParticleSimulationSpace __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setsimulationSpace(ScriptParticleSystemSettings* thisPtr, ParticleSimulationSpace value)
	{
		thisPtr->GetInternal()->simulationSpace = value;
	}

	ParticleOrientation ScriptParticleSystemSettings::Internal_getorientation(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleOrientation tmp__output;
		tmp__output = thisPtr->GetInternal()->orientation;

		ParticleOrientation __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setorientation(ScriptParticleSystemSettings* thisPtr, ParticleOrientation value)
	{
		thisPtr->GetInternal()->orientation = value;
	}

	float ScriptParticleSystemSettings::Internal_getduration(ScriptParticleSystemSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->duration;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setduration(ScriptParticleSystemSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->duration = value;
	}

	bool ScriptParticleSystemSettings::Internal_getisLooping(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->isLooping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setisLooping(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->isLooping = value;
	}

	uint32_t ScriptParticleSystemSettings::Internal_getmaxParticles(ScriptParticleSystemSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->maxParticles;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setmaxParticles(ScriptParticleSystemSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->maxParticles = value;
	}

	bool ScriptParticleSystemSettings::Internal_getgpuSimulation(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->gpuSimulation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setgpuSimulation(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->gpuSimulation = value;
	}

	ParticleRenderMode ScriptParticleSystemSettings::Internal_getrenderMode(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleRenderMode tmp__output;
		tmp__output = thisPtr->GetInternal()->renderMode;

		ParticleRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setrenderMode(ScriptParticleSystemSettings* thisPtr, ParticleRenderMode value)
	{
		thisPtr->GetInternal()->renderMode = value;
	}

	bool ScriptParticleSystemSettings::Internal_getorientationLockY(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->orientationLockY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setorientationLockY(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->orientationLockY = value;
	}

	void ScriptParticleSystemSettings::Internal_getorientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->orientationPlaneNormal;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::Internal_setorientationPlaneNormal(ScriptParticleSystemSettings* thisPtr, Vector3* value)
	{
		thisPtr->GetInternal()->orientationPlaneNormal = *value;
	}

	ParticleSortMode ScriptParticleSystemSettings::Internal_getsortMode(ScriptParticleSystemSettings* thisPtr)
	{
		ParticleSortMode tmp__output;
		tmp__output = thisPtr->GetInternal()->sortMode;

		ParticleSortMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setsortMode(ScriptParticleSystemSettings* thisPtr, ParticleSortMode value)
	{
		thisPtr->GetInternal()->sortMode = value;
	}

	bool ScriptParticleSystemSettings::Internal_getuseAutomaticSeed(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->useAutomaticSeed;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setuseAutomaticSeed(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->useAutomaticSeed = value;
	}

	uint32_t ScriptParticleSystemSettings::Internal_getmanualSeed(ScriptParticleSystemSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->manualSeed;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setmanualSeed(ScriptParticleSystemSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->manualSeed = value;
	}

	bool ScriptParticleSystemSettings::Internal_getuseAutomaticBounds(ScriptParticleSystemSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->useAutomaticBounds;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::Internal_setuseAutomaticBounds(ScriptParticleSystemSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->useAutomaticBounds = value;
	}

	void ScriptParticleSystemSettings::Internal_getcustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetInternal()->customBounds;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::Internal_setcustomBounds(ScriptParticleSystemSettings* thisPtr, AABox* value)
	{
		thisPtr->GetInternal()->customBounds = *value;
	}
}
