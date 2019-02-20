//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterRectShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_RECT_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterRectShape.generated.h"

namespace bs
{
	ScriptParticleEmitterRectShape::ScriptParticleEmitterRectShape(MonoObject* managedInstance, const SPtr<ParticleEmitterRectShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterRectShape::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterRectShape::Internal_setOptions);
		metaData.scriptClass->addInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterRectShape::Internal_getOptions);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptParticleEmitterRectShape::Internal_create);
		metaData.scriptClass->addInternalCall("Internal_create0", (void*)&ScriptParticleEmitterRectShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterRectShape::create(const SPtr<ParticleEmitterRectShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterRectShape>()) ScriptParticleEmitterRectShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterRectShape::Internal_setOptions(ScriptParticleEmitterRectShape* thisPtr, __PARTICLE_RECT_SHAPE_DESCInterop* options)
	{
		PARTICLE_RECT_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_RECT_SHAPE_DESC::fromInterop(*options);
		thisPtr->getInternal()->setOptions(tmpoptions);
	}

	void ScriptParticleEmitterRectShape::Internal_getOptions(ScriptParticleEmitterRectShape* thisPtr, __PARTICLE_RECT_SHAPE_DESCInterop* __output)
	{
		PARTICLE_RECT_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->getInternal()->getOptions();

		__PARTICLE_RECT_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_RECT_SHAPE_DESC::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptPARTICLE_RECT_SHAPE_DESC::getMetaData()->scriptClass->_getInternalClass());
	}

	void ScriptParticleEmitterRectShape::Internal_create(MonoObject* managedInstance, __PARTICLE_RECT_SHAPE_DESCInterop* desc)
	{
		PARTICLE_RECT_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_RECT_SHAPE_DESC::fromInterop(*desc);
		SPtr<ParticleEmitterRectShape> instance = ParticleEmitterRectShape::create(tmpdesc);
		new (bs_alloc<ScriptParticleEmitterRectShape>())ScriptParticleEmitterRectShape(managedInstance, instance);
	}

	void ScriptParticleEmitterRectShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterRectShape> instance = ParticleEmitterRectShape::create();
		new (bs_alloc<ScriptParticleEmitterRectShape>())ScriptParticleEmitterRectShape(managedInstance, instance);
	}
}
