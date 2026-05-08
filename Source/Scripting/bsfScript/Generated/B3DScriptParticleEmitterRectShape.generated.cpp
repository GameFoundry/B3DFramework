//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterRectShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleRectangleShapeSettings.generated.h"
#include "B3DScriptParticleEmitterRectShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterRectShape::ScriptParticleEmitterRectShape(const TShared<ParticleEmitterRectShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterRectShape::~ScriptParticleEmitterRectShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterRectShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterRectShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterRectShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterRectShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterRectShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterRectShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterRectShape::InternalSetSettings(ScriptParticleEmitterRectShape* self, __ParticleRectangleShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleRectangleShapeSettings tmpsettings;
		tmpsettings = ScriptParticleRectShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterRectShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterRectShape::InternalGetSettings(ScriptParticleEmitterRectShape* self, __ParticleRectangleShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleRectangleShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterRectShape*>(self->GetNativeObject())->GetSettings();

		__ParticleRectangleShapeSettingsInterop interop__output;
		interop__output = ScriptParticleRectShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRectShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterRectShape::InternalCreate(MonoObject* scriptObject, __ParticleRectangleShapeSettingsInterop* settings)
	{
		ParticleRectangleShapeSettings tmpsettings;
		tmpsettings = ScriptParticleRectShapeSettings::FromInterop(*settings);
		TShared<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterRectShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterRectShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterRectShape> nativeObject = ParticleEmitterRectShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterRectShape>(nativeObject, scriptObject);
	}
}
