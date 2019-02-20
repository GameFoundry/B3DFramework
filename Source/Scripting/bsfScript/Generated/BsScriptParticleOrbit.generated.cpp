//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptParticleOrbit::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleOrbit::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleOrbit::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleOrbit::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleOrbit::Internal_create0);

	}

	MonoObject* ScriptParticleOrbit::create(const SPtr<ParticleOrbit>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleOrbit>()) ScriptParticleOrbit(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleOrbit::Internal_setOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* options)
	{
		PARTICLE_ORBIT_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_ORBIT_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleOrbit::Internal_getOptions(ScriptParticleOrbit* thisPtr, __PARTICLE_ORBIT_DESCInterop* __output)
	{
		PARTICLE_ORBIT_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_ORBIT_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_ORBIT_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_ORBIT_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleOrbit::Internal_create(MonoObject* managedInstance, __PARTICLE_ORBIT_DESCInterop* desc)
	{
		PARTICLE_ORBIT_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_ORBIT_DESC::fromInterop(*desc);
		SPtr<ParticleOrbit> instance = ParticleOrbit::create(tmpdesc);
		new (bs_alloc<ScriptParticleOrbit>())ScriptParticleOrbit(managedInstance, instance);
	}

	void ScriptParticleOrbit::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleOrbit> instance = ParticleOrbit::create();
		new (bs_alloc<ScriptParticleOrbit>())ScriptParticleOrbit(managedInstance, instance);
	}
}
