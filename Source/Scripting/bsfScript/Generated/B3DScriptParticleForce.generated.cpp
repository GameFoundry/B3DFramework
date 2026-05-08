//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleForce.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleForceSettings.generated.h"
#include "B3DScriptParticleForce.generated.h"

namespace b3d
{
	ScriptParticleForce::ScriptParticleForce(const TShared<ParticleForce>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleForce::~ScriptParticleForce()
	{
		UnregisterEvents();
	}

	void ScriptParticleForce::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleForce::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleForce::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleForce::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleForce::InternalCreate0);

	}

	MonoObject* ScriptParticleForce::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleForce::InternalSetSettings(ScriptParticleForce* self, __ParticleForceSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleForceSettings tmpsettings;
		tmpsettings = ScriptParticleForceSettings::FromInterop(*settings);
		static_cast<ParticleForce*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleForce::InternalGetSettings(ScriptParticleForce* self, __ParticleForceSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleForceSettings tmp__output;
		tmp__output = static_cast<ParticleForce*>(self->GetNativeObject())->GetSettings();

		__ParticleForceSettingsInterop interop__output;
		interop__output = ScriptParticleForceSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleForceSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleForce::InternalCreate(MonoObject* scriptObject, __ParticleForceSettingsInterop* settings)
	{
		ParticleForceSettings tmpsettings;
		tmpsettings = ScriptParticleForceSettings::FromInterop(*settings);
		TShared<ParticleForce> nativeObject = ParticleForce::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleForce>(nativeObject, scriptObject);
	}

	void ScriptParticleForce::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleForce> nativeObject = ParticleForce::Create();
		ScriptObjectWrapper::Create<ScriptParticleForce>(nativeObject, scriptObject);
	}
}
