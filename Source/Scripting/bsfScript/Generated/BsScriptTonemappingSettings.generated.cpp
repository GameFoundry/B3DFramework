//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTonemappingSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTonemappingSettings::ScriptTonemappingSettings(MonoObject* managedInstance, const SPtr<TonemappingSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptTonemappingSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_TonemappingSettings", (void*)&ScriptTonemappingSettings::Internal_TonemappingSettings);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveShoulderStrength);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveShoulderStrength);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveLinearStrength);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveLinearStrength);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveLinearAngle);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveLinearAngle);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveToeStrength);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveToeStrength);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveToeNumerator);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveToeNumerator);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveToeDenominator);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveToeDenominator);
		metaData.scriptClass->AddInternalCall("Internal_getfilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::Internal_getfilmicCurveLinearWhitePoint);
		metaData.scriptClass->AddInternalCall("Internal_setfilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::Internal_setfilmicCurveLinearWhitePoint);

	}

	MonoObject* ScriptTonemappingSettings::create(const SPtr<TonemappingSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptTonemappingSettings>()) ScriptTonemappingSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptTonemappingSettings::Internal_TonemappingSettings(MonoObject* managedInstance)
	{
		SPtr<TonemappingSettings> instance = bs_shared_ptr_new<TonemappingSettings>();
		new (bs_alloc<ScriptTonemappingSettings>())ScriptTonemappingSettings(managedInstance, instance);
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveShoulderStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveShoulderStrength = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveLinearStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveLinearStrength = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveLinearAngle;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveLinearAngle = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveToeStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveToeStrength = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveToeNumerator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveToeNumerator = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveToeDenominator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveToeDenominator = value;
	}

	float ScriptTonemappingSettings::Internal_getfilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filmicCurveLinearWhitePoint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::Internal_setfilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filmicCurveLinearWhitePoint = value;
	}
}
