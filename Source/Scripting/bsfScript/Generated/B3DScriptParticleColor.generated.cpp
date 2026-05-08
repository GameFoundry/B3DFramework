//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptParticleColor.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptParticleColorSettings.generated.h"
#include "B3DScriptParticleColor.generated.h"

namespace b3d
{
	ScriptParticleColor::ScriptParticleColor(const TShared<ParticleColor>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptParticleColor::~ScriptParticleColor()
	{
		UnregisterEvents();
	}

	void ScriptParticleColor::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSettings", (void*)&ScriptParticleColor::InternalSetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSettings", (void*)&ScriptParticleColor::InternalGetSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptParticleColor::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptParticleColor::InternalCreate0);

	}

	MonoObject* ScriptParticleColor::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptParticleColor::InternalSetSettings(ScriptParticleColor* self, __ParticleColorSettingsInterop* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		ParticleColorSettings tmpsettings;
		tmpsettings = ScriptParticleColorOptions::FromInterop(*settings);
		static_cast<ParticleColor*>(self->GetNativeObject())->SetSettings(tmpsettings);
	}

	void ScriptParticleColor::InternalGetSettings(ScriptParticleColor* self, __ParticleColorSettingsInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		ParticleColorSettings tmp__output;
		tmp__output = static_cast<ParticleColor*>(self->GetNativeObject())->GetSettings();

		__ParticleColorSettingsInterop interop__output;
		interop__output = ScriptParticleColorOptions::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptParticleColorOptions::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptParticleColor::InternalCreate(MonoObject* scriptObject, __ParticleColorSettingsInterop* settings)
	{
		ParticleColorSettings tmpsettings;
		tmpsettings = ScriptParticleColorOptions::FromInterop(*settings);
		TShared<ParticleColor> nativeObject = ParticleColor::Create(tmpsettings);
		ScriptObjectWrapper::Create<ScriptParticleColor>(nativeObject, scriptObject);
	}

	void ScriptParticleColor::InternalCreate0(MonoObject* scriptObject)
	{
		TShared<ParticleColor> nativeObject = ParticleColor::Create();
		ScriptObjectWrapper::Create<ScriptParticleColor>(nativeObject, scriptObject);
	}
}
