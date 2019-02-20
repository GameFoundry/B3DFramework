//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptDepthOfFieldSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptDepthOfFieldSettings::ScriptDepthOfFieldSettings(MonoObject* managedInstance, const SPtr<DepthOfFieldSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptDepthOfFieldSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_DepthOfFieldSettings", (void*)&ScriptDepthOfFieldSettings::Internal_DepthOfFieldSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptDepthOfFieldSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptDepthOfFieldSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getfocalDistance", (void*)&ScriptDepthOfFieldSettings::Internal_getfocalDistance);
		metaData.scriptClass->addInternalCall("Internal_setfocalDistance", (void*)&ScriptDepthOfFieldSettings::Internal_setfocalDistance);
		metaData.scriptClass->addInternalCall("Internal_getfocalRange", (void*)&ScriptDepthOfFieldSettings::Internal_getfocalRange);
		metaData.scriptClass->addInternalCall("Internal_setfocalRange", (void*)&ScriptDepthOfFieldSettings::Internal_setfocalRange);
		metaData.scriptClass->addInternalCall("Internal_getnearTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_getnearTransitionRange);
		metaData.scriptClass->addInternalCall("Internal_setnearTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_setnearTransitionRange);
		metaData.scriptClass->addInternalCall("Internal_getfarTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_getfarTransitionRange);
		metaData.scriptClass->addInternalCall("Internal_setfarTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_setfarTransitionRange);
		metaData.scriptClass->addInternalCall("Internal_getnearBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_getnearBlurAmount);
		metaData.scriptClass->addInternalCall("Internal_setnearBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_setnearBlurAmount);
		metaData.scriptClass->addInternalCall("Internal_getfarBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_getfarBlurAmount);
		metaData.scriptClass->addInternalCall("Internal_setfarBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_setfarBlurAmount);

	}

	MonoObject* ScriptDepthOfFieldSettings::create(const SPtr<DepthOfFieldSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptDepthOfFieldSettings>()) ScriptDepthOfFieldSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptDepthOfFieldSettings::Internal_DepthOfFieldSettings(MonoObject* managedInstance)
	{
		SPtr<DepthOfFieldSettings> instance = bs_shared_ptr_new<DepthOfFieldSettings>();
		new (bs_alloc<ScriptDepthOfFieldSettings>())ScriptDepthOfFieldSettings(managedInstance, instance);
	}

	bool ScriptDepthOfFieldSettings::Internal_getenabled(ScriptDepthOfFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setenabled(ScriptDepthOfFieldSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfocalDistance(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->focalDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfocalDistance(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->focalDistance = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfocalRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->focalRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfocalRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->focalRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->nearTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->nearTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->farTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->farTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->nearBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->nearBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->farBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->farBlurAmount = value;
	}
}
