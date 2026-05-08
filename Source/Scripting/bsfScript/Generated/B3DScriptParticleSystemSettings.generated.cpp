//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleSystemSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Material/B3DMaterial.h"
#include "../../../Engine/Core/Mesh/B3DMesh.h"
#include "B3DScriptTAABox.generated.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptParticleSystemSettings::ScriptParticleSystemSettings(const TShared<ParticleSystemSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleSystemSettings::~ScriptParticleSystemSettings()
	{
		UnregisterEvents();
	}

	void ScriptParticleSystemSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptParticleSystemSettings::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptParticleSystemSettings::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptParticleSystemSettings::InternalGetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptParticleSystemSettings::InternalSetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSimulationSpace", (void*)&ScriptParticleSystemSettings::InternalGetSimulationSpace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSimulationSpace", (void*)&ScriptParticleSystemSettings::InternalSetSimulationSpace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrientation", (void*)&ScriptParticleSystemSettings::InternalGetOrientation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrientation", (void*)&ScriptParticleSystemSettings::InternalSetOrientation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDuration", (void*)&ScriptParticleSystemSettings::InternalGetDuration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDuration", (void*)&ScriptParticleSystemSettings::InternalSetDuration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsLooping", (void*)&ScriptParticleSystemSettings::InternalGetIsLooping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsLooping", (void*)&ScriptParticleSystemSettings::InternalSetIsLooping);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxParticles", (void*)&ScriptParticleSystemSettings::InternalGetMaxParticles);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxParticles", (void*)&ScriptParticleSystemSettings::InternalSetMaxParticles);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGpuSimulation", (void*)&ScriptParticleSystemSettings::InternalGetGpuSimulation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGpuSimulation", (void*)&ScriptParticleSystemSettings::InternalSetGpuSimulation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRenderMode", (void*)&ScriptParticleSystemSettings::InternalGetRenderMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRenderMode", (void*)&ScriptParticleSystemSettings::InternalSetRenderMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrientationLockY", (void*)&ScriptParticleSystemSettings::InternalGetOrientationLockY);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrientationLockY", (void*)&ScriptParticleSystemSettings::InternalSetOrientationLockY);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrientationPlaneNormal", (void*)&ScriptParticleSystemSettings::InternalGetOrientationPlaneNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrientationPlaneNormal", (void*)&ScriptParticleSystemSettings::InternalSetOrientationPlaneNormal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSortMode", (void*)&ScriptParticleSystemSettings::InternalGetSortMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSortMode", (void*)&ScriptParticleSystemSettings::InternalSetSortMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseAutomaticSeed", (void*)&ScriptParticleSystemSettings::InternalGetUseAutomaticSeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseAutomaticSeed", (void*)&ScriptParticleSystemSettings::InternalSetUseAutomaticSeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetManualSeed", (void*)&ScriptParticleSystemSettings::InternalGetManualSeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetManualSeed", (void*)&ScriptParticleSystemSettings::InternalSetManualSeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseAutomaticBounds", (void*)&ScriptParticleSystemSettings::InternalGetUseAutomaticBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseAutomaticBounds", (void*)&ScriptParticleSystemSettings::InternalSetUseAutomaticBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCustomBounds", (void*)&ScriptParticleSystemSettings::InternalGetCustomBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCustomBounds", (void*)&ScriptParticleSystemSettings::InternalSetCustomBounds);

	}

	MonoObject* ScriptParticleSystemSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptParticleSystemSettings::InternalGetMaterial(ScriptParticleSystemSettings* self)
	{
		TResourceHandle<Material> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Material;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaterial(ScriptParticleSystemSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Material> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Material>(scriptObjectWrappervalue->GetNativeObject());
		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Material = tmpvalue;
	}

	MonoObject* ScriptParticleSystemSettings::InternalGetMesh(ScriptParticleSystemSettings* self)
	{
		TResourceHandle<Mesh> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Mesh;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMesh(ScriptParticleSystemSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Mesh> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Mesh>(scriptObjectWrappervalue->GetNativeObject());
		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Mesh = tmpvalue;
	}

	ParticleSimulationSpace ScriptParticleSystemSettings::InternalGetSimulationSpace(ScriptParticleSystemSettings* self)
	{
		ParticleSimulationSpace tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->SimulationSpace;

		ParticleSimulationSpace __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSimulationSpace(ScriptParticleSystemSettings* self, ParticleSimulationSpace value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->SimulationSpace = value;
	}

	ParticleOrientation ScriptParticleSystemSettings::InternalGetOrientation(ScriptParticleSystemSettings* self)
	{
		ParticleOrientation tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Orientation;

		ParticleOrientation __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientation(ScriptParticleSystemSettings* self, ParticleOrientation value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Orientation = value;
	}

	float ScriptParticleSystemSettings::InternalGetDuration(ScriptParticleSystemSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Duration;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetDuration(ScriptParticleSystemSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->Duration = value;
	}

	bool ScriptParticleSystemSettings::InternalGetIsLooping(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->IsLooping;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetIsLooping(ScriptParticleSystemSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->IsLooping = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetMaxParticles(ScriptParticleSystemSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->MaxParticles;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetMaxParticles(ScriptParticleSystemSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->MaxParticles = value;
	}

	bool ScriptParticleSystemSettings::InternalGetGpuSimulation(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->GpuSimulation;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetGpuSimulation(ScriptParticleSystemSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->GpuSimulation = value;
	}

	ParticleRenderMode ScriptParticleSystemSettings::InternalGetRenderMode(ScriptParticleSystemSettings* self)
	{
		ParticleRenderMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->RenderMode;

		ParticleRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetRenderMode(ScriptParticleSystemSettings* self, ParticleRenderMode value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->RenderMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetOrientationLockY(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->OrientationLockY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetOrientationLockY(ScriptParticleSystemSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->OrientationLockY = value;
	}

	void ScriptParticleSystemSettings::InternalGetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->OrientationPlaneNormal;

		*__output = tmp__output;


	}

	void ScriptParticleSystemSettings::InternalSetOrientationPlaneNormal(ScriptParticleSystemSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->OrientationPlaneNormal = *value;
	}

	ParticleSortMode ScriptParticleSystemSettings::InternalGetSortMode(ScriptParticleSystemSettings* self)
	{
		ParticleSortMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->SortMode;

		ParticleSortMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetSortMode(ScriptParticleSystemSettings* self, ParticleSortMode value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->SortMode = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticSeed(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->UseAutomaticSeed;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticSeed(ScriptParticleSystemSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->UseAutomaticSeed = value;
	}

	uint32_t ScriptParticleSystemSettings::InternalGetManualSeed(ScriptParticleSystemSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->ManualSeed;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetManualSeed(ScriptParticleSystemSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->ManualSeed = value;
	}

	bool ScriptParticleSystemSettings::InternalGetUseAutomaticBounds(ScriptParticleSystemSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->UseAutomaticBounds;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleSystemSettings::InternalSetUseAutomaticBounds(ScriptParticleSystemSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->UseAutomaticBounds = value;
	}

	void ScriptParticleSystemSettings::InternalGetCustomBounds(ScriptParticleSystemSettings* self, __TAABox_float_Interop* __output)
	{
		TAABox<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleSystemSettings*>(self->GetNativeObject())->CustomBounds;

		__TAABox_float_Interop interop__output;
		interop__output = ScriptAABox::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAABox::GetMetaData()->ScriptClass->GetInternalClass());


	}

	void ScriptParticleSystemSettings::InternalSetCustomBounds(ScriptParticleSystemSettings* self, __TAABox_float_Interop* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TAABox<float> tmpvalue;
		tmpvalue = ScriptAABox::FromInterop(*value);
		static_cast<ParticleSystemSettings*>(self->GetNativeObject())->CustomBounds = tmpvalue;
	}
}
