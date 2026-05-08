//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterCircleShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleCircleShapeSettings.generated.h"
#include "B3DScriptParticleEmitterCircleShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterCircleShape::ScriptParticleEmitterCircleShape(const TShared<ParticleEmitterCircleShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterCircleShape::~ScriptParticleEmitterCircleShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterCircleShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterCircleShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterCircleShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterCircleShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterCircleShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterCircleShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterCircleShape::InternalSetSettings(ScriptParticleEmitterCircleShape* self, __ParticleCircleShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleCircleShapeSettings tmpsettings;
		tmpsettings = ScriptParticleCircleShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterCircleShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterCircleShape::InternalGetSettings(ScriptParticleEmitterCircleShape* self, __ParticleCircleShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleCircleShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterCircleShape*>(self->GetNativeObject())->GetSettings();

		__ParticleCircleShapeSettingsInterop interop__output;
		interop__output = ScriptParticleCircleShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleCircleShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterCircleShape::InternalCreate(MonoObject* scriptObject, __ParticleCircleShapeSettingsInterop* settings)
	{
		ParticleCircleShapeSettings tmpsettings;
		tmpsettings = ScriptParticleCircleShapeSettings::FromInterop(*settings);
		TShared<ParticleEmitterCircleShape> nativeObject = ParticleEmitterCircleShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterCircleShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterCircleShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterCircleShape> nativeObject = ParticleEmitterCircleShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterCircleShape>(nativeObject, scriptObject);
	}
}
