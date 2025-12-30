//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleSystem.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DParticleSystem.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptParticleSystemSettings.generated.h"
#include "B3DScriptParticleGpuSimulationSettings.generated.h"
#include "B3DScriptParticleEmitter.generated.h"
#include "B3DScriptParticleEvolver.generated.h"

namespace b3d
{
	ScriptParticleSystem::ScriptParticleSystem(const TGameObjectHandle<ParticleSystem>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleSystem::~ScriptParticleSystem()
	{
		UnregisterEvents();
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TogglePreviewMode", (void*)&ScriptParticleSystem::InternalTogglePreviewMode);

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
		if(!self->IsNativeObjectValid())
			return;

		SPtr<ParticleSystemSettings> tmpsettings;
		ScriptParticleSystemSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleSystemSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<ParticleSystemSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<ParticleSystem*>(self->GetNativeObject())->SetSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetSettings(ScriptParticleSystem* self)
	{
		SPtr<ParticleSystemSettings> tmp__output = B3DMakeShared<ParticleSystemSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleSystem*>(self->GetNativeObject())->GetSettings();

		MonoObject* __output;
		__output = ScriptParticleSystemSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetGpuSimulationSettings(ScriptParticleSystem* self, MonoObject* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		SPtr<ParticleGpuSimulationSettings> tmpsettings;
		ScriptParticleGpuSimulationSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptParticleGpuSimulationSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<ParticleGpuSimulationSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<ParticleSystem*>(self->GetNativeObject())->SetGpuSimulationSettings(*tmpsettings);
	}

	MonoObject* ScriptParticleSystem::InternalGetGpuSimulationSettings(ScriptParticleSystem* self)
	{
		SPtr<ParticleGpuSimulationSettings> tmp__output = B3DMakeShared<ParticleGpuSimulationSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleSystem*>(self->GetNativeObject())->GetGpuSimulationSettings();

		MonoObject* __output;
		__output = ScriptParticleGpuSimulationSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleSystem::InternalSetEmitters(ScriptParticleSystem* self, MonoArray* emitters)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<SPtr<ParticleEmitter>> nativeArrayemitters;
		if(emitters != nullptr)
		{
			ScriptArray scriptArrayemitters(emitters);
			nativeArrayemitters.resize(scriptArrayemitters.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayemitters.Size(); elementIndex++)
			{
				SPtr<ParticleEmitter> arrayElementPointeremitters;
				ScriptParticleEmitter* scriptObjectWrapperemitters;
				scriptObjectWrapperemitters = ScriptParticleEmitter::GetScriptObjectWrapper(scriptArrayemitters.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperemitters != nullptr)
				{
					arrayElementPointeremitters = std::static_pointer_cast<ParticleEmitter>(scriptObjectWrapperemitters->GetBaseNativeObjectAsShared());
					nativeArrayemitters[elementIndex] = arrayElementPointeremitters;
				}
			}
		}
		static_cast<ParticleSystem*>(self->GetNativeObject())->SetEmitters(nativeArrayemitters);
	}

	MonoArray* ScriptParticleSystem::InternalGetEmitters(ScriptParticleSystem* self)
	{
		Vector<SPtr<ParticleEmitter>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ParticleSystem*>(self->GetNativeObject())->GetEmitters();

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
		if(!self->IsNativeObjectValid())
			return;

		Vector<SPtr<ParticleEvolver>> nativeArrayevolvers;
		if(evolvers != nullptr)
		{
			ScriptArray scriptArrayevolvers(evolvers);
			nativeArrayevolvers.resize(scriptArrayevolvers.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayevolvers.Size(); elementIndex++)
			{
				SPtr<ParticleEvolver> arrayElementPointerevolvers;
				ScriptParticleEvolverWrapperBase* scriptObjectWrapperevolvers;
				scriptObjectWrapperevolvers = (ScriptParticleEvolverWrapperBase*)ScriptParticleEvolver::GetScriptObjectWrapper(scriptArrayevolvers.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperevolvers != nullptr)
				{
					arrayElementPointerevolvers = std::static_pointer_cast<ParticleEvolver>(scriptObjectWrapperevolvers->GetBaseNativeObjectAsShared());
					nativeArrayevolvers[elementIndex] = arrayElementPointerevolvers;
				}
			}
		}
		static_cast<ParticleSystem*>(self->GetNativeObject())->SetEvolvers(nativeArrayevolvers);
	}

	MonoArray* ScriptParticleSystem::InternalGetEvolvers(ScriptParticleSystem* self)
	{
		Vector<SPtr<ParticleEvolver>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ParticleSystem*>(self->GetNativeObject())->GetEvolvers();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptParticleEvolver>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<ParticleEvolver> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptParticleEvolver::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptParticleSystem::InternalSetLayer(ScriptParticleSystem* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystem*>(self->GetNativeObject())->SetLayer(layer);
	}

	uint64_t ScriptParticleSystem::InternalGetLayer(ScriptParticleSystem* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystem*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptParticleSystem::InternalTogglePreviewMode(ScriptParticleSystem* self, bool enabled)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystem*>(self->GetNativeObject())->TogglePreviewMode(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
