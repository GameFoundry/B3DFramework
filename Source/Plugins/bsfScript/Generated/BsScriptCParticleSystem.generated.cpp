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

	void ScriptCParticleSystem::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setSettings", (void*)&ScriptCParticleSystem::Internal_setSettings);
		metaData.scriptClass->addInternalCall("Internal_getSettings", (void*)&ScriptCParticleSystem::Internal_getSettings);
		metaData.scriptClass->addInternalCall("Internal_setGpuSimulationSettings", (void*)&ScriptCParticleSystem::Internal_setGpuSimulationSettings);
		metaData.scriptClass->addInternalCall("Internal_getGpuSimulationSettings", (void*)&ScriptCParticleSystem::Internal_getGpuSimulationSettings);
		metaData.scriptClass->addInternalCall("Internal_setEmitters", (void*)&ScriptCParticleSystem::Internal_setEmitters);
		metaData.scriptClass->addInternalCall("Internal_getEmitters", (void*)&ScriptCParticleSystem::Internal_getEmitters);
		metaData.scriptClass->addInternalCall("Internal_setEvolvers", (void*)&ScriptCParticleSystem::Internal_setEvolvers);
		metaData.scriptClass->addInternalCall("Internal_getEvolvers", (void*)&ScriptCParticleSystem::Internal_getEvolvers);
		metaData.scriptClass->addInternalCall("Internal_setLayer", (void*)&ScriptCParticleSystem::Internal_setLayer);
		metaData.scriptClass->addInternalCall("Internal_getLayer", (void*)&ScriptCParticleSystem::Internal_getLayer);
		metaData.scriptClass->addInternalCall("Internal__togglePreviewMode", (void*)&ScriptCParticleSystem::Internal__togglePreviewMode);

	}

	void ScriptCParticleSystem::Internal_setSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings)
	{
		SPtr<ParticleSystemSettings> tmpsettings;
		ScriptParticleSystemSettings* scriptsettings;
		scriptsettings = ScriptParticleSystemSettings::toNative(settings);
		if(scriptsettings != nullptr)
			tmpsettings = scriptsettings->getInternal();
		thisPtr->getHandle()->setSettings(*tmpsettings);
	}

	MonoObject* ScriptCParticleSystem::Internal_getSettings(ScriptCParticleSystem* thisPtr)
	{
		SPtr<ParticleSystemSettings> tmp__output = bs_shared_ptr_new<ParticleSystemSettings>();
		*tmp__output = thisPtr->getHandle()->getSettings();

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
			tmpsettings = scriptsettings->getInternal();
		thisPtr->getHandle()->setGpuSimulationSettings(*tmpsettings);
	}

	MonoObject* ScriptCParticleSystem::Internal_getGpuSimulationSettings(ScriptCParticleSystem* thisPtr)
	{
		SPtr<ParticleGpuSimulationSettings> tmp__output = bs_shared_ptr_new<ParticleGpuSimulationSettings>();
		*tmp__output = thisPtr->getHandle()->getGpuSimulationSettings();

		MonoObject* __output;
		__output = ScriptParticleGpuSimulationSettings::create(tmp__output);

		return __output;
	}

	void ScriptCParticleSystem::Internal_setEmitters(ScriptCParticleSystem* thisPtr, MonoArray* emitters)
	{
		Vector<SPtr<ParticleEmitter>> vecemitters;
		if(emitters != nullptr)
		{
			ScriptArray arrayemitters(emitters);
			vecemitters.resize(arrayemitters.size());
			for(int i = 0; i < (int)arrayemitters.size(); i++)
			{
				ScriptParticleEmitter* scriptemitters;
				scriptemitters = ScriptParticleEmitter::toNative(arrayemitters.get<MonoObject*>(i));
				if(scriptemitters != nullptr)
				{
					SPtr<ParticleEmitter> arrayElemPtremitters = scriptemitters->getInternal();
					vecemitters[i] = arrayElemPtremitters;
				}
			}
		}
		thisPtr->getHandle()->setEmitters(vecemitters);
	}

	MonoArray* ScriptCParticleSystem::Internal_getEmitters(ScriptCParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEmitter>> vec__output;
		vec__output = thisPtr->getHandle()->getEmitters();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptParticleEmitter>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<ParticleEmitter> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptParticleEmitter::create(arrayElemPtr__output);
			array__output.set(i, arrayElem__output);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptCParticleSystem::Internal_setEvolvers(ScriptCParticleSystem* thisPtr, MonoArray* evolvers)
	{
		Vector<SPtr<ParticleEvolver>> vecevolvers;
		if(evolvers != nullptr)
		{
			ScriptArray arrayevolvers(evolvers);
			vecevolvers.resize(arrayevolvers.size());
			for(int i = 0; i < (int)arrayevolvers.size(); i++)
			{
				ScriptParticleEvolverBase* scriptevolvers;
				scriptevolvers = (ScriptParticleEvolverBase*)ScriptParticleEvolver::toNative(arrayevolvers.get<MonoObject*>(i));
				if(scriptevolvers != nullptr)
				{
					SPtr<ParticleEvolver> arrayElemPtrevolvers = scriptevolvers->getInternal();
					vecevolvers[i] = arrayElemPtrevolvers;
				}
			}
		}
		thisPtr->getHandle()->setEvolvers(vecevolvers);
	}

	MonoArray* ScriptCParticleSystem::Internal_getEvolvers(ScriptCParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEvolver>> vec__output;
		vec__output = thisPtr->getHandle()->getEvolvers();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptParticleEvolver>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<ParticleEvolver> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			if(arrayElemPtr__output)
			{
				if(rtti_is_of_type<ParticleGravity>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleGravity::create(std::static_pointer_cast<ParticleGravity>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleForce>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleForce::create(std::static_pointer_cast<ParticleForce>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleVelocity>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleVelocity::create(std::static_pointer_cast<ParticleVelocity>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleTextureAnimation>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleTextureAnimation::create(std::static_pointer_cast<ParticleTextureAnimation>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleOrbit>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleOrbit::create(std::static_pointer_cast<ParticleOrbit>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleColor>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleColor::create(std::static_pointer_cast<ParticleColor>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleSize>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleSize::create(std::static_pointer_cast<ParticleSize>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleRotation>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleRotation::create(std::static_pointer_cast<ParticleRotation>(arrayElemPtr__output));
				else if(rtti_is_of_type<ParticleCollisions>(arrayElemPtr__output))
					arrayElem__output = ScriptParticleCollisions::create(std::static_pointer_cast<ParticleCollisions>(arrayElemPtr__output));
				else
					arrayElem__output = ScriptParticleEvolver::create(arrayElemPtr__output);
			}
			else
				arrayElem__output = ScriptParticleEvolver::create(arrayElemPtr__output);
			array__output.set(i, arrayElem__output);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptCParticleSystem::Internal_setLayer(ScriptCParticleSystem* thisPtr, uint64_t layer)
	{
		thisPtr->getHandle()->setLayer(layer);
	}

	uint64_t ScriptCParticleSystem::Internal_getLayer(ScriptCParticleSystem* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->getHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCParticleSystem::Internal__togglePreviewMode(ScriptCParticleSystem* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->_togglePreviewMode(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
