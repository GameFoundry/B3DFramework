//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	ScriptParticleEmitterShapeBase::ScriptParticleEmitterShapeBase(MonoObject* managedInstance)
		:ScriptObjectBase(managedInstance)
	 { }

	ScriptParticleEmitterShape::ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value)
		:ScriptObject(managedInstance)
	{
		mInternal = value;
	}

	SPtr<ParticleEmitterShape> ScriptParticleEmitterShape::getInternal() const 
	{
		return std::static_pointer_cast<ParticleEmitterShape>(mInternal);
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
