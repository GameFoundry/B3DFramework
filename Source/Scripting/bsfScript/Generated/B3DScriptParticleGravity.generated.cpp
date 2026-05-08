//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleGravity.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleGravitySettings.generated.h"
#include "B3DScriptParticleGravity.generated.h"

namespace b3d
{
	ScriptParticleGravity::ScriptParticleGravity(const TShared<ParticleGravity>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleGravity::~ScriptParticleGravity()
	{
		UnregisterEvents();
	}

	void ScriptParticleGravity::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleGravity::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleGravity::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleGravity::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleGravity::InternalCreate0);

	}

	MonoObject* ScriptParticleGravity::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleGravity::InternalSetSettings(ScriptParticleGravity* self, ParticleGravitySettings* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ParticleGravity*>(self->GetNativeObject())->SetSettings(*settings);
	}

	void ScriptParticleGravity::InternalGetSettings(ScriptParticleGravity* self, ParticleGravitySettings* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleGravitySettings tmp__output;
		tmp__output = static_cast<ParticleGravity*>(self->GetNativeObject())->GetSettings();

		*__output = tmp__output;
	}

	void ScriptParticleGravity::InternalCreate(MonoObject* scriptObject, ParticleGravitySettings* settings)
	{
		TShared<ParticleGravity> nativeObject = ParticleGravity::Create(*settings);
		ScriptObjectWrapper::Create<ScriptParticleGravity>(nativeObject, scriptObject);
	}

	void ScriptParticleGravity::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleGravity> nativeObject = ParticleGravity::Create();
		ScriptObjectWrapper::Create<ScriptParticleGravity>(nativeObject, scriptObject);
	}
}
