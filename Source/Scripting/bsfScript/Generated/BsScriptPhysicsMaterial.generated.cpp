//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	ScriptPhysicsMaterial::ScriptPhysicsMaterial(MonoObject* managedInstance, const TResourceHandle<PhysicsMaterial>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptPhysicsMaterial::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPhysicsMaterial::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_SetStaticFriction", (void*)&ScriptPhysicsMaterial::InternalSetStaticFriction);
		metaData.ScriptClass->AddInternalCall("Internal_GetStaticFriction", (void*)&ScriptPhysicsMaterial::InternalGetStaticFriction);
		metaData.ScriptClass->AddInternalCall("Internal_SetDynamicFriction", (void*)&ScriptPhysicsMaterial::InternalSetDynamicFriction);
		metaData.ScriptClass->AddInternalCall("Internal_GetDynamicFriction", (void*)&ScriptPhysicsMaterial::InternalGetDynamicFriction);
		metaData.ScriptClass->AddInternalCall("Internal_SetRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::InternalSetRestitutionCoefficient);
		metaData.ScriptClass->AddInternalCall("Internal_GetRestitutionCoefficient", (void*)&ScriptPhysicsMaterial::InternalGetRestitutionCoefficient);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPhysicsMaterial::InternalCreate);

	}

	 MonoObject*ScriptPhysicsMaterial::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptPhysicsMaterial::InternalGetRef(ScriptPhysicsMaterial* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptPhysicsMaterial::InternalSetStaticFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetStaticFriction(value);
	}

	float ScriptPhysicsMaterial::InternalGetStaticFriction(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetStaticFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalSetDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetDynamicFriction(value);
	}

	float ScriptPhysicsMaterial::InternalGetDynamicFriction(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDynamicFriction();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalSetRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr, float value)
	{
		thisPtr->GetHandle()->SetRestitutionCoefficient(value);
	}

	float ScriptPhysicsMaterial::InternalGetRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRestitutionCoefficient();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPhysicsMaterial::InternalCreate(MonoObject* managedInstance, float staticFriction, float dynamicFriction, float restitution)
	{
		TResourceHandle<PhysicsMaterial> instance = PhysicsMaterial::Create(staticFriction, dynamicFriction, restitution);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
