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
	ScriptParticleEmitterSphereShape::ScriptParticleEmitterSphereShape(const SPtr<ParticleEmitterSphereShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptParticleEmitterSphereShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOptions", (void*)&ScriptParticleEmitterSphereShape::InternalSetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOptions", (void*)&ScriptParticleEmitterSphereShape::InternalGetOptions);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterSphereShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterSphereShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterSphereShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterSphereShape::InternalSetOptions(ScriptParticleEmitterSphereShape* self, PARTICLE_SPHERE_SHAPE_DESC* options)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitterSphereShape*>(self->GetNativeObject())->SetOptions(*options);
	}

	void ScriptParticleEmitterSphereShape::InternalGetOptions(ScriptParticleEmitterSphereShape* self, PARTICLE_SPHERE_SHAPE_DESC* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		PARTICLE_SPHERE_SHAPE_DESC tmp__output;
		tmp__output = static_cast<ParticleEmitterSphereShape*>(self->GetNativeObject())->GetOptions();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterSphereShape::InternalCreate(MonoObject* scriptObject, PARTICLE_SPHERE_SHAPE_DESC* desc)
	{
		SPtr<ParticleEmitterSphereShape> nativeObject = ParticleEmitterSphereShape::Create(*desc);
		ScriptObjectWrapper::Create<ScriptParticleEmitterSphereShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterSphereShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterSphereShape> nativeObject = ParticleEmitterSphereShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterSphereShape>(nativeObject, scriptObject);
	}
}
