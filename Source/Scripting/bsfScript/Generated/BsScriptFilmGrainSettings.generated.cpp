//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFilmGrainSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptFilmGrainSettings::ScriptFilmGrainSettings(MonoObject* managedInstance, const SPtr<FilmGrainSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptFilmGrainSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_FilmGrainSettings", (void*)&ScriptFilmGrainSettings::InternalFilmGrainSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptFilmGrainSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptFilmGrainSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptFilmGrainSettings::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptFilmGrainSettings::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptFilmGrainSettings::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpeed", (void*)&ScriptFilmGrainSettings::InternalSetSpeed);

	}

	MonoObject* ScriptFilmGrainSettings::Create(const SPtr<FilmGrainSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptFilmGrainSettings>()) ScriptFilmGrainSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptFilmGrainSettings::InternalFilmGrainSettings(MonoObject* managedInstance)
	{
		SPtr<FilmGrainSettings> nativeObject = B3DMakeShared<FilmGrainSettings>();
		new (B3DAllocate<ScriptFilmGrainSettings>())ScriptFilmGrainSettings(managedInstance, nativeObject);
	}

	bool ScriptFilmGrainSettings::InternalGetEnabled(ScriptFilmGrainSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetEnabled(ScriptFilmGrainSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	float ScriptFilmGrainSettings::InternalGetIntensity(ScriptFilmGrainSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetIntensity(ScriptFilmGrainSettings* self, float value)
	{
		self->GetInternal()->Intensity = value;
	}

	float ScriptFilmGrainSettings::InternalGetSpeed(ScriptFilmGrainSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Speed;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetSpeed(ScriptFilmGrainSettings* self, float value)
	{
		self->GetInternal()->Speed = value;
	}
}
