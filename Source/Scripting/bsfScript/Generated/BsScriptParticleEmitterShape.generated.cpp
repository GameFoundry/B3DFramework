//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleEmitterShapeBase::ScriptParticleEmitterShapeBase(MonoObject* managedInstance)
		:ScriptReflectableBase(managedInstance)
	 { }

	SPtr<ParticleEmitterShape> ScriptParticleEmitterShapeBase::getInternal() const
	{
		return std::static_pointer_cast<ParticleEmitterShape>(mInternal);
	}
	ScriptParticleEmitterShape::ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterShape::initRuntimeData()
	{

	}

	MonoObject* ScriptParticleEmitterShape::create(const SPtr<ParticleEmitterShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterShape>()) ScriptParticleEmitterShape(managedInstance, value);
		return managedInstance;
	}
}
