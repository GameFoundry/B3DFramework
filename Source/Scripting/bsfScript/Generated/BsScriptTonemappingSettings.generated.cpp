//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		new (B3DAllocate<ScriptTonemappingSettings>()) ScriptTonemappingSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptTonemappingSettings::InternalTonemappingSettings(MonoObject* managedInstance)
	{
		SPtr<TonemappingSettings> nativeObject = B3DMakeShared<TonemappingSettings>();
		new (B3DAllocate<ScriptTonemappingSettings>())ScriptTonemappingSettings(managedInstance, nativeObject);
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveShoulderStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveShoulderStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveLinearStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearStrength(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveLinearStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearAngle(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveLinearAngle;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearAngle(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveLinearAngle = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveToeStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeStrength(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveToeStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeNumerator(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveToeNumerator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeNumerator(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveToeNumerator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeDenominator(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveToeDenominator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeDenominator(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveToeDenominator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FilmicCurveLinearWhitePoint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self, float value)
	{
		self->GetInternal()->FilmicCurveLinearWhitePoint = value;
	}
}
