//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsMaterial.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMaterial.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMaterial.h"

namespace bs
{
	ScriptPhysicsMaterial::ScriptPhysicsMaterial(MonoObject* managedInstance, const ResourceHandle<PhysicsMaterial>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptPhysicsMaterial::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPhysicsMaterial::Internal_getRef);
		metaData.scriptClass->AddInternalCall("Internal_setStaticFriction", (void*)&ScriptPhysicsMaterial::Internal_setStaticFriction);
		metaData.scriptClass->AddInternalCall("Internal_getStaticFriction", (void*)&ScriptPhysicsMaterial::Internal_getStaticFriction);
		metaData.scriptClass->AddInternalCall("Internal_setDynamicFriction", (void*)&ScriptPhysicsMaterial::Internal_setDynamicFriction);
		metaData.scriptClass->AddInternalCall("Internal_getDynamicFriction", (void*)&ScriptPhysicsMaterial::Internal_getDynamicFriction);
		metaData.scriptClass->AddInternalCall("Internal_setRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::Internal_setRestitutionCoefficient);
		metaData.scriptClass->AddInternalCall("Internal_getRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::Internal_getRestitutionCoefficient);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptPhysicsMaterial::Internal_create);

	}

	 MonoObject*ScriptPhysicsMaterial::createInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.scriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptPhysicsMaterial::Internal_getRef(ScriptPhysicsMaterial* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptPhysicsMaterial::Internal_setStaticFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->setStaticFriction(value);
	}

	float ScriptPhysicsMaterial::Internal_getStaticFriction(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getStaticFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::Internal_setDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->setDynamicFriction(value);
	}

	float ScriptPhysicsMaterial::Internal_getDynamicFriction(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getDynamicFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::Internal_setRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->setRestitutionCoefficient(value);
	}

	float ScriptPhysicsMaterial::Internal_getRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getRestitutionCoefficient();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::Internal_create(MonoObject* managedInstance, float staticFriction, float dynamicFriction, float restitution)
	{
		ResourceHandle<PhysicsMaterial> instance = PhysicsMaterial::create(staticFriction, dynamicFriction, restitution);
		ScriptResourceManager::instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
