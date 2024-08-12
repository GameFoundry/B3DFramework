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
	MonoObject* ScriptParticleVectorFieldSettings::InternalGetVectorField(ScriptParticleVectorFieldSettings* thisPtr)
	{
		TResourceHandle<VectorField> tmp__output;
		tmp__output = thisPtr->GetInternal()->VectorField;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetVectorField(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value)
	{
		TResourceHandle<VectorField> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<VectorField>(scriptObjectWrappervalue->GetHandle());
		thisPtr->GetInternal()->VectorField = tmpvalue;
	}

	float ScriptParticleVectorFieldSettings::InternalGetIntensity(ScriptParticleVectorFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetIntensity(ScriptParticleVectorFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Intensity = value;
	}

	float ScriptParticleVectorFieldSettings::InternalGetTightness(ScriptParticleVectorFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Tightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTightness(ScriptParticleVectorFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Tightness = value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetScale(ScriptParticleVectorFieldSettings* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetInternal()->Scale;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetScale(ScriptParticleVectorFieldSettings* thisPtr, TVector3<float>* value)
	{
		thisPtr->GetInternal()->Scale = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetOffset(ScriptParticleVectorFieldSettings* thisPtr, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetInternal()->Offset;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetOffset(ScriptParticleVectorFieldSettings* thisPtr, TVector3<float>* value)
	{
		thisPtr->GetInternal()->Offset = *value;
	}

	void ScriptParticleVectorFieldSettings::InternalGetRotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetInternal()->Rotation;

		*__output = tmp__output;


	}

	void ScriptParticleVectorFieldSettings::InternalSetRotation(ScriptParticleVectorFieldSettings* thisPtr, Quaternion* value)
	{
		thisPtr->GetInternal()->Rotation = *value;
	}

	MonoObject* ScriptParticleVectorFieldSettings::InternalGetRotationRate(ScriptParticleVectorFieldSettings* thisPtr)
	{
		SPtr<TDistribution<TVector3<float>>> tmp__output = B3DMakeShared<TDistribution<TVector3<float>>>();
		*tmp__output = thisPtr->GetInternal()->RotationRate;

		MonoObject* __output;
		__output = ScriptVector3Distribution::Create(tmp__output);

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetRotationRate(ScriptParticleVectorFieldSettings* thisPtr, MonoObject* value)
	{
		SPtr<TDistribution<TVector3<float>>> tmpvalue;
		ScriptVector3Distribution* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptVector3Distribution::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = scriptObjectWrappervalue->GetInternal();
		thisPtr->GetInternal()->RotationRate = *tmpvalue;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingX(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->TilingX;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingX(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->TilingX = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingY(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->TilingY;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingY(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->TilingY = value;
	}

	bool ScriptParticleVectorFieldSettings::InternalGetTilingZ(ScriptParticleVectorFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->TilingZ;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleVectorFieldSettings::InternalSetTilingZ(ScriptParticleVectorFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->TilingZ = value;
	}
}
