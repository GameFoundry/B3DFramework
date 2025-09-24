//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEmitterBoxShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleBoxShapeSettings.generated.h"
#include "BsScriptParticleEmitterBoxShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterBoxShape::ScriptParticleEmitterBoxShape(const SPtr<ParticleEmitterBoxShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterBoxShape::~ScriptParticleEmitterBoxShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterBoxShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterBoxShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterBoxShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterBoxShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterBoxShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterBoxShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterBoxShape::InternalSetSettings(ScriptParticleEmitterBoxShape* self, __ParticleBoxShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleBoxShapeSettings tmpsettings;
		tmpsettings = ScriptParticleBoxShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterBoxShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterBoxShape::InternalGetSettings(ScriptParticleEmitterBoxShape* self, __ParticleBoxShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleBoxShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterBoxShape*>(self->GetNativeObject())->GetSettings();

		__ParticleBoxShapeSettingsInterop interop__output;
		interop__output = ScriptParticleBoxShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleBoxShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterBoxShape::InternalCreate(MonoObject* scriptObject, __ParticleBoxShapeSettingsInterop* settings)
	{
		ParticleBoxShapeSettings tmpsettings;
		tmpsettings = ScriptParticleBoxShapeSettings::FromInterop(*settings);
		SPtr<ParticleEmitterBoxShape> nativeObject = ParticleEmitterBoxShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterBoxShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterBoxShape::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleEmitterBoxShape> nativeObject = ParticleEmitterBoxShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterBoxShape>(nativeObject, scriptObject);
	}
}
