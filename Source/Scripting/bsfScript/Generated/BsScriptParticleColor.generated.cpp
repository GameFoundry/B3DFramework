//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleColor.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_COLOR_DESC.generated.h"
#include "BsScriptParticleColor.generated.h"

namespace bs
{
	ScriptParticleColor::ScriptParticleColor(MonoObject* managedInstance, const SPtr<ParticleColor>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleColor::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleColor::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleColor::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleColor::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleColor::Internal_create0);

	}

	MonoObject* ScriptParticleColor::create(const SPtr<ParticleColor>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleColor>()) ScriptParticleColor(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleColor::Internal_setOptions(ScriptParticleColor* thisPtr, __PARTICLE_COLOR_DESCInterop* options)
	{
		PARTICLE_COLOR_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_COLOR_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleColor::Internal_getOptions(ScriptParticleColor* thisPtr, __PARTICLE_COLOR_DESCInterop* __output)
	{
		PARTICLE_COLOR_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_COLOR_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_COLOR_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_COLOR_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleColor::Internal_create(MonoObject* managedInstance, __PARTICLE_COLOR_DESCInterop* desc)
	{
		PARTICLE_COLOR_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_COLOR_DESC::fromInterop(*desc);
		SPtr<ParticleColor> instance = ParticleColor::create(tmpdesc);
		new (bs_alloc<ScriptParticleColor>())ScriptParticleColor(managedInstance, instance);
	}

	void ScriptParticleColor::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleColor> instance = ParticleColor::create();
		new (bs_alloc<ScriptParticleColor>())ScriptParticleColor(managedInstance, instance);
	}
}
