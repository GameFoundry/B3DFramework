//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleOrbit.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_ORBIT_DESC.generated.h"
#include "BsScriptParticleOrbit.generated.h"

namespace bs
{
	ScriptParticleOrbit::ScriptParticleOrbit(MonoObject* managedInstance, const SPtr<ParticleOrbit>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleOrbit::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleOrbit::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleOrbit::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleOrbit::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleOrbit::InternalCreate0);

	}

	MonoObject* ScriptParticleOrbit::Create(const SPtr<ParticleOrbit>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleOrbit>()) ScriptParticleOrbit(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleOrbit::InternalSetOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* options)
	{
		PARTICLE_ORBIT_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_ORBIT_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleOrbit::InternalGetOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* __output)
	{
		PARTICLE_ORBIT_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_ORBIT_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_ORBIT_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_ORBIT_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleOrbit::InternalCreate(MonoObject* managedInstance, __PARTICLE_ORBIT_DESCInterop* desc)
	{
		PARTICLE_ORBIT_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_ORBIT_DESC::FromInterop(*desc);
		SPtr<ParticleOrbit> instance = ParticleOrbit::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleOrbit>())ScriptParticleOrbit(managedInstance, instance);
	}

	void ScriptParticleOrbit::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleOrbit> instance = ParticleOrbit::Create();
		new (B3DAllocate<ScriptParticleOrbit>())ScriptParticleOrbit(managedInstance, instance);
	}
}
