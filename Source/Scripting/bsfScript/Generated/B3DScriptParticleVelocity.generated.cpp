//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleVelocity.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleVelocitySettings.generated.h"
#include "B3DScriptParticleVelocity.generated.h"

namespace b3d
{
	ScriptParticleVelocity::ScriptParticleVelocity(const TShared<ParticleVelocity>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleVelocity::~ScriptParticleVelocity()
	{
		UnregisterEvents();
	}

	void ScriptParticleVelocity::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleVelocity::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleVelocity::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleVelocity::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleVelocity::InternalCreate0);

	}

	MonoObject* ScriptParticleVelocity::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleVelocity::InternalSetSettings(ScriptParticleVelocity* self, __ParticleVelocitySettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleVelocitySettings tmpsettings;
		tmpsettings = ScriptParticleVelocitySettings::FromInterop(*settings);
		static_cast<ParticleVelocity*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleVelocity::InternalGetSettings(ScriptParticleVelocity* self, __ParticleVelocitySettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleVelocitySettings tmp__output;
		tmp__output = static_cast<ParticleVelocity*>(self->GetNativeObject())->GetSettings();

		__ParticleVelocitySettingsInterop interop__output;
		interop__output = ScriptParticleVelocitySettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleVelocitySettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleVelocity::InternalCreate(MonoObject* scriptObject, __ParticleVelocitySettingsInterop* settings)
	{
		ParticleVelocitySettings tmpsettings;
		tmpsettings = ScriptParticleVelocitySettings::FromInterop(*settings);
		TShared<ParticleVelocity> nativeObject = ParticleVelocity::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleVelocity>(nativeObject, scriptObject);
	}

	void ScriptParticleVelocity::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleVelocity> nativeObject = ParticleVelocity::Create();
		ScriptObjectWrapper::Create<ScriptParticleVelocity>(nativeObject, scriptObject);
	}
}
