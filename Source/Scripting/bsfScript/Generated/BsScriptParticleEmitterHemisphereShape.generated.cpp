//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterHemisphereShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterHemisphereShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterHemisphereShape::Create(const SPtr<ParticleEmitterHemisphereShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterHemisphereShape>()) ScriptParticleEmitterHemisphereShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterHemisphereShape::InternalSetOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* options)
	{
		thisPtr->GetInternal()->SetOptions(*options);
	}

	void ScriptParticleEmitterHemisphereShape::InternalGetOptions(ScriptParticleEmitterHemisphereShape* thisPtr, PARTICLE_HEMISPHERE_SHAPE_DESC* __output)
	{
		PARTICLE_HEMISPHERE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate(MonoObject* managedInstance, PARTICLE_HEMISPHERE_SHAPE_DESC* desc)
	{
		SPtr<ParticleEmitterHemisphereShape> instance = ParticleEmitterHemisphereShape::Create(*desc);
		new (B3DAllocate<ScriptParticleEmitterHemisphereShape>())ScriptParticleEmitterHemisphereShape(managedInstance, instance);
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterHemisphereShape> instance = ParticleEmitterHemisphereShape::Create();
		new (B3DAllocate<ScriptParticleEmitterHemisphereShape>())ScriptParticleEmitterHemisphereShape(managedInstance, instance);
	}
}
