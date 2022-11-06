//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleDepthCollisionSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleDepthCollisionSettings::ScriptParticleDepthCollisionSettings(MonoObject* managedInstance, const SPtr<ParticleDepthCollisionSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptParticleDepthCollisionSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ParticleDepthCollisionSettings", (void*)&ScriptParticleDepthCollisionSettings::InternalParticleDepthCollisionSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptParticleDepthCollisionSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptParticleDepthCollisionSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetRestitution", (void*)&ScriptParticleDepthCollisionSettings::InternalGetRestitution);
		metaData.ScriptClass->AddInternalCall("Internal_SetRestitution", (void*)&ScriptParticleDepthCollisionSettings::InternalSetRestitution);
		metaData.ScriptClass->AddInternalCall("Internal_GetDampening", (void*)&ScriptParticleDepthCollisionSettings::InternalGetDampening);
		metaData.ScriptClass->AddInternalCall("Internal_SetDampening", (void*)&ScriptParticleDepthCollisionSettings::InternalSetDampening);
		metaData.ScriptClass->AddInternalCall("Internal_GetRadiusScale", (void*)&ScriptParticleDepthCollisionSettings::InternalGetRadiusScale);
		metaData.ScriptClass->AddInternalCall("Internal_SetRadiusScale", (void*)&ScriptParticleDepthCollisionSettings::InternalSetRadiusScale);

	}

	MonoObject* ScriptParticleDepthCollisionSettings::Create(const SPtr<ParticleDepthCollisionSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleDepthCollisionSettings>()) ScriptParticleDepthCollisionSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleDepthCollisionSettings::InternalParticleDepthCollisionSettings(MonoObject* managedInstance)
	{
		SPtr<ParticleDepthCollisionSettings> instance = B3DMakeShared<ParticleDepthCollisionSettings>();
		new (B3DAllocate<ScriptParticleDepthCollisionSettings>())ScriptParticleDepthCollisionSettings(managedInstance, instance);
	}

	bool ScriptParticleDepthCollisionSettings::InternalGetEnabled(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetEnabled(ScriptParticleDepthCollisionSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetRestitution(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Restitution;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetRestitution(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Restitution = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetDampening(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Dampening;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetDampening(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Dampening = value;
	}

	float ScriptParticleDepthCollisionSettings::InternalGetRadiusScale(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->RadiusScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::InternalSetRadiusScale(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->RadiusScale = value;
	}
}
