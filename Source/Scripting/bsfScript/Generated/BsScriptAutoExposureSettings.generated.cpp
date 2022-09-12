//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAutoExposureSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAutoExposureSettings::ScriptAutoExposureSettings(MonoObject* managedInstance, const SPtr<AutoExposureSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptAutoExposureSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_AutoExposureSettings", (void*)&ScriptAutoExposureSettings::Internal_AutoExposureSettings);
		metaData.scriptClass->AddInternalCall("Internal_gethistogramLog2Min", (void*)&ScriptAutoExposureSettings::Internal_gethistogramLog2Min);
		metaData.scriptClass->AddInternalCall("Internal_sethistogramLog2Min", (void*)&ScriptAutoExposureSettings::Internal_sethistogramLog2Min);
		metaData.scriptClass->AddInternalCall("Internal_gethistogramLog2Max", (void*)&ScriptAutoExposureSettings::Internal_gethistogramLog2Max);
		metaData.scriptClass->AddInternalCall("Internal_sethistogramLog2Max", (void*)&ScriptAutoExposureSettings::Internal_sethistogramLog2Max);
		metaData.scriptClass->AddInternalCall("Internal_gethistogramPctLow", (void*)&ScriptAutoExposureSettings::Internal_gethistogramPctLow);
		metaData.scriptClass->AddInternalCall("Internal_sethistogramPctLow", (void*)&ScriptAutoExposureSettings::Internal_sethistogramPctLow);
		metaData.scriptClass->AddInternalCall("Internal_gethistogramPctHigh", (void*)&ScriptAutoExposureSettings::Internal_gethistogramPctHigh);
		metaData.scriptClass->AddInternalCall("Internal_sethistogramPctHigh", (void*)&ScriptAutoExposureSettings::Internal_sethistogramPctHigh);
		metaData.scriptClass->AddInternalCall("Internal_getminEyeAdaptation", (void*)&ScriptAutoExposureSettings::Internal_getminEyeAdaptation);
		metaData.scriptClass->AddInternalCall("Internal_setminEyeAdaptation", (void*)&ScriptAutoExposureSettings::Internal_setminEyeAdaptation);
		metaData.scriptClass->AddInternalCall("Internal_getmaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::Internal_getmaxEyeAdaptation);
		metaData.scriptClass->AddInternalCall("Internal_setmaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::Internal_setmaxEyeAdaptation);
		metaData.scriptClass->AddInternalCall("Internal_geteyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::Internal_geteyeAdaptationSpeedUp);
		metaData.scriptClass->AddInternalCall("Internal_seteyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::Internal_seteyeAdaptationSpeedUp);
		metaData.scriptClass->AddInternalCall("Internal_geteyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::Internal_geteyeAdaptationSpeedDown);
		metaData.scriptClass->AddInternalCall("Internal_seteyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::Internal_seteyeAdaptationSpeedDown);

	}

	MonoObject* ScriptAutoExposureSettings::create(const SPtr<AutoExposureSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptAutoExposureSettings>()) ScriptAutoExposureSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptAutoExposureSettings::Internal_AutoExposureSettings(MonoObject* managedInstance)
	{
		SPtr<AutoExposureSettings> instance = bs_shared_ptr_new<AutoExposureSettings>();
		new (bs_alloc<ScriptAutoExposureSettings>())ScriptAutoExposureSettings(managedInstance, instance);
	}

	float ScriptAutoExposureSettings::Internal_gethistogramLog2Min(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->histogramLog2Min;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_sethistogramLog2Min(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->histogramLog2Min = value;
	}

	float ScriptAutoExposureSettings::Internal_gethistogramLog2Max(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->histogramLog2Max;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_sethistogramLog2Max(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->histogramLog2Max = value;
	}

	float ScriptAutoExposureSettings::Internal_gethistogramPctLow(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->histogramPctLow;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_sethistogramPctLow(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->histogramPctLow = value;
	}

	float ScriptAutoExposureSettings::Internal_gethistogramPctHigh(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->histogramPctHigh;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_sethistogramPctHigh(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->histogramPctHigh = value;
	}

	float ScriptAutoExposureSettings::Internal_getminEyeAdaptation(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->minEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_setminEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->minEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::Internal_getmaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->maxEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_setmaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->maxEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::Internal_geteyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->eyeAdaptationSpeedUp;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_seteyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->eyeAdaptationSpeedUp = value;
	}

	float ScriptAutoExposureSettings::Internal_geteyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->eyeAdaptationSpeedDown;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::Internal_seteyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->eyeAdaptationSpeedDown = value;
	}
}
