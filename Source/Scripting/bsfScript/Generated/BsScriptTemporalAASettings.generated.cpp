//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTemporalAASettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTemporalAASettings::ScriptTemporalAASettings(MonoObject* managedInstance, const SPtr<TemporalAASettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptTemporalAASettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_TemporalAASettings", (void*)&ScriptTemporalAASettings::Internal_TemporalAASettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptTemporalAASettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptTemporalAASettings::Internal_setenabled);

	}

	MonoObject* ScriptTemporalAASettings::create(const SPtr<TemporalAASettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptTemporalAASettings>()) ScriptTemporalAASettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptTemporalAASettings::Internal_TemporalAASettings(MonoObject* managedInstance)
	{
		SPtr<TemporalAASettings> instance = bs_shared_ptr_new<TemporalAASettings>();
		new (bs_alloc<ScriptTemporalAASettings>())ScriptTemporalAASettings(managedInstance, instance);
	}

	bool ScriptTemporalAASettings::Internal_getenabled(ScriptTemporalAASettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::Internal_setenabled(ScriptTemporalAASettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}
}
