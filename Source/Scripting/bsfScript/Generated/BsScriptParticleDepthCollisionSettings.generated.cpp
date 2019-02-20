//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptParticleDepthCollisionSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ParticleDepthCollisionSettings", (void*)&ScriptParticleDepthCollisionSettings::Internal_ParticleDepthCollisionSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptParticleDepthCollisionSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptParticleDepthCollisionSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getrestitution", (void*)&ScriptParticleDepthCollisionSettings::Internal_getrestitution);
		metaData.scriptClass->addInternalCall("Internal_setrestitution", (void*)&ScriptParticleDepthCollisionSettings::Internal_setrestitution);
		metaData.scriptClass->addInternalCall("Internal_getdampening", (void*)&ScriptParticleDepthCollisionSettings::Internal_getdampening);
		metaData.scriptClass->addInternalCall("Internal_setdampening", (void*)&ScriptParticleDepthCollisionSettings::Internal_setdampening);
		metaData.scriptClass->addInternalCall("Internal_getradiusScale", (void*)&ScriptParticleDepthCollisionSettings::Internal_getradiusScale);
		metaData.scriptClass->addInternalCall("Internal_setradiusScale", (void*)&ScriptParticleDepthCollisionSettings::Internal_setradiusScale);

	}

	MonoObject* ScriptParticleDepthCollisionSettings::create(const SPtr<ParticleDepthCollisionSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleDepthCollisionSettings>()) ScriptParticleDepthCollisionSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleDepthCollisionSettings::Internal_ParticleDepthCollisionSettings(MonoObject* managedInstance)
	{
		SPtr<ParticleDepthCollisionSettings> instance = bs_shared_ptr_new<ParticleDepthCollisionSettings>();
		new (bs_alloc<ScriptParticleDepthCollisionSettings>())ScriptParticleDepthCollisionSettings(managedInstance, instance);
	}

	bool ScriptParticleDepthCollisionSettings::Internal_getenabled(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::Internal_setenabled(ScriptParticleDepthCollisionSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	float ScriptParticleDepthCollisionSettings::Internal_getrestitution(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->restitution;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::Internal_setrestitution(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->restitution = value;
	}

	float ScriptParticleDepthCollisionSettings::Internal_getdampening(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->dampening;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::Internal_setdampening(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->dampening = value;
	}

	float ScriptParticleDepthCollisionSettings::Internal_getradiusScale(ScriptParticleDepthCollisionSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->radiusScale;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptParticleDepthCollisionSettings::Internal_setradiusScale(ScriptParticleDepthCollisionSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->radiusScale = value;
	}
}
