//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptParticleVelocity>()) ScriptParticleVelocity(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleVelocity::InternalSetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* options)
	{
		PARTICLE_VELOCITY_DESC tmpoptions;
		tmpoptions = ScriptParticleVelocityOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleVelocity::InternalGetOptions(ScriptParticleVelocity* self, __PARTICLE_VELOCITY_DESCInterop* __output)
	{
		PARTICLE_VELOCITY_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_VELOCITY_DESCInterop interop__output;
		interop__output = ScriptParticleVelocityOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleVelocityOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleVelocity::InternalCreate(MonoObject* managedInstance, __PARTICLE_VELOCITY_DESCInterop* desc)
	{
		PARTICLE_VELOCITY_DESC tmpdesc;
		tmpdesc = ScriptParticleVelocityOptions::FromInterop(*desc);
		SPtr<ParticleVelocity> nativeObject = ParticleVelocity::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleVelocity>())ScriptParticleVelocity(managedInstance, nativeObject);
	}

	void ScriptParticleVelocity::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleVelocity> nativeObject = ParticleVelocity::Create();
		new (B3DAllocate<ScriptParticleVelocity>())ScriptParticleVelocity(managedInstance, nativeObject);
	}
}
