//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterCircleShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_CIRCLE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterCircleShape.generated.h"

namespace bs
{
	ScriptParticleEmitterCircleShape::ScriptParticleEmitterCircleShape(MonoObject* managedInstance, const SPtr<ParticleEmitterCircleShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterCircleShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterCircleShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterCircleShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterCircleShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterCircleShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterCircleShape::create(const SPtr<ParticleEmitterCircleShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterCircleShape>()) ScriptParticleEmitterCircleShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterCircleShape::Internal_setOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_CIRCLE_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterCircleShape::Internal_getOptions(ScriptParticleEmitterCircleShape* thisPtr, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_CIRCLE_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_CIRCLE_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_CIRCLE_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterCircleShape::Internal_create(MonoObject* managedInstance, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc)
	{
		PARTICLE_CIRCLE_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_CIRCLE_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterCircleShape> instance = ParticleEmitterCircleShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterCircleShape>())ScriptParticleEmitterCircleShape(managedInstance, instance);
	}

	void ScriptParticleEmitterCircleShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterCircleShape> instance = ParticleEmitterCircleShape::create();
		new (bs_alloc<ScriptParticleEmitterCircleShape>())ScriptParticleEmitterCircleShape(managedInstance, instance);
	}
}
