//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptFilmGrainSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptFilmGrainSettings::ScriptFilmGrainSettings(const TShared<FilmGrainSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptFilmGrainSettings::~ScriptFilmGrainSettings()
	{
		UnregisterEvents();
	}

	void ScriptFilmGrainSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_FilmGrainSettings", (void*)&ScriptFilmGrainSettings::InternalFilmGrainSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptFilmGrainSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptFilmGrainSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptFilmGrainSettings::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptFilmGrainSettings::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptFilmGrainSettings::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpeed", (void*)&ScriptFilmGrainSettings::InternalSetSpeed);

	}

	MonoObject* ScriptFilmGrainSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptFilmGrainSettings::InternalFilmGrainSettings(MonoObject* scriptObject)
	{
		TShared<FilmGrainSettings> nativeObject = B3DMakeShared<FilmGrainSettings>();
		ScriptObjectWrapper::Create<ScriptFilmGrainSettings>(nativeObject, scriptObject);
	}

	bool ScriptFilmGrainSettings::InternalGetEnabled(ScriptFilmGrainSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FilmGrainSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetEnabled(ScriptFilmGrainSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FilmGrainSettings*>(self->GetNativeObject())->Enabled = value;
	}

	float ScriptFilmGrainSettings::InternalGetIntensity(ScriptFilmGrainSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FilmGrainSettings*>(self->GetNativeObject())->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetIntensity(ScriptFilmGrainSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FilmGrainSettings*>(self->GetNativeObject())->Intensity = value;
	}

	float ScriptFilmGrainSettings::InternalGetSpeed(ScriptFilmGrainSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FilmGrainSettings*>(self->GetNativeObject())->Speed;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetSpeed(ScriptFilmGrainSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FilmGrainSettings*>(self->GetNativeObject())->Speed = value;
	}
}
