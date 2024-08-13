//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleSize.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_SIZE_DESC.generated.h"
#include "BsScriptParticleSize.generated.h"

namespace bs
{
	ScriptParticleSize::ScriptParticleSize(MonoObject* managedInstance, const SPtr<ParticleSize>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleSize::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleSize::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleSize::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleSize::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleSize::InternalCreate0);

	}

	MonoObject* ScriptParticleSize::Create(const SPtr<ParticleSize>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleSize>()) ScriptParticleSize(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleSize::InternalSetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* options)
	{
		PARTICLE_SIZE_DESC tmpoptions;
		tmpoptions = ScriptParticleSizeOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleSize::InternalGetOptions(ScriptParticleSize* self, __PARTICLE_SIZE_DESCInterop* __output)
	{
		PARTICLE_SIZE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_SIZE_DESCInterop interop__output;
		interop__output = ScriptParticleSizeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleSizeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleSize::InternalCreate(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc)
	{
		PARTICLE_SIZE_DESC tmpdesc;
		tmpdesc = ScriptParticleSizeOptions::FromInterop(*desc);
		SPtr<ParticleSize> nativeObject = ParticleSize::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleSize>())ScriptParticleSize(managedInstance, nativeObject);
	}

	void ScriptParticleSize::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleSize> nativeObject = ParticleSize::Create();
		new (B3DAllocate<ScriptParticleSize>())ScriptParticleSize(managedInstance, nativeObject);
	}
}
