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
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleVelocity::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setOptions", (void*)&ScriptParticleVelocity::Internal_setOptions);
		metaData.scriptClass->AddInternalCall("Internal_getOptions", (void*)&ScriptParticleVelocity::Internal_getOptions);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptParticleVelocity::Internal_create);
		metaData.scriptClass->AddInternalCall("Internal_create0", (void*)&ScriptParticleVelocity::Internal_create0);

	}

	MonoObject* ScriptParticleVelocity::create(const SPtr<ParticleVelocity>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleVelocity>()) ScriptParticleVelocity(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleVelocity::Internal_setOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* options)
	{
		PARTICLE_VELOCITY_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_VELOCITY_DESC::fromInterop(*options);
		thisPtr->GetInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleVelocity::Internal_getOptions(ScriptParticleVelocity* thisPtr, __PARTICLE_VELOCITY_DESCInterop* __output)
	{
		PARTICLE_VELOCITY_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->getOptions();

		__PARTICLE_VELOCITY_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_VELOCITY_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_VELOCITY_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleVelocity::Internal_create(MonoObject* managedInstance, __PARTICLE_VELOCITY_DESCInterop* desc)
	{
		PARTICLE_VELOCITY_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_VELOCITY_DESC::fromInterop(*desc);
		SPtr<ParticleVelocity> instance = ParticleVelocity::create(tmpdesc);
		new (bs_alloc<ScriptParticleVelocity>())ScriptParticleVelocity(managedInstance, instance);
	}

	void ScriptParticleVelocity::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleVelocity> instance = ParticleVelocity::create();
		new (bs_alloc<ScriptParticleVelocity>())ScriptParticleVelocity(managedInstance, instance);
	}
}
