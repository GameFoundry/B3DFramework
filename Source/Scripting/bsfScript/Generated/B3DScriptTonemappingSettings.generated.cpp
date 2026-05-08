//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTonemappingSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptTonemappingSettings::ScriptTonemappingSettings(const TShared<TonemappingSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptTonemappingSettings::~ScriptTonemappingSettings()
	{
		UnregisterEvents();
	}

	void ScriptTonemappingSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TonemappingSettings", (void*)&ScriptTonemappingSettings::InternalTonemappingSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveShoulderStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveShoulderStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveShoulderStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearAngle", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeStrength", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeStrength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeNumerator);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeNumerator", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeNumerator);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveToeDenominator);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveToeDenominator", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveToeDenominator);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::InternalGetFilmicCurveLinearWhitePoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilmicCurveLinearWhitePoint", (void*)&ScriptTonemappingSettings::InternalSetFilmicCurveLinearWhitePoint);

	}

	MonoObject* ScriptTonemappingSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptTonemappingSettings::InternalTonemappingSettings(MonoObject* scriptObject)
	{
		TShared<TonemappingSettings> nativeObject = B3DMakeShared<TonemappingSettings>();
		ScriptObjectWrapper::Create<ScriptTonemappingSettings>(nativeObject, scriptObject);
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveShoulderStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveShoulderStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearStrength(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearAngle(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearAngle;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearAngle(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearAngle = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeStrength(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeStrength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeStrength(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeStrength = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeNumerator(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeNumerator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeNumerator(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeNumerator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveToeDenominator(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeDenominator;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveToeDenominator(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveToeDenominator = value;
	}

	float ScriptTonemappingSettings::InternalGetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearWhitePoint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTonemappingSettings::InternalSetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TonemappingSettings*>(self->GetNativeObject())->FilmicCurveLinearWhitePoint = value;
	}
}
