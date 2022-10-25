//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		: TScriptReflectable(managedInstance, value)
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
		new(bs_alloc<ScriptParticleSize>()) ScriptParticleSize(managedInstance, value);
		return managedInstance;
	}

	void ScriptParticleSize::InternalSetOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* options)
	{
		PARTICLE_SIZE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_SIZE_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleSize::InternalGetOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* __output)
	{
		PARTICLE_SIZE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_SIZE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_SIZE_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_SIZE_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleSize::InternalCreate(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc)
	{
		PARTICLE_SIZE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_SIZE_DESC::FromInterop(*desc);
		SPtr<ParticleSize> instance = ParticleSize::Create(tmpdesc);
		new(bs_alloc<ScriptParticleSize>()) ScriptParticleSize(managedInstance, instance);
	}

	void ScriptParticleSize::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleSize> instance = ParticleSize::Create();
		new(bs_alloc<ScriptParticleSize>()) ScriptParticleSize(managedInstance, instance);
	}
} // namespace bs
