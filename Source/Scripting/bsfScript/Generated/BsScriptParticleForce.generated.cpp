//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleForce.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_FORCE_DESC.generated.h"
#include "BsScriptParticleForce.generated.h"

namespace bs
{
	ScriptParticleForce::ScriptParticleForce(MonoObject* managedInstance, const SPtr<ParticleForce>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleForce::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleForce::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleForce::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleForce::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleForce::InternalCreate0);

	}

	MonoObject* ScriptParticleForce::Create(const SPtr<ParticleForce>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleForce>()) ScriptParticleForce(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleForce::InternalSetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* options)
	{
		PARTICLE_FORCE_DESC tmpoptions;
		tmpoptions = ScriptParticleForceOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleForce::InternalGetOptions(ScriptParticleForce* self, __PARTICLE_FORCE_DESCInterop* __output)
	{
		PARTICLE_FORCE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_FORCE_DESCInterop interop__output;
		interop__output = ScriptParticleForceOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleForceOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleForce::InternalCreate(MonoObject* managedInstance, __PARTICLE_FORCE_DESCInterop* desc)
	{
		PARTICLE_FORCE_DESC tmpdesc;
		tmpdesc = ScriptParticleForceOptions::FromInterop(*desc);
		SPtr<ParticleForce> nativeObject = ParticleForce::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleForce>())ScriptParticleForce(managedInstance, nativeObject);
	}

	void ScriptParticleForce::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleForce> nativeObject = ParticleForce::Create();
		new (B3DAllocate<ScriptParticleForce>())ScriptParticleForce(managedInstance, nativeObject);
	}
}
