//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCParticleSystem.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCParticleSystem.h"
#include "Reflection/BsRTTIType.h"
#include "BsScriptParticleSystemSettings.generated.h"
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
#include "BsScriptParticleOrbit.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleColor.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleSize.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleRotation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleCollisions.generated.h"

using namespace bs;
ScriptCParticleSystem::ScriptCParticleSystem(MonoObject* managedInstance, const GameObjectHandle<CParticleSystem>& value)
	: TScriptComponent(managedInstance, value)
{
}

void ScriptCParticleSystem::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptCParticleSystem::InternalSetSettings);
	metaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptCParticleSystem::InternalGetSettings);
	metaData.ScriptClass->AddInternalCall("Internal_SetGpuSimulationSettings", (void*)&ScriptCParticleSystem::InternalSetGpuSimulationSettings);
	metaData.ScriptClass->AddInternalCall("Internal_GetGpuSimulationSettings", (void*)&ScriptCParticleSystem::InternalGetGpuSimulationSettings);
	metaData.ScriptClass->AddInternalCall("Internal_SetEmitters", (void*)&ScriptCParticleSystem::InternalSetEmitters);
	metaData.ScriptClass->AddInternalCall("Internal_GetEmitters", (void*)&ScriptCParticleSystem::InternalGetEmitters);
	metaData.ScriptClass->AddInternalCall("Internal_SetEvolvers", (void*)&ScriptCParticleSystem::InternalSetEvolvers);
	metaData.ScriptClass->AddInternalCall("Internal_GetEvolvers", (void*)&ScriptCParticleSystem::InternalGetEvolvers);
	metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCParticleSystem::InternalSetLayer);
	metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCParticleSystem::InternalGetLayer);
	metaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptCParticleSystem::InternalTogglePreviewModeInternal);
}

void ScriptCParticleSystem::InternalSetSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings)
{
	SPtr<ParticleSystemSettings> tmpsettings;
	ScriptParticleSystemSettings* scriptsettings;
	scriptsettings = ScriptParticleSystemSettings::ToNative(settings);
	if(scriptsettings != nullptr)
		tmpsettings = scriptsettings->GetInternal();
	thisPtr->GetHandle()->SetSettings(*tmpsettings);
}

MonoObject* ScriptCParticleSystem::InternalGetSettings(ScriptCParticleSystem* thisPtr)
{
	SPtr<ParticleSystemSettings> tmp__output = B3DMakeShared<ParticleSystemSettings>();
	*tmp__output = thisPtr->GetHandle()->GetSettings();

	MonoObject* __output;
	__output = ScriptParticleSystemSettings::Create(tmp__output);

	return __output;
}

void ScriptCParticleSystem::InternalSetGpuSimulationSettings(ScriptCParticleSystem* thisPtr, MonoObject* settings)
{
	SPtr<ParticleGpuSimulationSettings> tmpsettings;
	ScriptParticleGpuSimulationSettings* scriptsettings;
	scriptsettings = ScriptParticleGpuSimulationSettings::ToNative(settings);
	if(scriptsettings != nullptr)
		tmpsettings = scriptsettings->GetInternal();
	thisPtr->GetHandle()->SetGpuSimulationSettings(*tmpsettings);
}

MonoObject* ScriptCParticleSystem::InternalGetGpuSimulationSettings(ScriptCParticleSystem* thisPtr)
{
	SPtr<ParticleGpuSimulationSettings> tmp__output = B3DMakeShared<ParticleGpuSimulationSettings>();
	*tmp__output = thisPtr->GetHandle()->GetGpuSimulationSettings();

	MonoObject* __output;
	__output = ScriptParticleGpuSimulationSettings::Create(tmp__output);

	return __output;
}

void ScriptCParticleSystem::InternalSetEmitters(ScriptCParticleSystem* thisPtr, MonoArray* emitters)
{
	Vector<SPtr<ParticleEmitter>> vecemitters;
	if(emitters != nullptr)
	{
		ScriptArray arrayemitters(emitters);
		vecemitters.resize(arrayemitters.Size());
		for(int i = 0; i < (int)arrayemitters.Size(); i++)
		{
			ScriptParticleEmitter* scriptemitters;
			scriptemitters = ScriptParticleEmitter::ToNative(arrayemitters.Get<MonoObject*>(i));
			if(scriptemitters != nullptr)
			{
				SPtr<ParticleEmitter> arrayElemPtremitters = scriptemitters->GetInternal();
				vecemitters[i] = arrayElemPtremitters;
			}
		}
	}
	thisPtr->GetHandle()->SetEmitters(vecemitters);
}

MonoArray* ScriptCParticleSystem::InternalGetEmitters(ScriptCParticleSystem* thisPtr)
{
	Vector<SPtr<ParticleEmitter>> vec__output;
	vec__output = thisPtr->GetHandle()->GetEmitters();

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptParticleEmitter>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		SPtr<ParticleEmitter> arrayElemPtr__output = vec__output[i];
		MonoObject* arrayElem__output;
		arrayElem__output = ScriptParticleEmitter::Create(arrayElemPtr__output);
		array__output.Set(i, arrayElem__output);
	}
	__output = array__output.GetInternal();

	return __output;
}

void ScriptCParticleSystem::InternalSetEvolvers(ScriptCParticleSystem* thisPtr, MonoArray* evolvers)
{
	Vector<SPtr<ParticleEvolver>> vecevolvers;
	if(evolvers != nullptr)
	{
		ScriptArray arrayevolvers(evolvers);
		vecevolvers.resize(arrayevolvers.Size());
		for(int i = 0; i < (int)arrayevolvers.Size(); i++)
		{
			ScriptParticleEvolverBase* scriptevolvers;
			scriptevolvers = (ScriptParticleEvolverBase*)ScriptParticleEvolver::ToNative(arrayevolvers.Get<MonoObject*>(i));
			if(scriptevolvers != nullptr)
			{
				SPtr<ParticleEvolver> arrayElemPtrevolvers = scriptevolvers->GetInternal();
				vecevolvers[i] = arrayElemPtrevolvers;
			}
		}
	}
	thisPtr->GetHandle()->SetEvolvers(vecevolvers);
}

MonoArray* ScriptCParticleSystem::InternalGetEvolvers(ScriptCParticleSystem* thisPtr)
{
	Vector<SPtr<ParticleEvolver>> vec__output;
	vec__output = thisPtr->GetHandle()->GetEvolvers();

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptParticleEvolver>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		SPtr<ParticleEvolver> arrayElemPtr__output = vec__output[i];
		MonoObject* arrayElem__output;
		if(arrayElemPtr__output)
		{
			if(rtti_is_of_type<ParticleGravity>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleGravity::Create(std::static_pointer_cast<ParticleGravity>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleForce>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleForce::Create(std::static_pointer_cast<ParticleForce>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleVelocity>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleVelocity::Create(std::static_pointer_cast<ParticleVelocity>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleTextureAnimation>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleTextureAnimation::Create(std::static_pointer_cast<ParticleTextureAnimation>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleOrbit>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleOrbit::Create(std::static_pointer_cast<ParticleOrbit>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleColor>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleColor::Create(std::static_pointer_cast<ParticleColor>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleSize>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleSize::Create(std::static_pointer_cast<ParticleSize>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleRotation>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleRotation::Create(std::static_pointer_cast<ParticleRotation>(arrayElemPtr__output));
			else if(rtti_is_of_type<ParticleCollisions>(arrayElemPtr__output))
				arrayElem__output = ScriptParticleCollisions::Create(std::static_pointer_cast<ParticleCollisions>(arrayElemPtr__output));
			else
				arrayElem__output = ScriptParticleEvolver::Create(arrayElemPtr__output);
		}
		else
			arrayElem__output = ScriptParticleEvolver::Create(arrayElemPtr__output);
		array__output.Set(i, arrayElem__output);
	}
	__output = array__output.GetInternal();

	return __output;
}

void ScriptCParticleSystem::InternalSetLayer(ScriptCParticleSystem* thisPtr, uint64_t layer)
{
	thisPtr->GetHandle()->SetLayer(layer);
}

uint64_t ScriptCParticleSystem::InternalGetLayer(ScriptCParticleSystem* thisPtr)
{
	uint64_t tmp__output;
	tmp__output = thisPtr->GetHandle()->GetLayer();

	uint64_t __output;
	__output = tmp__output;

	return __output;
}

bool ScriptCParticleSystem::InternalTogglePreviewModeInternal(ScriptCParticleSystem* thisPtr, bool enabled)
{
	bool tmp__output;
	tmp__output = thisPtr->GetHandle()->TogglePreviewModeInternal(enabled);

	bool __output;
	__output = tmp__output;

	return __output;
}
