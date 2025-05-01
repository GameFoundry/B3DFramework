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
	ScriptParticleEmitterSkinnedMeshShape::ScriptParticleEmitterSkinnedMeshShape(const SPtr<ParticleEmitterSkinnedMeshShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterSkinnedMeshShape::~ScriptParticleEmitterSkinnedMeshShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterSkinnedMeshShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterSkinnedMeshShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterSkinnedMeshShape::InternalSetOptions(ScriptParticleEmitterSkinnedMeshShape* self, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleSkinnedMeshShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterSkinnedMeshShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalGetOptions(ScriptParticleEmitterSkinnedMeshShape* self, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		PARTICLE_SKINNED_MESH_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterSkinnedMeshShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_SKINNED_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleSkinnedMeshShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleSkinnedMeshShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_SKINNED_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_SKINNED_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleSkinnedMeshShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterSkinnedMeshShape> nativeObject = ParticleEmitterSkinnedMeshShape::Create(tmpdesc);
		ScriptObjectWrapper::Create<ScriptParticleEmitterSkinnedMeshShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterSkinnedMeshShape> nativeObject = ParticleEmitterSkinnedMeshShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterSkinnedMeshShape>(nativeObject, scriptObject);
	}
}
