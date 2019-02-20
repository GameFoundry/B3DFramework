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
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleSize::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleSize::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleSize::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleSize::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleSize::Internal_create0);

	}

	MonoObject* ScriptParticleSize::create(const SPtr<ParticleSize>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleSize>()) ScriptParticleSize(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleSize::Internal_setOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* options)
	{
		PARTICLE_SIZE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_SIZE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleSize::Internal_getOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* __output)
	{
		PARTICLE_SIZE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_SIZE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_SIZE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_SIZE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleSize::Internal_create(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc)
	{
		PARTICLE_SIZE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_SIZE_DESC::fromInterop(*desc);
		SPtr<ParticleSize> instance = ParticleSize::create(tmpdesc);
		new (bs_alloc<ScriptParticleSize>())ScriptParticleSize(managedInstance, instance);
	}

	void ScriptParticleSize::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleSize> instance = ParticleSize::create();
		new (bs_alloc<ScriptParticleSize>())ScriptParticleSize(managedInstance, instance);
	}
}
