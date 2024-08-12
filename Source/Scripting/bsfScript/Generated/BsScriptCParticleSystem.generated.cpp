//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

namespace bs
{
	ScriptParticleSystem::ScriptParticleSystem(MonoObject* managedInstance, const GameObjectHandle<CParticleSystem>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptParticleSystem::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleSystem::InternalSetSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleSystem::InternalGetSettings);
		metaData.ScriptClass->AddInternalCall("Internal_SetGpuSimulationSettings", (void*)&ScriptParticleSystem::InternalSetGpuSimulationSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetGpuSimulationSettings", (void*)&ScriptParticleSystem::InternalGetGpuSimulationSettings);
		metaData.ScriptClass->AddInternalCall("Internal_SetEmitters", (void*)&ScriptParticleSystem::InternalSetEmitters);
		metaData.ScriptClass->AddInternalCall("Internal_GetEmitters", (void*)&ScriptParticleSystem::InternalGetEmitters);
		metaData.ScriptClass->AddInternalCall("Internal_SetEvolvers", (void*)&ScriptParticleSystem::InternalSetEvolvers);
		metaData.ScriptClass->AddInternalCall("Internal_GetEvolvers", (void*)&ScriptParticleSystem::InternalGetEvolvers);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptParticleSystem::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptParticleSystem::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptParticleSystem::InternalTogglePreviewModeInternal);

	}

	void ScriptParticleSystem::InternalSetSettings(ScriptParticleSystem* thisPtr, MonoObject* settings)
	{
		SPtr<ParticleSystemSettings> tmpsettings;
		ScriptParticleSystemSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleSystemSettings::ToNative(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = scriptObjectWrappersettings->GetInternal();
		thisPtr->GetHandle()->SetSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetSettings(ScriptParticleSystem* thisPtr)
	{
		SPtr<ParticleSystemSettings> tmp__output = B3DMakeShared<ParticleSystemSettings>();
		*tmp__output = thisPtr->GetHandle()->GetSettings();

		MonoObject* __output;
		__output = ScriptParticleSystemSettings::Create(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetGpuSimulationSettings(ScriptParticleSystem* thisPtr, MonoObject* settings)
	{
		SPtr<ParticleGpuSimulationSettings> tmpsettings;
		ScriptParticleGpuSimulationSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleGpuSimulationSettings::ToNative(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = scriptObjectWrappersettings->GetInternal();
		thisPtr->GetHandle()->SetGpuSimulationSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetGpuSimulationSettings(ScriptParticleSystem* thisPtr)
	{
		SPtr<ParticleGpuSimulationSettings> tmp__output = B3DMakeShared<ParticleGpuSimulationSettings>();
		*tmp__output = thisPtr->GetHandle()->GetGpuSimulationSettings();

		MonoObject* __output;
		__output = ScriptParticleGpuSimulationSettings::Create(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetEmitters(ScriptParticleSystem* thisPtr, MonoArray* emitters)
	{
		Vector<SPtr<ParticleEmitter>> nativeArrayemitters;
		if(emitters != nullptr)
		{
			ScriptArray scriptArrayemitters(emitters);
			nativeArrayemitters.resize(scriptArrayemitters.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayemitters.Size(); elementIndex++)
			{
				ScriptParticleEmitter* scriptObjectWrapperemitters;
				scriptObjectWrapperemitters = ScriptParticleEmitter::ToNative(scriptArrayemitters.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperemitters != nullptr)
				{
					SPtr<ParticleEmitter> arrayElementPointeremitters = scriptObjectWrapperemitters->GetInternal();
					nativeArrayemitters[elementIndex] = arrayElementPointeremitters;
				}
			}
		}
		thisPtr->GetHandle()->SetEmitters(nativeArrayemitters);
	}

	MonoArray* ScriptParticleSystem::InternalGetEmitters(ScriptParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEmitter>> nativeArray__output;
		nativeArray__output = thisPtr->GetHandle()->GetEmitters();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleEmitter>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ParticleEmitter> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptParticleEmitter::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleSystem::InternalSetEvolvers(ScriptParticleSystem* thisPtr, MonoArray* evolvers)
	{
		Vector<SPtr<ParticleEvolver>> nativeArrayevolvers;
		if(evolvers != nullptr)
		{
			ScriptArray scriptArrayevolvers(evolvers);
			nativeArrayevolvers.resize(scriptArrayevolvers.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayevolvers.Size(); elementIndex++)
			{
				ScriptParticleEvolverBase* scriptObjectWrapperevolvers;
				scriptObjectWrapperevolvers = (ScriptParticleEvolverBase*)ScriptParticleEvolver::ToNative(scriptArrayevolvers.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperevolvers != nullptr)
				{
					SPtr<ParticleEvolver> arrayElementPointerevolvers = scriptObjectWrapperevolvers->GetInternal();
					nativeArrayevolvers[elementIndex] = arrayElementPointerevolvers;
				}
			}
		}
		thisPtr->GetHandle()->SetEvolvers(nativeArrayevolvers);
	}

	MonoArray* ScriptParticleSystem::InternalGetEvolvers(ScriptParticleSystem* thisPtr)
	{
		Vector<SPtr<ParticleEvolver>> nativeArray__output;
		nativeArray__output = thisPtr->GetHandle()->GetEvolvers();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleEvolver>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ParticleEvolver> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			if(arrayElementPointer__output)
			{
				if(B3DRTTIIsOfType<ParticleGravity>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleGravity::Create(std::static_pointer_cast<ParticleGravity>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleForce>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleForce::Create(std::static_pointer_cast<ParticleForce>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleVelocity>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleVelocity::Create(std::static_pointer_cast<ParticleVelocity>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleTextureAnimation>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleTextureAnimation::Create(std::static_pointer_cast<ParticleTextureAnimation>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleOrbit>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleOrbit::Create(std::static_pointer_cast<ParticleOrbit>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleColor>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleColor::Create(std::static_pointer_cast<ParticleColor>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleSize>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleSize::Create(std::static_pointer_cast<ParticleSize>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleRotation>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleRotation::Create(std::static_pointer_cast<ParticleRotation>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleCollisions>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleCollisions::Create(std::static_pointer_cast<ParticleCollisions>(arrayElementPointer__output));
				else
					arrayElement__output = ScriptParticleEvolver::Create(arrayElementPointer__output);
			}
			else
				arrayElement__output = ScriptParticleEvolver::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleSystem::InternalSetLayer(ScriptParticleSystem* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptParticleSystem::InternalGetLayer(ScriptParticleSystem* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptParticleSystem::InternalTogglePreviewModeInternal(ScriptParticleSystem* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
