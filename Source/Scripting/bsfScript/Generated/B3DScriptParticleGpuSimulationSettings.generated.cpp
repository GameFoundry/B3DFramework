//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleGpuSimulationSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleDepthCollisionSettings.generated.h"
#include "B3DScriptParticleVectorFieldSettings.generated.h"
#include "B3DScriptTColorDistribution.generated.h"
#include "B3DScriptTDistribution.generated.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptParticleGpuSimulationSettings::ScriptParticleGpuSimulationSettings(const TShared<ParticleGpuSimulationSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleGpuSimulationSettings::~ScriptParticleGpuSimulationSettings()
	{
		UnregisterEvents();
	}

	void ScriptParticleGpuSimulationSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVectorField", (void*)&ScriptParticleGpuSimulationSettings::InternalGetVectorField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVectorField", (void*)&ScriptParticleGpuSimulationSettings::InternalSetVectorField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalGetColorOverLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColorOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalSetColorOverLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalGetSizeScaleOverLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSizeScaleOverLifetime", (void*)&ScriptParticleGpuSimulationSettings::InternalSetSizeScaleOverLifetime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAcceleration", (void*)&ScriptParticleGpuSimulationSettings::InternalGetAcceleration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAcceleration", (void*)&ScriptParticleGpuSimulationSettings::InternalSetAcceleration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDrag", (void*)&ScriptParticleGpuSimulationSettings::InternalGetDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDrag", (void*)&ScriptParticleGpuSimulationSettings::InternalSetDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepthCollision", (void*)&ScriptParticleGpuSimulationSettings::InternalGetDepthCollision);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDepthCollision", (void*)&ScriptParticleGpuSimulationSettings::InternalSetDepthCollision);

	}

	MonoObject* ScriptParticleGpuSimulationSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetVectorField(ScriptParticleGpuSimulationSettings* self)
	{
		TShared<ParticleVectorFieldSettings> tmp__output = B3DMakeShared<ParticleVectorFieldSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->VectorField;

		MonoObject* __output;
		__output = ScriptParticleVectorFieldSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetVectorField(ScriptParticleGpuSimulationSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ParticleVectorFieldSettings> tmpvalue;
		ScriptParticleVectorFieldSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptParticleVectorFieldSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ParticleVectorFieldSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->VectorField = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetColorOverLifetime(ScriptParticleGpuSimulationSettings* self)
	{
		TShared<TColorDistribution<ColorGradient>> tmp__output = B3DMakeShared<TColorDistribution<ColorGradient>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->ColorOverLifetime;

		MonoObject* __output;
		__output = ScriptColorDistribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetColorOverLifetime(ScriptParticleGpuSimulationSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TColorDistribution<ColorGradient>> tmpvalue;
		ScriptColorDistribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptColorDistribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TColorDistribution<ColorGradient>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->ColorOverLifetime = *tmpvalue;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* self)
	{
		TShared<TDistribution<TVector2<float>>> tmp__output = B3DMakeShared<TDistribution<TVector2<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->SizeScaleOverLifetime;

		MonoObject* __output;
		__output = ScriptVector2Distribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetSizeScaleOverLifetime(ScriptParticleGpuSimulationSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<TVector2<float>>> tmpvalue;
		ScriptVector2Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector2Distribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<TVector2<float>>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->SizeScaleOverLifetime = *tmpvalue;
	}

	void ScriptParticleGpuSimulationSettings::InternalGetAcceleration(ScriptParticleGpuSimulationSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->Acceleration;

		*__output = tmp__output;


	}

	void ScriptParticleGpuSimulationSettings::InternalSetAcceleration(ScriptParticleGpuSimulationSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->Acceleration = *value;
	}

	float ScriptParticleGpuSimulationSettings::InternalGetDrag(ScriptParticleGpuSimulationSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->Drag;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetDrag(ScriptParticleGpuSimulationSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->Drag = value;
	}

	MonoObject* ScriptParticleGpuSimulationSettings::InternalGetDepthCollision(ScriptParticleGpuSimulationSettings* self)
	{
		TShared<ParticleDepthCollisionSettings> tmp__output = B3DMakeShared<ParticleDepthCollisionSettings>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->DepthCollision;

		MonoObject* __output;
		__output = ScriptParticleDepthCollisionSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleGpuSimulationSettings::InternalSetDepthCollision(ScriptParticleGpuSimulationSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ParticleDepthCollisionSettings> tmpvalue;
		ScriptParticleDepthCollisionSettings* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptParticleDepthCollisionSettings::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ParticleDepthCollisionSettings>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleGpuSimulationSettings*>(self->GetNativeObject())->DepthCollision = *tmpvalue;
	}
}
