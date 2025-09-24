//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleRotation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptParticleRotationSettings.generated.h"
#include "BsScriptParticleRotation.generated.h"

namespace b3d
{
	ScriptParticleRotation::ScriptParticleRotation(const SPtr<ParticleRotation>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleRotation::~ScriptParticleRotation()
	{
		UnregisterEvents();
	}

	void ScriptParticleRotation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleRotation::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleRotation::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleRotation::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleRotation::InternalCreate0);

	}

	MonoObject* ScriptParticleRotation::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleRotation::InternalSetSettings(ScriptParticleRotation* self, __ParticleRotationSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleRotationSettings tmpsettings;
		tmpsettings = ScriptParticleRotationSettings::FromInterop(*settings);
		static_cast<ParticleRotation*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleRotation::InternalGetSettings(ScriptParticleRotation* self, __ParticleRotationSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleRotationSettings tmp__output;
		tmp__output = static_cast<ParticleRotation*>(self->GetNativeObject())->GetSettings();

		__ParticleRotationSettingsInterop interop__output;
		interop__output = ScriptParticleRotationSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleRotationSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleRotation::InternalCreate(MonoObject* scriptObject, __ParticleRotationSettingsInterop* settings)
	{
		ParticleRotationSettings tmpsettings;
		tmpsettings = ScriptParticleRotationSettings::FromInterop(*settings);
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleRotation>(nativeObject, scriptObject);
	}

	void ScriptParticleRotation::InternalCreate0(MonoObject* scriptObject)
	{
		SPtr<ParticleRotation> nativeObject = ParticleRotation::Create();
		ScriptObjectWrapper::Create<ScriptParticleRotation>(nativeObject, scriptObject);
	}
}
