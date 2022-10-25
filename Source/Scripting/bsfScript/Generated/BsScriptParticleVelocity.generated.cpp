//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleVelocity.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_VELOCITY_DESC.generated.h"
#include "BsScriptParticleVelocity.generated.h"

namespace bs
{
	ScriptParticleVelocity::ScriptParticleVelocity(MonoObject* managedInstance, const SPtr<ParticleVelocity>& value)
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleVelocity::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleVelocity::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleVelocity::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleVelocity::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleVelocity::InternalCreate0);
	}

	MonoObject* ScriptParticleVelocity::Create(const SPtr<ParticleVelocity>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptParticleVelocity>()) ScriptParticleVelocity(managedInstance, value);
		return managedInstance;
	}

	void ScriptParticleVelocity::InternalSetOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* options)
	{
		PARTICLE_VELOCITY_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_VELOCITY_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleVelocity::InternalGetOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* __output)
	{
		PARTICLE_VELOCITY_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_VELOCITY_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_VELOCITY_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_VELOCITY_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleVelocity::InternalCreate(MonoObject* managedInstance, __PARTICLE_VELOCITY_DESCInterop* desc)
	{
		PARTICLE_VELOCITY_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_VELOCITY_DESC::FromInterop(*desc);
		SPtr<ParticleVelocity> instance = ParticleVelocity::Create(tmpdesc);
		new(bs_alloc<ScriptParticleVelocity>()) ScriptParticleVelocity(managedInstance, instance);
	}

	void ScriptParticleVelocity::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleVelocity> instance = ParticleVelocity::Create();
		new(bs_alloc<ScriptParticleVelocity>()) ScriptParticleVelocity(managedInstance, instance);
	}
} // namespace bs
