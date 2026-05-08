//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleOrbit.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleOrbitSettings.generated.h"
#include "B3DScriptParticleOrbit.generated.h"

namespace b3d
{
	ScriptParticleOrbit::ScriptParticleOrbit(const TShared<ParticleOrbit>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleOrbit::~ScriptParticleOrbit()
	{
		UnregisterEvents();
	}

	void ScriptParticleOrbit::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleOrbit::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleOrbit::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleOrbit::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleOrbit::InternalCreate0);

	}

	MonoObject* ScriptParticleOrbit::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleOrbit::InternalSetSettings(ScriptParticleOrbit* self, __ParticleOrbitSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleOrbitSettings tmpsettings;
		tmpsettings = ScriptParticleOrbitSettings::FromInterop(*settings);
		static_cast<ParticleOrbit*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleOrbit::InternalGetSettings(ScriptParticleOrbit* self, __ParticleOrbitSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleOrbitSettings tmp__output;
		tmp__output = static_cast<ParticleOrbit*>(self->GetNativeObject())->GetSettings();

		__ParticleOrbitSettingsInterop interop__output;
		interop__output = ScriptParticleOrbitSettings::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleOrbitSettings::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleOrbit::InternalCreate(MonoObject* scriptObject, __ParticleOrbitSettingsInterop* settings)
	{
		ParticleOrbitSettings tmpsettings;
		tmpsettings = ScriptParticleOrbitSettings::FromInterop(*settings);
		TShared<ParticleOrbit> nativeObject = ParticleOrbit::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleOrbit>(nativeObject, scriptObject);
	}

	void ScriptParticleOrbit::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleOrbit> nativeObject = ParticleOrbit::Create();
		ScriptObjectWrapper::Create<ScriptParticleOrbit>(nativeObject, scriptObject);
	}
}
