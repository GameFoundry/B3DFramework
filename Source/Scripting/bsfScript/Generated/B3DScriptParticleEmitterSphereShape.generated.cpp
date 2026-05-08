//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterSphereShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleSphereShapeSettings.generated.h"
#include "B3DScriptParticleEmitterSphereShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterSphereShape::ScriptParticleEmitterSphereShape(const TShared<ParticleEmitterSphereShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterSphereShape::~ScriptParticleEmitterSphereShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterSphereShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterSphereShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterSphereShape::InternalGetSettings);
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
	void ScriptParticleEmitterSphereShape::InternalSetSettings(ScriptParticleEmitterSphereShape* self, ParticleSphereShapeSettings* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitterSphereShape*>(self->GetNativeObject())->SetSettings(*settings);
	}

	void ScriptParticleEmitterSphereShape::InternalGetSettings(ScriptParticleEmitterSphereShape* self, ParticleSphereShapeSettings* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleSphereShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterSphereShape*>(self->GetNativeObject())->GetSettings();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterSphereShape::InternalCreate(MonoObject* scriptObject, ParticleSphereShapeSettings* settings)
	{
		TShared<ParticleEmitterSphereShape> nativeObject = ParticleEmitterSphereShape::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterSphereShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterSphereShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterSphereShape> nativeObject = ParticleEmitterSphereShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterSphereShape>(nativeObject, scriptObject);
	}
}
