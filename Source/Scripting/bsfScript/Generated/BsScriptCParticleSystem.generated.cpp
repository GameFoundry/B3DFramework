//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCParticleSystem.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCParticleSystem.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptParticleSystemSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleOrbit.generated.h"
#include "BsScriptParticleGpuSimulationSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleGravity.generated.h"
#include "BsScriptParticleEmitter.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleTextureAnimation.generated.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleForce.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleVelocity.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleColor.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleSize.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleRotation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleCollisions.generated.h"

namespace bs
{
	ScriptCParticleSystem::ScriptCParticleSystem(MonoObject* managedInstance, const GameObjectHandle<CParticleSystem>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCParticleSystem::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setSettings", (void*)&ScriptCParticleSystem::Internal_setSettings);
		metaData.scriptClass->AddInternalCall("Internal_getSettings", (void*)&ScriptCParticleSystem::Internal_getSettings);
		metaData.scriptClass->AddInternalCall("Internal_setGpuSimulationSettings", (void*)&ScriptCParticleSystem::Internal_setGpuSimulationSettings);
		metaData.scriptClass->AddInternalCall("Internal_getGpuSimulationSettings", (void*)&ScriptCParticleSystem::Internal_getGpuSimulationSettings);
		metaData.scriptClass->AddInternalCall("Internal_setEmitters", (void*)&ScriptCParticleSystem::Internal_setEmitters);
		metaData.scriptClass->AddInternalCall("Internal_getEmitters", (void*)&ScriptCParticleSystem::Internal_getEmitters);
		metaData.scriptClass->AddInternalCall("Internal_setEvolvers", (void*)&ScriptCParticleSystem::Internal_setEvolvers);
		metaData.scriptClass->AddInternalCall("Internal_getEvolvers", (void*)&ScriptCParticleSystem::Internal_getEvolvers);
		metaData.scriptClass->AddInternalCall("Internal_setLayer", (void*)&ScriptCParticleSystem::Internal_setLayer);
		metaData.scriptClass->AddInternalCall("Internal_getLayer", (void*)&ScriptCParticleSystem::Internal_getLayer);
		metaData.scriptClass->AddInternalCall("Internal__togglePreviewMode", (void*)&ScriptCParticleSystem::Internal__togglePreviewMode);

	}

	void ScriptCParticleSystem::Internal_setSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings)
	{
		SPtr<ParticleSystemSettings> tmpsettings;
		ScriptParticleSystemSettings* scriptsettings;
		scriptsettings = ScriptParticleSystemSettings::toNative(settings);
		if(scriptsettings != nullptr)
			tmpsettings = scriptsettings->GetInternal();
		thisPtr->GetHandle()->setSettings(*tmpsettings);
	}

	MonoObject* ScriptCParticleSystem::Internal_getSettings(ScriptCParticleSystem* thisPtr)
	{
		SPtr<ParticleSystemSettings> tmp__output = bs_shared_ptr_new<ParticleSystemSettings>();
		*tmp__output = thisPtr->GetHandle()->getSettings();

		MonoObject* __output;
		__output = ScriptParticleSystemSettings::create(tmp__output);

		return __output;
	}

	void ScriptCParticleSystem::Internal_setGpuSimulationSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings)
	{
		SPtr<ParticleGpuSimulationSettings> tmpsettings;
		ScriptParticleGpuSimulationSettings* scriptsettings;
		scriptsettings = ScriptParticleGpuSimulationSettings::toNative(settings);
		if(scriptsettings != nullptr)
			tmpsettings = scriptsettings->GetInternal();
		thisPtr->GetHandle()->setGpuSimulationSettings(*tmpsettings);
	}

	MonoObject* ScriptCParticleSystem::Internal_getGpuSimulationSettings(ScriptCParticleSystem* thisPtr)
	{
		SPtr<ParticleGpuSimulationSettings> tmp__output = bs_shared_ptr_new<ParticleGpuSimulationSettings>();
		*tmp__output = thisPtr->GetHandle()->getGpuSimulationSettings();

		MonoObject* __output;
		__output = ScriptParticleGpuSimulationSettings::create(tmp__output);

		return __output;
	}

	void ScriptCParticleSystem::Internal_setEmitters(ScriptCParticleSystem* thisPtr, MonoArray* emitters)
	{
		Vector<SPtr<ParticleEmitter>> vecemitters;
		if(emitters != nullptr)
		{
			ScriptArray Arrayemitters(emitters);
			vecemitters.Resize(arrayemitters.size());
			for(int i = 0; i < (int)arrayemitters.Size(); i++)
			{
				ScriptParticleEmitter* scriptemitters;
				scriptemitters = ScriptParticleEmitter::toNative(arrayemitters.get<MonoObject*>(i));
				if(scriptemitters != nullptr)
				{
					SPtr<ParticleEmitter> arrayElemPtremitters = scriptemitters->GetInternal();
					vecemitters[i] = arrayElemPtremitters;
				}
			}
		}
		thisPtr->GetHandle()->setEmitters(vecemitters);
	}

	MonoArray* ScriptCParticleSystem::Internal_getEmitters(ScriptCParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEmitter>> vec__output;
		vec__output = thisPtr->GetHandle()->getEmitters();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptParticleEmitter>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<ParticleEmitter> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptParticleEmitter::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptCParticleSystem::Internal_setEvolvers(ScriptCParticleSystem* thisPtr, MonoArray* evolvers)
	{
		Vector<SPtr<ParticleEvolver>> vecevolvers;
		if(evolvers != nullptr)
		{
			ScriptArray Arrayevolvers(evolvers);
			vecevolvers.Resize(arrayevolvers.size());
			for(int i = 0; i < (int)arrayevolvers.Size(); i++)
			{
				ScriptParticleEvolverBase* scriptevolvers;
				scriptevolvers = (ScriptParticleEvolverBase*)ScriptParticleEvolver::toNative(arrayevolvers.get<MonoObject*>(i));
				if(scriptevolvers != nullptr)
				{
					SPtr<ParticleEvolver> arrayElemPtrevolvers = scriptevolvers->GetInternal();
					vecevolvers[i] = arrayElemPtrevolvers;
				}
			}
		}
		thisPtr->GetHandle()->setEvolvers(vecevolvers);
	}

	MonoArray* ScriptCParticleSystem::Internal_getEvolvers(ScriptCParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEvolver>> vec__output;
		vec__output = thisPtr->GetHandle()->getEvolvers();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptParticleEvolver>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<ParticleEvolver> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			if(arrayElemPtr__output)
			{
				if(rtti_is_of_type<ParticleGravity>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleGravity::create(std::static_pointer_cast<ParticleGravity>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleForce>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleForce::create(std::static_pointer_cast<ParticleForce>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleVelocity>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleVelocity::create(std::static_pointer_cast<ParticleVelocity>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleTextureAnimation>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleTextureAnimation::create(std::static_pointer_cast<ParticleTextureAnimation>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleOrbit>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleOrbit::create(std::static_pointer_cast<ParticleOrbit>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleColor>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleColor::create(std::static_pointer_cast<ParticleColor>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleSize>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleSize::create(std::static_pointer_cast<ParticleSize>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleRotation>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleRotation::create(std::static_pointer_cast<ParticleRotation>(arrayElemPtr__output));
				else If(rtti_is_of_type<ParticleCollisions>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleCollisions::create(std::static_pointer_cast<ParticleCollisions>(arrayElemPtr__output));
				else
					arrayElem__output = ScriptParticleEvolver::create(arrayElemPtr__output);
			}
			else
				arrayElem__output = ScriptParticleEvolver::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptCParticleSystem::Internal_setLayer(ScriptCParticleSystem* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->setLayer(layer);
	}

	uint64_t ScriptCParticleSystem::Internal_getLayer(ScriptCParticleSystem* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCParticleSystem::Internal__togglePreviewMode(ScriptCParticleSystem* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->_togglePreviewMode(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
