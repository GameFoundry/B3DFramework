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

	void ScriptFilmGrainSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_FilmGrainSettings", (void*)&ScriptFilmGrainSettings::Internal_FilmGrainSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptFilmGrainSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptFilmGrainSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getintensity", (void*)&ScriptFilmGrainSettings::Internal_getintensity);
		metaData.scriptClass->addInternalCall("Internal_setintensity", (void*)&ScriptFilmGrainSettings::Internal_setintensity);
		metaData.scriptClass->addInternalCall("Internal_getspeed", (void*)&ScriptFilmGrainSettings::Internal_getspeed);
		metaData.scriptClass->addInternalCall("Internal_setspeed", (void*)&ScriptFilmGrainSettings::Internal_setspeed);

	}

	MonoObject* ScriptFilmGrainSettings::create(const SPtr<FilmGrainSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
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
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setenabled(ScriptFilmGrainSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	float ScriptFilmGrainSettings::Internal_getintensity(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setintensity(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->intensity = value;
	}

	float ScriptFilmGrainSettings::Internal_getspeed(ScriptFilmGrainSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->speed;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFilmGrainSettings::Internal_setspeed(ScriptFilmGrainSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->speed = value;
	}
}
