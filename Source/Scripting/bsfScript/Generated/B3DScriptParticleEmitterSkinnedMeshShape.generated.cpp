//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterSkinnedMeshShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleSkinnedMeshShapeSettings.generated.h"
#include "B3DScriptParticleEmitterSkinnedMeshShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterSkinnedMeshShape::ScriptParticleEmitterSkinnedMeshShape(const TShared<ParticleEmitterSkinnedMeshShape>& nativeObject)
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterSkinnedMeshShape::InternalGetSettings);
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
	void ScriptParticleEmitterSkinnedMeshShape::InternalSetSettings(ScriptParticleEmitterSkinnedMeshShape* self, __ParticleSkinnedMeshShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleSkinnedMeshShapeSettings tmpsettings;
		tmpsettings = ScriptParticleSkinnedMeshShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterSkinnedMeshShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalGetSettings(ScriptParticleEmitterSkinnedMeshShape* self, __ParticleSkinnedMeshShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleSkinnedMeshShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterSkinnedMeshShape*>(self->GetNativeObject())->GetSettings();

		__ParticleSkinnedMeshShapeSettingsInterop interop__output;
		interop__output = ScriptParticleSkinnedMeshShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleSkinnedMeshShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate(MonoObject* scriptObject, __ParticleSkinnedMeshShapeSettingsInterop* settings)
	{
		ParticleSkinnedMeshShapeSettings tmpsettings;
		tmpsettings = ScriptParticleSkinnedMeshShapeSettings::FromInterop(*settings);
		TShared<ParticleEmitterSkinnedMeshShape> nativeObject = ParticleEmitterSkinnedMeshShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterSkinnedMeshShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterSkinnedMeshShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterSkinnedMeshShape> nativeObject = ParticleEmitterSkinnedMeshShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterSkinnedMeshShape>(nativeObject, scriptObject);
	}
}
