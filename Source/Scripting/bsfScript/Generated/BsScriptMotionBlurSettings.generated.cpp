//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMotionBlurSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptMotionBlurSettings::ScriptMotionBlurSettings(MonoObject* managedInstance, const SPtr<MotionBlurSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptMotionBlurSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_MotionBlurSettings", (void*)&ScriptMotionBlurSettings::Internal_MotionBlurSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptMotionBlurSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptMotionBlurSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getdomain", (void*)&ScriptMotionBlurSettings::Internal_getdomain);
		metaData.scriptClass->addInternalCall("Internal_setdomain", (void*)&ScriptMotionBlurSettings::Internal_setdomain);
		metaData.scriptClass->addInternalCall("Internal_getfilter", (void*)&ScriptMotionBlurSettings::Internal_getfilter);
		metaData.scriptClass->addInternalCall("Internal_setfilter", (void*)&ScriptMotionBlurSettings::Internal_setfilter);
		metaData.scriptClass->addInternalCall("Internal_getquality", (void*)&ScriptMotionBlurSettings::Internal_getquality);
		metaData.scriptClass->addInternalCall("Internal_setquality", (void*)&ScriptMotionBlurSettings::Internal_setquality);
		metaData.scriptClass->addInternalCall("Internal_getmaximumRadius", (void*)&ScriptMotionBlurSettings::Internal_getmaximumRadius);
		metaData.scriptClass->addInternalCall("Internal_setmaximumRadius", (void*)&ScriptMotionBlurSettings::Internal_setmaximumRadius);

	}

	MonoObject* ScriptMotionBlurSettings::create(const SPtr<MotionBlurSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptMotionBlurSettings>()) ScriptMotionBlurSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptMotionBlurSettings::Internal_MotionBlurSettings(MonoObject* managedInstance)
	{
		SPtr<MotionBlurSettings> instance = bs_shared_ptr_new<MotionBlurSettings>();
		new (bs_alloc<ScriptMotionBlurSettings>())ScriptMotionBlurSettings(managedInstance, instance);
	}

	bool ScriptMotionBlurSettings::Internal_getenabled(ScriptMotionBlurSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::Internal_setenabled(ScriptMotionBlurSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	MotionBlurDomain ScriptMotionBlurSettings::Internal_getdomain(ScriptMotionBlurSettings* thisPtr)
	{
		MotionBlurDomain tmp__output;
		tmp__output = thisPtr->getInternal()->domain;

		MotionBlurDomain __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::Internal_setdomain(ScriptMotionBlurSettings* thisPtr, MotionBlurDomain value)
	{
		thisPtr->getInternal()->domain = value;
	}

	MotionBlurFilter ScriptMotionBlurSettings::Internal_getfilter(ScriptMotionBlurSettings* thisPtr)
	{
		MotionBlurFilter tmp__output;
		tmp__output = thisPtr->getInternal()->filter;

		MotionBlurFilter __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::Internal_setfilter(ScriptMotionBlurSettings* thisPtr, MotionBlurFilter value)
	{
		thisPtr->getInternal()->filter = value;
	}

	MotionBlurQuality ScriptMotionBlurSettings::Internal_getquality(ScriptMotionBlurSettings* thisPtr)
	{
		MotionBlurQuality tmp__output;
		tmp__output = thisPtr->getInternal()->quality;

		MotionBlurQuality __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::Internal_setquality(ScriptMotionBlurSettings* thisPtr, MotionBlurQuality value)
	{
		thisPtr->getInternal()->quality = value;
	}

	float ScriptMotionBlurSettings::Internal_getmaximumRadius(ScriptMotionBlurSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->maximumRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::Internal_setmaximumRadius(ScriptMotionBlurSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->maximumRadius = value;
	}
}
