//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterSphereShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_SPHERE_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterSphereShape.generated.h"

namespace bs
{
	ScriptParticleEmitterSphereShape::ScriptParticleEmitterSphereShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSphereShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterSphereShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterSphereShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterSphereShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterSphereShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterSphereShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterSphereShape::Create(const SPtr<ParticleEmitterSphereShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterSphereShape>()) ScriptParticleEmitterSphereShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterSphereShape::InternalSetOptions(ScriptParticleEmitterSphereShape* thisPtr, PARTICLE_SPHERE_SHAPE_DESC* options)
	{
		thisPtr->GetInternal()->SetOptions(*options);
	}

	void ScriptParticleEmitterSphereShape::InternalGetOptions(ScriptParticleEmitterSphereShape* thisPtr, PARTICLE_SPHERE_SHAPE_DESC* __output)
	{
		PARTICLE_SPHERE_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterSphereShape::InternalCreate(MonoObject* managedInstance, PARTICLE_SPHERE_SHAPE_DESC* desc)
	{
		SPtr<ParticleEmitterSphereShape> instance = ParticleEmitterSphereShape::Create(*desc);
		new (B3DAllocate<ScriptParticleEmitterSphereShape>())ScriptParticleEmitterSphereShape(managedInstance, instance);
	}

	void ScriptParticleEmitterSphereShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterSphereShape> instance = ParticleEmitterSphereShape::Create();
		new (B3DAllocate<ScriptParticleEmitterSphereShape>())ScriptParticleEmitterSphereShape(managedInstance, instance);
	}
}
