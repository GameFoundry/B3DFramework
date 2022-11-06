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
		SPtr<FilmGrainSettings> instance = B3DMakeShared<FilmGrainSettings>();
		new (B3DAllocate<ScriptFilmGrainSettings>())ScriptFilmGrainSettings(managedInstance, instance);
	}

	bool ScriptFilmGrainSettings::InternalGetEnabled(ScriptFilmGrainSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetEnabled(ScriptFilmGrainSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	float ScriptFilmGrainSettings::InternalGetIntensity(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetIntensity(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Intensity = value;
	}

	float ScriptFilmGrainSettings::InternalGetSpeed(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Speed;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::InternalSetSpeed(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->Speed = value;
	}
}
