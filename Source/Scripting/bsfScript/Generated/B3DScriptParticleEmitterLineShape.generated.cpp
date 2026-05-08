//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterLineShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleLineShapeSettings.generated.h"
#include "B3DScriptParticleEmitterLineShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterLineShape::ScriptParticleEmitterLineShape(const TShared<ParticleEmitterLineShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterLineShape::~ScriptParticleEmitterLineShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterLineShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterLineShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterLineShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterLineShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterLineShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterLineShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterLineShape::InternalSetSettings(ScriptParticleEmitterLineShape* self, __ParticleLineShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleLineShapeSettings tmpsettings;
		tmpsettings = ScriptParticleLineShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterLineShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterLineShape::InternalGetSettings(ScriptParticleEmitterLineShape* self, __ParticleLineShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleLineShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterLineShape*>(self->GetNativeObject())->GetSettings();

		__ParticleLineShapeSettingsInterop interop__output;
		interop__output = ScriptParticleLineShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleLineShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterLineShape::InternalCreate(MonoObject* scriptObject, __ParticleLineShapeSettingsInterop* settings)
	{
		ParticleLineShapeSettings tmpsettings;
		tmpsettings = ScriptParticleLineShapeSettings::FromInterop(*settings);
		TShared<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterLineShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterLineShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterLineShape> nativeObject = ParticleEmitterLineShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterLineShape>(nativeObject, scriptObject);
	}
}
