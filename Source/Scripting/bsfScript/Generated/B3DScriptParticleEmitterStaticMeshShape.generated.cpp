//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleStaticMeshShapeSettings.generated.h"
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterStaticMeshShape::ScriptParticleEmitterStaticMeshShape(const SPtr<ParticleEmitterStaticMeshShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterStaticMeshShape::~ScriptParticleEmitterStaticMeshShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterStaticMeshShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterStaticMeshShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterStaticMeshShape::InternalGetSettings);
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
	void ScriptParticleEmitterStaticMeshShape::InternalSetSettings(ScriptParticleEmitterStaticMeshShape* self, __ParticleStaticMeshShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleStaticMeshShapeSettings tmpsettings;
		tmpsettings = ScriptParticleStaticMeshShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterStaticMeshShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalGetSettings(ScriptParticleEmitterStaticMeshShape* self, __ParticleStaticMeshShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleStaticMeshShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterStaticMeshShape*>(self->GetNativeObject())->GetSettings();

		__ParticleStaticMeshShapeSettingsInterop interop__output;
		interop__output = ScriptParticleStaticMeshShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleStaticMeshShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate(MonoObject* scriptObject, __ParticleStaticMeshShapeSettingsInterop* settings)
	{
		ParticleStaticMeshShapeSettings tmpsettings;
		tmpsettings = ScriptParticleStaticMeshShapeSettings::FromInterop(*settings);
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterStaticMeshShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterStaticMeshShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterStaticMeshShape> nativeObject = ParticleEmitterStaticMeshShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterStaticMeshShape>(nativeObject, scriptObject);
	}
}
