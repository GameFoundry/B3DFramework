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
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleColor.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleForce.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleVelocity.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleTextureAnimation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleOrbit.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleSize.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleRotation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleCollisions.generated.h"

namespace bs
{
	ScriptParticleSystem::ScriptParticleSystem(const GameObjectHandle<CParticleSystem>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleSystem::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleSystem::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleSystem::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGpuSimulationSettings", (void*)&ScriptParticleSystem::InternalSetGpuSimulationSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGpuSimulationSettings", (void*)&ScriptParticleSystem::InternalGetGpuSimulationSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEmitters", (void*)&ScriptParticleSystem::InternalSetEmitters);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEmitters", (void*)&ScriptParticleSystem::InternalGetEmitters);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEvolvers", (void*)&ScriptParticleSystem::InternalSetEvolvers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEvolvers", (void*)&ScriptParticleSystem::InternalGetEvolvers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptParticleSystem::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptParticleSystem::InternalGetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptParticleSystem::InternalTogglePreviewModeInternal);

	}

	MonoObject* ScriptParticleSystem::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleSystem::InternalSetSettings(ScriptParticleSystem* self, MonoObject* settings)
	{
		SPtr<ParticleSystemSettings> tmpsettings;
		ScriptParticleSystemSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleSystemSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<ParticleSystemSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<CParticleSystem*>(self->GetNativeObject())->SetSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetSettings(ScriptParticleSystem* self)
	{
		SPtr<ParticleSystemSettings> tmp__output = B3DMakeShared<ParticleSystemSettings>();
		*tmp__output = static_cast<CParticleSystem*>(self->GetNativeObject())->GetSettings();

		MonoObject* __output;
		__output = ScriptParticleSystemSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetGpuSimulationSettings(ScriptParticleSystem* self, MonoObject* settings)
	{
		SPtr<ParticleGpuSimulationSettings> tmpsettings;
		ScriptParticleGpuSimulationSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleGpuSimulationSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<ParticleGpuSimulationSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<CParticleSystem*>(self->GetNativeObject())->SetGpuSimulationSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetGpuSimulationSettings(ScriptParticleSystem* self)
	{
		SPtr<ParticleGpuSimulationSettings> tmp__output = B3DMakeShared<ParticleGpuSimulationSettings>();
		*tmp__output = static_cast<CParticleSystem*>(self->GetNativeObject())->GetGpuSimulationSettings();

		MonoObject* __output;
		__output = ScriptParticleGpuSimulationSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetEmitters(ScriptParticleSystem* self, MonoArray* emitters)
	{
		Vector<SPtr<ParticleEmitter>> nativeArrayemitters;
		if(emitters != nullptr)
		{
			ScriptArray scriptArrayemitters(emitters);
			nativeArrayemitters.resize(scriptArrayemitters.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayemitters.Size(); elementIndex++)
			{
				ScriptParticleEmitter* scriptObjectWrapperemitters;
				scriptObjectWrapperemitters = ScriptParticleEmitter::GetScriptObjectWrapper(scriptArrayemitters.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperemitters != nullptr)
				{
					SPtr<ParticleEmitter> arrayElementPointeremitters = std::static_pointer_cast<ParticleEmitter>(scriptObjectWrapperemitters->GetBaseNativeObjectAsShared());
					nativeArrayemitters[elementIndex] = arrayElementPointeremitters;
				}
			}
		}
		static_cast<CParticleSystem*>(self->GetNativeObject())->SetEmitters(nativeArrayemitters);
	}

	MonoArray* ScriptParticleSystem::InternalGetEmitters(ScriptParticleSystem* self)
	{
		Vector<SPtr<ParticleEmitter>> nativeArray__output;
		nativeArray__output = static_cast<CParticleSystem*>(self->GetNativeObject())->GetEmitters();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleEmitter>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ParticleEmitter> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptParticleEmitter::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleSystem::InternalSetEvolvers(ScriptParticleSystem* self, MonoArray* evolvers)
	{
		Vector<SPtr<ParticleEvolver>> nativeArrayevolvers;
		if(evolvers != nullptr)
		{
			ScriptArray scriptArrayevolvers(evolvers);
			nativeArrayevolvers.resize(scriptArrayevolvers.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayevolvers.Size(); elementIndex++)
			{
				ScriptParticleEvolverWrapperBase* scriptObjectWrapperevolvers;
				scriptObjectWrapperevolvers = (ScriptParticleEvolverWrapperBase*)ScriptParticleEvolver::GetScriptObjectWrapper(scriptArrayevolvers.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperevolvers != nullptr)
				{
					SPtr<ParticleEvolver> arrayElementPointerevolvers = std::static_pointer_cast<ParticleEvolver>(scriptObjectWrapperevolvers->GetBaseNativeObjectAsShared());
					nativeArrayevolvers[elementIndex] = arrayElementPointerevolvers;
				}
			}
		}
		static_cast<CParticleSystem*>(self->GetNativeObject())->SetEvolvers(nativeArrayevolvers);
	}

	MonoArray* ScriptParticleSystem::InternalGetEvolvers(ScriptParticleSystem* self)
	{
		Vector<SPtr<ParticleEvolver>> nativeArray__output;
		nativeArray__output = static_cast<CParticleSystem*>(self->GetNativeObject())->GetEvolvers();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleEvolver>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ParticleEvolver> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			if(arrayElementPointer__output)
			{
				if(B3DRTTIIsOfType<ParticleColor>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleColor::GetOrCreateScriptObject(std::static_pointer_cast<ParticleColor>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleGravity>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleGravity::GetOrCreateScriptObject(std::static_pointer_cast<ParticleGravity>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleForce>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleForce::GetOrCreateScriptObject(std::static_pointer_cast<ParticleForce>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleVelocity>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleVelocity::GetOrCreateScriptObject(std::static_pointer_cast<ParticleVelocity>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleTextureAnimation>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleTextureAnimation::GetOrCreateScriptObject(std::static_pointer_cast<ParticleTextureAnimation>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleOrbit>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleOrbit::GetOrCreateScriptObject(std::static_pointer_cast<ParticleOrbit>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleSize>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleSize::GetOrCreateScriptObject(std::static_pointer_cast<ParticleSize>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleRotation>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleRotation::GetOrCreateScriptObject(std::static_pointer_cast<ParticleRotation>(arrayElementPointer__output));
				else if(B3DRTTIIsOfType<ParticleCollisions>(arrayElementPointer__output))
					arrayElement__output = ScriptParticleCollisions::GetOrCreateScriptObject(std::static_pointer_cast<ParticleCollisions>(arrayElementPointer__output));
				else
					arrayElement__output = ScriptParticleEvolver::GetOrCreateScriptObject(arrayElementPointer__output);
			}
			else
				arrayElement__output = ScriptParticleEvolver::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleSystem::InternalSetLayer(ScriptParticleSystem* self, uint64_t layer)
	{
		static_cast<CParticleSystem*>(self->GetNativeObject())->SetLayer(layer);
	}

	uint64_t ScriptParticleSystem::InternalGetLayer(ScriptParticleSystem* self)
	{
		uint64_t tmp__output;
		tmp__output = static_cast<CParticleSystem*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptParticleSystem::InternalTogglePreviewModeInternal(ScriptParticleSystem* self, bool enabled)
	{
		bool tmp__output;
		tmp__output = static_cast<CParticleSystem*>(self->GetNativeObject())->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
