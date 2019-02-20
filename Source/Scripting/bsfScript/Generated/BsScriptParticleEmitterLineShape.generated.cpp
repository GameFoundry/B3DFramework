//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterLineShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_LINE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterLineShape.generated.h"

namespace bs
{
	ScriptParticleEmitterLineShape::ScriptParticleEmitterLineShape(MonoObject* managedInstance, const SPtr<ParticleEmitterLineShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterLineShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterLineShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterLineShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterLineShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterLineShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterLineShape::create(const SPtr<ParticleEmitterLineShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterLineShape>()) ScriptParticleEmitterLineShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterLineShape::Internal_setOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* options)
	{
		PARTICLE_LINE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_LINE_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterLineShape::Internal_getOptions(ScriptParticleEmitterLineShape* thisPtr, __PARTICLE_LINE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_LINE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_LINE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_LINE_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_LINE_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterLineShape::Internal_create(MonoObject* managedInstance, __PARTICLE_LINE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_LINE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_LINE_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterLineShape> instance = ParticleEmitterLineShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterLineShape>())ScriptParticleEmitterLineShape(managedInstance, instance);
	}

	void ScriptParticleEmitterLineShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterLineShape> instance = ParticleEmitterLineShape::create();
		new (bs_alloc<ScriptParticleEmitterLineShape>())ScriptParticleEmitterLineShape(managedInstance, instance);
	}
}
