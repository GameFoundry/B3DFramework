//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptParticleEmitterBoxShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterBoxShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterBoxShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterBoxShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterBoxShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterBoxShape::Create(const SPtr<ParticleEmitterBoxShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterBoxShape>()) ScriptParticleEmitterBoxShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterBoxShape::InternalSetOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* options)
	{
		PARTICLE_BOX_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleBoxShapeOptions::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterBoxShape::InternalGetOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* __output)
	{
		PARTICLE_BOX_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_BOX_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleBoxShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleBoxShapeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterBoxShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_BOX_SHAPE_DESCInterop* desc)
	{
		PARTICLE_BOX_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleBoxShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterBoxShape> instance = ParticleEmitterBoxShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterBoxShape>())ScriptParticleEmitterBoxShape(managedInstance, instance);
	}

	void ScriptParticleEmitterBoxShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterBoxShape> instance = ParticleEmitterBoxShape::Create();
		new (B3DAllocate<ScriptParticleEmitterBoxShape>())ScriptParticleEmitterBoxShape(managedInstance, instance);
	}
}
