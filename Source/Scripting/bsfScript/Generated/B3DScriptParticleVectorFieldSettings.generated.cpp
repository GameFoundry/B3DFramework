//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleVectorFieldSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Particles/B3DVectorField.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTQuaternion.generated.h"
#include "B3DScriptTDistribution.generated.h"

namespace b3d
{
	ScriptParticleVectorFieldSettings::ScriptParticleVectorFieldSettings(const TShared<ParticleVectorFieldSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleVectorFieldSettings::~ScriptParticleVectorFieldSettings()
	{
		UnregisterEvents();
	}

	void ScriptParticleVectorFieldSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVectorField", (void*)&ScriptParticleVectorFieldSettings::InternalGetVectorField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVectorField", (void*)&ScriptParticleVectorFieldSettings::InternalSetVectorField);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptParticleVectorFieldSettings::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptParticleVectorFieldSettings::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTightness", (void*)&ScriptParticleVectorFieldSettings::InternalGetTightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTightness", (void*)&ScriptParticleVectorFieldSettings::InternalSetTightness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptParticleVectorFieldSettings::InternalGetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScale", (void*)&ScriptParticleVectorFieldSettings::InternalSetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOffset", (void*)&ScriptParticleVectorFieldSettings::InternalGetOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOffset", (void*)&ScriptParticleVectorFieldSettings::InternalSetOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptParticleVectorFieldSettings::InternalGetRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptParticleVectorFieldSettings::InternalSetRotation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotationRate", (void*)&ScriptParticleVectorFieldSettings::InternalGetRotationRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRotationRate", (void*)&ScriptParticleVectorFieldSettings::InternalSetRotationRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTilingX", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingX);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTilingX", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingX);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTilingY", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingY);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTilingY", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingY);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTilingZ", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingZ);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTilingZ", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingZ);

	}

	MonoObject* ScriptParticleVectorFieldSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptParticleVectorFieldSettings::InternalGetVectorField(ScriptParticleVectorFieldSettings* self)
	{
		TResourceHandle<VectorField> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->VectorField;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetVectorField(ScriptParticleVectorFieldSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<VectorField> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<VectorField>(scriptObjectWrappervalue->GetNativeObject());
		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->VectorField = tmpvalue;
	}

	float ScriptParticleVectorFieldSettings::InternalGetIntensity(ScriptParticleVectorFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetIntensity(ScriptParticleVectorFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Intensity = value;
	}

	float ScriptParticleVectorFieldSettings::InternalGetTightness(ScriptParticleVectorFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Tightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTightness(ScriptParticleVectorFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Tightness = value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Scale;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Scale = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Offset;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Offset = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetRotation(ScriptParticleVectorFieldSettings* self, TQuaternion<float>* __output)
	{
		TQuaternion<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Rotation;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetRotation(ScriptParticleVectorFieldSettings* self, TQuaternion<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->Rotation = *value;
	}

	MonoObject* ScriptParticleVectorFieldSettings::InternalGetRotationRate(ScriptParticleVectorFieldSettings* self)
	{
		TShared<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->RotationRate;

		MonoObject* __output;
		__output = ScriptVector3Distribution::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetRotationRate(ScriptParticleVectorFieldSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->RotationRate = *tmpvalue;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingX(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingX;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingX(ScriptParticleVectorFieldSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingX = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingY(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingY(ScriptParticleVectorFieldSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingY = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingZ(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingZ;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingZ(ScriptParticleVectorFieldSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleVectorFieldSettings*>(self->GetNativeObject())->TilingZ = value;
	}
}
