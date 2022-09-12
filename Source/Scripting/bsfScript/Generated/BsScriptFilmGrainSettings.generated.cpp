//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_FilmGrainSettings", (void*)&ScriptFilmGrainSettings::Internal_FilmGrainSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptFilmGrainSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptFilmGrainSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_getintensity", (void*)&ScriptFilmGrainSettings::Internal_getintensity);
		metaData.scriptClass->AddInternalCall("Internal_setintensity", (void*)&ScriptFilmGrainSettings::Internal_setintensity);
		metaData.scriptClass->AddInternalCall("Internal_getspeed", (void*)&ScriptFilmGrainSettings::Internal_getspeed);
		metaData.scriptClass->AddInternalCall("Internal_setspeed", (void*)&ScriptFilmGrainSettings::Internal_setspeed);

	}

	MonoObject* ScriptFilmGrainSettings::create(const SPtr<FilmGrainSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptFilmGrainSettings>()) ScriptFilmGrainSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptFilmGrainSettings::Internal_FilmGrainSettings(MonoObject* managedInstance)
	{
		SPtr<FilmGrainSettings> instance = bs_shared_ptr_new<FilmGrainSettings>();
		new (bs_alloc<ScriptFilmGrainSettings>())ScriptFilmGrainSettings(managedInstance, instance);
	}

	bool ScriptFilmGrainSettings::Internal_getenabled(ScriptFilmGrainSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setenabled(ScriptFilmGrainSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	float ScriptFilmGrainSettings::Internal_getintensity(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setintensity(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->intensity = value;
	}

	float ScriptFilmGrainSettings::Internal_getspeed(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->speed;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setspeed(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->speed = value;
	}
}
