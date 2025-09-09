//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterHemisphereShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleHemisphereShapeSettings.generated.h"
#include "BsScriptParticleEmitterHemisphereShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterHemisphereShape::ScriptParticleEmitterHemisphereShape(const SPtr<ParticleEmitterHemisphereShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterHemisphereShape::~ScriptParticleEmitterHemisphereShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterHemisphereShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterHemisphereShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterHemisphereShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterHemisphereShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterHemisphereShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterHemisphereShape::InternalSetSettings(ScriptParticleEmitterHemisphereShape* self, ParticleHemisphereShapeSettings* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleEmitterHemisphereShape*>(self->GetNativeObject())->SetSettings(*settings);
	}

	void ScriptParticleEmitterHemisphereShape::InternalGetSettings(ScriptParticleEmitterHemisphereShape* self, ParticleHemisphereShapeSettings* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleHemisphereShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterHemisphereShape*>(self->GetNativeObject())->GetSettings();

		*__output = tmp__output;
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate(MonoObject* scriptObject, ParticleHemisphereShapeSettings* settings)
	{
		SPtr<ParticleEmitterHemisphereShape> nativeObject = ParticleEmitterHemisphereShape::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterHemisphereShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterHemisphereShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterHemisphereShape> nativeObject = ParticleEmitterHemisphereShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterHemisphereShape>(nativeObject, scriptObject);
	}
}
