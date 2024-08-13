//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleRotation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_ROTATION_DESC.generated.h"
#include "BsScriptParticleRotation.generated.h"

namespace bs
{
	ScriptParticleRotation::ScriptParticleRotation(MonoObject* managedInstance, const SPtr<ParticleRotation>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleRotation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleRotation::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleRotation::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleRotation::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleRotation::InternalCreate0);

	}

	MonoObject* ScriptParticleRotation::Create(const SPtr<ParticleRotation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleRotation>()) ScriptParticleRotation(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleRotation::InternalSetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* options)
	{
		PARTICLE_ROTATION_DESC tmpoptions;
		tmpoptions = ScriptParticleRotationOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleRotation::InternalGetOptions(ScriptParticleRotation* self, __PARTICLE_ROTATION_DESCInterop* __output)
	{
		PARTICLE_ROTATION_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_ROTATION_DESCInterop interop__output;
		interop__output = ScriptParticleRotationOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRotationOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleRotation::InternalCreate(MonoObject* managedInstance, __PARTICLE_ROTATION_DESCInterop* desc)
	{
		PARTICLE_ROTATION_DESC tmpdesc;
		tmpdesc = ScriptParticleRotationOptions::FromInterop(*desc);
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleRotation>())ScriptParticleRotation(managedInstance, nativeObject);
	}

	void ScriptParticleRotation::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create();
		new (B3DAllocate<ScriptParticleRotation>())ScriptParticleRotation(managedInstance, nativeObject);
	}
}
