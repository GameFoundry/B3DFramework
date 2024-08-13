//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleVectorFieldSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Particles/BsVectorField.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTDistribution.generated.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptParticleVectorFieldSettings::ScriptParticleVectorFieldSettings(MonoObject* managedInstance, const SPtr<ParticleVectorFieldSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleVectorFieldSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetVectorField", (void*)&ScriptParticleVectorFieldSettings::InternalGetVectorField);
		metaData.ScriptClass->AddInternalCall("Internal_SetVectorField", (void*)&ScriptParticleVectorFieldSettings::InternalSetVectorField);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptParticleVectorFieldSettings::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptParticleVectorFieldSettings::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetTightness", (void*)&ScriptParticleVectorFieldSettings::InternalGetTightness);
		metaData.ScriptClass->AddInternalCall("Internal_SetTightness", (void*)&ScriptParticleVectorFieldSettings::InternalSetTightness);
		metaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptParticleVectorFieldSettings::InternalGetScale);
		metaData.ScriptClass->AddInternalCall("Internal_SetScale", (void*)&ScriptParticleVectorFieldSettings::InternalSetScale);
		metaData.ScriptClass->AddInternalCall("Internal_GetOffset", (void*)&ScriptParticleVectorFieldSettings::InternalGetOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetOffset", (void*)&ScriptParticleVectorFieldSettings::InternalSetOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptParticleVectorFieldSettings::InternalGetRotation);
		metaData.ScriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptParticleVectorFieldSettings::InternalSetRotation);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotationRate", (void*)&ScriptParticleVectorFieldSettings::InternalGetRotationRate);
		metaData.ScriptClass->AddInternalCall("Internal_SetRotationRate", (void*)&ScriptParticleVectorFieldSettings::InternalSetRotationRate);
		metaData.ScriptClass->AddInternalCall("Internal_GetTilingX", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingX);
		metaData.ScriptClass->AddInternalCall("Internal_SetTilingX", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingX);
		metaData.ScriptClass->AddInternalCall("Internal_GetTilingY", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingY);
		metaData.ScriptClass->AddInternalCall("Internal_SetTilingY", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingY);
		metaData.ScriptClass->AddInternalCall("Internal_GetTilingZ", (void*)&ScriptParticleVectorFieldSettings::InternalGetTilingZ);
		metaData.ScriptClass->AddInternalCall("Internal_SetTilingZ", (void*)&ScriptParticleVectorFieldSettings::InternalSetTilingZ);

	}

	MonoObject* ScriptParticleVectorFieldSettings::Create(const SPtr<ParticleVectorFieldSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleVectorFieldSettings>()) ScriptParticleVectorFieldSettings(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptParticleVectorFieldSettings::InternalGetVectorField(ScriptParticleVectorFieldSettings* self)
	{
		TResourceHandle<VectorField> tmp__output;
		tmp__output = self->GetInternal()->VectorField;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetVectorField(ScriptParticleVectorFieldSettings* self, MonoObject* value)
	{
		TResourceHandle<VectorField> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<VectorField>(scriptObjectWrappervalue->GetHandle());
		self->GetInternal()->VectorField = tmpvalue;
	}

	float ScriptParticleVectorFieldSettings::InternalGetIntensity(ScriptParticleVectorFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetIntensity(ScriptParticleVectorFieldSettings* self, float value)
	{
		self->GetInternal()->Intensity = value;
	}

	float ScriptParticleVectorFieldSettings::InternalGetTightness(ScriptParticleVectorFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Tightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTightness(ScriptParticleVectorFieldSettings* self, float value)
	{
		self->GetInternal()->Tightness = value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Scale;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Scale = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Offset;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Offset = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetRotation(ScriptParticleVectorFieldSettings* self, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = self->GetInternal()->Rotation;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetRotation(ScriptParticleVectorFieldSettings* self, Quaternion* value)
	{
		self->GetInternal()->Rotation = *value;
	}

	MonoObject* ScriptParticleVectorFieldSettings::InternalGetRotationRate(ScriptParticleVectorFieldSettings* self)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = self->GetInternal()->RotationRate;

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetRotationRate(ScriptParticleVectorFieldSettings* self, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		self->GetInternal()->RotationRate = *tmpvalue;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingX(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->TilingX;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingX(ScriptParticleVectorFieldSettings* self, bool value)
	{
		self->GetInternal()->TilingX = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingY(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->TilingY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingY(ScriptParticleVectorFieldSettings* self, bool value)
	{
		self->GetInternal()->TilingY = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingZ(ScriptParticleVectorFieldSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->TilingZ;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingZ(ScriptParticleVectorFieldSettings* self, bool value)
	{
		self->GetInternal()->TilingZ = value;
	}
}
