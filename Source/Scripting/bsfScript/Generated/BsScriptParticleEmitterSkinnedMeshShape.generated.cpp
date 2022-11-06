//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_SKINNED_MESH_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"

namespace bs
{
	ScriptParticleEmitterSkinnedMeshShape::ScriptParticleEmitterSkinnedMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterSkinnedMeshShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterSkinnedMeshShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterSkinnedMeshShape::Create(const SPtr<ParticleEmitterSkinnedMeshShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterSkinnedMeshShape>()) ScriptParticleEmitterSkinnedMeshShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterSkinnedMeshShape::InternalSetOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* options)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::FromInterop(*options);
		thisPtr->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalGetOptions(ScriptParticleEmitterSkinnedMeshShape* thisPtr, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* __output)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmp__output;
		tmp__output = thisPtr->GetInternal()->GetOptions();

		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptPARTICLE_SKINNED_MESH_SHAPE_DESC::FromInterop(*desc);
		SPtr<ParticleEmitterSkinnedMeshShape> instance = ParticleEmitterSkinnedMeshShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterSkinnedMeshShape>())ScriptParticleEmitterSkinnedMeshShape(managedInstance, instance);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterSkinnedMeshShape> instance = ParticleEmitterSkinnedMeshShape::Create();
		new (B3DAllocate<ScriptParticleEmitterSkinnedMeshShape>())ScriptParticleEmitterSkinnedMeshShape(managedInstance, instance);
	}
}
