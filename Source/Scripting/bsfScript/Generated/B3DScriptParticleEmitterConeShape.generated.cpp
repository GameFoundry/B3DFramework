//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleEmitterConeShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleConeShapeSettings.generated.h"
#include "B3DScriptParticleEmitterConeShape.generated.h"

namespace b3d
{
	ScriptParticleEmitterConeShape::ScriptParticleEmitterConeShape(const TShared<ParticleEmitterConeShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleEmitterConeShape::~ScriptParticleEmitterConeShape()
	{
		UnregisterEvents();
	}

	void ScriptParticleEmitterConeShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleEmitterConeShape::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleEmitterConeShape::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleEmitterConeShape::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleEmitterConeShape::InternalCreate0);

	}

	MonoObject* ScriptParticleEmitterConeShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleEmitterConeShape::InternalSetSettings(ScriptParticleEmitterConeShape* self, __ParticleConeShapeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleConeShapeSettings tmpsettings;
		tmpsettings = ScriptParticleConeShapeSettings::FromInterop(*settings);
		static_cast<ParticleEmitterConeShape*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleEmitterConeShape::InternalGetSettings(ScriptParticleEmitterConeShape* self, __ParticleConeShapeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleConeShapeSettings tmp__output;
		tmp__output = static_cast<ParticleEmitterConeShape*>(self->GetNativeObject())->GetSettings();

		__ParticleConeShapeSettingsInterop interop__output;
		interop__output = ScriptParticleConeShapeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleConeShapeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleEmitterConeShape::InternalCreate(MonoObject* scriptObject, __ParticleConeShapeSettingsInterop* settings)
	{
		ParticleConeShapeSettings tmpsettings;
		tmpsettings = ScriptParticleConeShapeSettings::FromInterop(*settings);
		TShared<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleEmitterConeShape>(nativeObject, scriptObject);
	}

	void ScriptParticleEmitterConeShape::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleEmitterConeShape> nativeObject = ParticleEmitterConeShape::Create();
		ScriptObjectWrapper::Create<ScriptParticleEmitterConeShape>(nativeObject, scriptObject);
	}
}
