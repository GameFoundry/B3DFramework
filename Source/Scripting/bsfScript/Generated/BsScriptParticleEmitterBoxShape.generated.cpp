//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterBoxShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_BOX_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterBoxShape.generated.h"

namespace bs
{
	ScriptParticleEmitterBoxShape::ScriptParticleEmitterBoxShape(MonoObject* managedInstance, const SPtr<ParticleEmitterBoxShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterBoxShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterBoxShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterBoxShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterBoxShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterBoxShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterBoxShape::create(const SPtr<ParticleEmitterBoxShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterBoxShape>()) ScriptParticleEmitterBoxShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterBoxShape::Internal_setOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* options)
	{
		PARTICLE_BOX_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_BOX_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterBoxShape::Internal_getOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* __output)
	{
		PARTICLE_BOX_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_BOX_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_BOX_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_BOX_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterBoxShape::Internal_create(MonoObject* managedInstance, __PARTICLE_BOX_SHAPE_DESCInterop* desc)
	{
		PARTICLE_BOX_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_BOX_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterBoxShape> instance = ParticleEmitterBoxShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterBoxShape>())ScriptParticleEmitterBoxShape(managedInstance, instance);
	}

	void ScriptParticleEmitterBoxShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterBoxShape> instance = ParticleEmitterBoxShape::create();
		new (bs_alloc<ScriptParticleEmitterBoxShape>())ScriptParticleEmitterBoxShape(managedInstance, instance);
	}
}
