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
	ScriptParticleEmitterStaticMeshShape::ScriptParticleEmitterStaticMeshShape(const SPtr<ParticleEmitterStaticMeshShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitterStaticMeshShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterStaticMeshShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterStaticMeshShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterStaticMeshShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterStaticMeshShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterStaticMeshShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterStaticMeshShape::InternalSetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* options)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpoptions;
		tmpoptions = ScriptParticleStaticMeshShapeOptions::FromInterop(*options);
		static_cast<ParticleEmitterStaticMeshShape*>(self->GetNativeObject())->SetOptions(tmpoptions);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalGetOptions(ScriptParticleEmitterStaticMeshShape* self, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* __output)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterStaticMeshShape*>(self->GetNativeObject())->GetOptions();

		__PARTICLE_STATIC_MESH_SHAPE_DESCInterop interop__output;
		interop__output = ScriptParticleStaticMeshShapeOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleStaticMeshShapeOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate(MonoObject* scriptObject, __PARTICLE_STATIC_MESH_SHAPE_DESCInterop* desc)
	{
		PARTICLE_STATIC_MESH_SHAPE_DESC tmpdesc;
		tmpdesc = ScriptParticleStaticMeshShapeOptions::FromInterop(*desc);
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create(tmpdesc);
		B3DNew<ScriptParticleEmitterStaticMeshShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create();
		B3DNew<ScriptParticleEmitterStaticMeshShape>(nativeObject, scriptObject);
	}
}
