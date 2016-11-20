//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPhysicsMaterial.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptPhysicsMaterial::ScriptPhysicsMaterial(MonoObject* instance, const HPhysicsMaterial& material)
		:TScriptResource(instance, material)
	{

	}

	void ScriptPhysicsMaterial::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPhysicsMaterial::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetStaticFriction", &ScriptPhysicsMaterial::internal_GetStaticFriction);
		metaData.scriptClass->addInternalCall("Internal_SetStaticFriction", &ScriptPhysicsMaterial::internal_SetStaticFriction);
		metaData.scriptClass->addInternalCall("Internal_GetDynamicFriction", &ScriptPhysicsMaterial::internal_GetDynamicFriction);
		metaData.scriptClass->addInternalCall("Internal_SetDynamicFriction", &ScriptPhysicsMaterial::internal_SetDynamicFriction);
		metaData.scriptClass->addInternalCall("Internal_GetRestitution", &ScriptPhysicsMaterial::internal_GetRestitution);
		metaData.scriptClass->addInternalCall("Internal_SetRestitution", &ScriptPhysicsMaterial::internal_SetRestitution);
	}

	void ScriptPhysicsMaterial::internal_CreateInstance(MonoObject* instance, float staticFriction,
		float dynamicFriction, float restitution)
	{
		HPhysicsMaterial material = PhysicsMaterial::create(staticFriction, dynamicFriction, restitution);

		ScriptPhysicsMaterial* scriptInstance;
		ScriptResourceManager::instance().createScriptResource(instance, material, &scriptInstance);
	}

	float ScriptPhysicsMaterial::internal_GetStaticFriction(ScriptPhysicsMaterial* thisPtr)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		return material->getStaticFriction();
	}

	void ScriptPhysicsMaterial::internal_SetStaticFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		material->setStaticFriction(value);
	}

	float ScriptPhysicsMaterial::internal_GetDynamicFriction(ScriptPhysicsMaterial* thisPtr)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		return material->getDynamicFriction();
	}

	void ScriptPhysicsMaterial::internal_SetDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		material->setDynamicFriction(value);
	}

	float ScriptPhysicsMaterial::internal_GetRestitution(ScriptPhysicsMaterial* thisPtr)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		return material->getRestitutionCoefficient();
	}

	void ScriptPhysicsMaterial::internal_SetRestitution(ScriptPhysicsMaterial* thisPtr, float value)
	{
		HPhysicsMaterial material = thisPtr->getHandle();
		material->setRestitutionCoefficient(value);
	}

	MonoObject* ScriptPhysicsMaterial::createInstance()
	{
		bool dummy = false;

		void* params[1];
		params[0] = &dummy;

		return metaData.scriptClass->createInstance("bool", params);
	}
}