//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterHemisphereShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_HEMISPHERE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterHemisphereShape.generated.h"

namespace bs
{
	ScriptParticleEmitterHemisphereShape::ScriptParticleEmitterHemisphereShape(MonoObject* managedInstance, const SPtr<ParticleEmitterHemisphereShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterHemisphereShape::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setOptions", (void*)&ScriptParticleEmitterHemisphereShape::Internal_setOptions);
		metaData.scriptClass->AddInternalCall("Internal_getOptions", (void*)&ScriptParticleEmitterHemisphereShape::Internal_getOptions);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptParticleEmitterHemisphereShape::Internal_create);
		metaData.scriptClass->AddInternalCall("Internal_create0", (void*)&ScriptParticleEmitterHemisphereShape::Internal_create0);

	}

	MonoObject* ScriptParticleEmitterHemisphereShape::create(const SPtr<ParticleEmitterHemisphereShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptParticleEmitterHemisphereShape>()) ScriptParticleEmitterHemisphereShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterHemisphereShape::Internal_setOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* options)
	{
		thisPtr->GetInternal()->setOptions(*options);
	}

	void ScriptParticleEmitterHemisphereShape::Internal_getOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* __output)
	{
		PARTICLE_HEMISPHERE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->getOptions();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterHemisphereShape::Internal_create(MonoObject* managedInstance, PARTICLE_HEMISPHERE_SHAPE_DESC* desc)
	{
		SPtr<ParticleEmitterHemisphereShape> instance = ParticleEmitterHemisphereShape::create(*desc);
		new (bs_alloc<ScriptParticleEmitterHemisphereShape>())ScriptParticleEmitterHemisphereShape(managedInstance, instance);
	}

	void ScriptParticleEmitterHemisphereShape::Internal_create0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterHemisphereShape> instance = ParticleEmitterHemisphereShape::create();
		new (bs_alloc<ScriptParticleEmitterHemisphereShape>())ScriptParticleEmitterHemisphereShape(managedInstance, instance);
	}
}
