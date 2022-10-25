//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTonemappingSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTonemappingSettings::ScriptTonemappingSettings(MonoObject* managedInstance, const SPtr<TonemappingSettings>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptTonemappingSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TonemappingSettings", (void*)&ScriptTonemappingSettings::InternalTonemappingSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveShoulderStrength);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveShoulderStrength);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearStrength);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearStrength);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearAngle);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeStrength);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeStrength);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeNumerator);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeNumerator);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeDenominator);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeDenominator);
		metaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearWhitePoint);
		metaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearWhitePoint);
	}

	MonoObject* ScriptTonemappingSettings::Create(const SPtr<TonemappingSettings>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptTonemappingSettings>()) ScriptTonemappingSettings(managedInstance, value);
		return managedInstance;
	}

	void ScriptTonemappingSettings::InternalTonemappingSettings(MonoObject* managedInstance)
	{
		SPtr<TonemappingSettings> instance = bs_shared_ptr_new<TonemappingSettings>();
		new(bs_alloc<ScriptTonemappingSettings>()) ScriptTonemappingSettings(managedInstance, instance);
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveShoulderStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveShoulderStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveLinearStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveLinearStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveLinearAngle;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveLinearAngle = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveToeStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveToeStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveToeNumerator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveToeNumerator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveToeDenominator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveToeDenominator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FilmicCurveLinearWhitePoint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FilmicCurveLinearWhitePoint = value;
	}
} // namespace bs
