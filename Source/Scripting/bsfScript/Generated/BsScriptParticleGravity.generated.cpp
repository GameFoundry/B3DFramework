//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleGravity.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_GRAVITY_DESC.generated.h"
#include "BsScriptParticleGravity.generated.h"

namespace bs
{
	ScriptParticleGravity::ScriptParticleGravity(MonoObject* managedInstance, const SPtr<ParticleGravity>& value)
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleGravity::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleGravity::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleGravity::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleGravity::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleGravity::InternalCreate0);
	}

	MonoObject* ScriptParticleGravity::Create(const SPtr<ParticleGravity>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptParticleGravity>()) ScriptParticleGravity(managedInstance, value);
		return managedInstance;
	}

	void ScriptParticleGravity::InternalSetOptions(ScriptParticleGravity* thisPtr, PARTICLE_GRAVITY_DESC* options)
	{
		thisPtr->GetInternal()->SetOptions(*options);
	}

	void ScriptParticleGravity::InternalGetOptions(ScriptParticleGravity* thisPtr, PARTICLE_GRAVITY_DESC* __output)
	{
		PARTICLE_GRAVITY_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleGravity::InternalCreate(MonoObject* managedInstance, PARTICLE_GRAVITY_DESC* desc)
	{
		SPtr<ParticleGravity> instance = ParticleGravity::Create(*desc);
		new(bs_alloc<ScriptParticleGravity>()) ScriptParticleGravity(managedInstance, instance);
	}

	void ScriptParticleGravity::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleGravity> instance = ParticleGravity::Create();
		new(bs_alloc<ScriptParticleGravity>()) ScriptParticleGravity(managedInstance, instance);
	}
} // namespace bs
