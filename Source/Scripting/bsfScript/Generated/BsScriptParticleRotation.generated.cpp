//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptParticleRotation::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleRotation::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleRotation::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleRotation::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleRotation::Internal_create0);

	}

	MonoObject* ScriptParticleRotation::create(const SPtr<ParticleRotation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleRotation>()) ScriptParticleRotation(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleRotation::Internal_setOptions(ScriptParticleRotation* thisPtr, __PARTICLE_ROTATION_DESCInterop* options)
	{
		PARTICLE_ROTATION_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_ROTATION_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleRotation::Internal_getOptions(ScriptParticleRotation* thisPtr, __PARTICLE_ROTATION_DESCInterop* __output)
	{
		PARTICLE_ROTATION_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_ROTATION_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_ROTATION_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_ROTATION_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleRotation::Internal_create(MonoObject* managedInstance, __PARTICLE_ROTATION_DESCInterop* desc)
	{
		PARTICLE_ROTATION_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_ROTATION_DESC::fromInterop(*desc);
		SPtr<ParticleRotation> instance = ParticleRotation::create(tmpdesc);
		new (bs_alloc<ScriptParticleRotation>())ScriptParticleRotation(managedInstance, instance);
	}

	void ScriptParticleRotation::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleRotation> instance = ParticleRotation::create();
		new (bs_alloc<ScriptParticleRotation>())ScriptParticleRotation(managedInstance, instance);
	}
}
