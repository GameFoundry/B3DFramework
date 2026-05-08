//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleSize.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleSizeSettings.generated.h"
#include "B3DScriptParticleSize.generated.h"

namespace b3d
{
	ScriptParticleSize::ScriptParticleSize(const TShared<ParticleSize>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleSize::~ScriptParticleSize()
	{
		UnregisterEvents();
	}

	void ScriptParticleSize::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleSize::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleSize::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleSize::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleSize::InternalCreate0);

	}

	MonoObject* ScriptParticleSize::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleSize::InternalSetSettings(ScriptParticleSize* self, __ParticleSizeSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleSizeSettings tmpsettings;
		tmpsettings = ScriptParticleSizeSettings::FromInterop(*settings);
		static_cast<ParticleSize*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleSize::InternalGetSettings(ScriptParticleSize* self, __ParticleSizeSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleSizeSettings tmp__output;
		tmp__output = static_cast<ParticleSize*>(self->GetNativeObject())->GetSettings();

		__ParticleSizeSettingsInterop interop__output;
		interop__output = ScriptParticleSizeSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleSizeSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleSize::InternalCreate(MonoObject* scriptObject, __ParticleSizeSettingsInterop* settings)
	{
		ParticleSizeSettings tmpsettings;
		tmpsettings = ScriptParticleSizeSettings::FromInterop(*settings);
		TShared<ParticleSize> nativeObject = ParticleSize::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleSize>(nativeObject, scriptObject);
	}

	void ScriptParticleSize::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleSize> nativeObject = ParticleSize::Create();
		ScriptObjectWrapper::Create<ScriptParticleSize>(nativeObject, scriptObject);
	}
}
