//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	SPtr<ParticleEmitterShape> ScriptParticleEmitterShapeBase::GetInternal() const
	{
		return std::static_pointer_cast<ParticleEmitterShape>(mInternal);
	}
	ScriptParticleEmitterShape::ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterShape::InitRuntimeData()
	{

	}

	MonoObject* ScriptParticleEmitterShape::Create(const SPtr<ParticleEmitterShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterShape>()) ScriptParticleEmitterShape(managedInstance, value);
		return managedInstance;
	}
}
