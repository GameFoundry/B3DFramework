//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptPARTICLE_STATIC_MESH_SHAPE_DESC.generated.h"
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"

namespace bs
{
	ScriptParticleEmitterStaticMeshShape::ScriptParticleEmitterStaticMeshShape(MonoObject* managedInstance, const SPtr<ParticleEmitterStaticMeshShape>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptParticleEmitterStaticMeshShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterStaticMeshShape::InternalSetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterStaticMeshShape::InternalGetOptions);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterStaticMeshShape::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterStaticMeshShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterStaticMeshShape::Create(const SPtr<ParticleEmitterStaticMeshShape>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptParticleEmitterStaticMeshShape>()) ScriptParticleEmitterStaticMeshShape(managedInstance, value);
		return managedInstance;
	}
	void ScriptParticleEmitterStaticMeshShape::InternalSetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleStaticMeshShapeOptions::FromInterop(*options);
		self->GetInternal()->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalGetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmp__output;
		tmp__output = self->GetInternal()->GetOptions();

		__PARTICLE_STATIC_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleStaticMeshShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleStaticMeshShapeOptions::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate(MonoObject* managedInstance, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleStaticMeshShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create(tmpdesc);
		new (B3DAllocate<ScriptParticleEmitterStaticMeshShape>())ScriptParticleEmitterStaticMeshShape(managedInstance, nativeObject);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate0(MonoObject* managedInstance)
	{
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create();
		new (B3DAllocate<ScriptParticleEmitterStaticMeshShape>())ScriptParticleEmitterStaticMeshShape(managedInstance, nativeObject);
	}
}
