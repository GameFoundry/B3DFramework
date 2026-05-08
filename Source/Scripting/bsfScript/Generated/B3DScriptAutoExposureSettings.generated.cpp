//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAutoExposureSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptAutoExposureSettings::ScriptAutoExposureSettings(const TShared<AutoExposureSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAutoExposureSettings::~ScriptAutoExposureSettings()
	{
		UnregisterEvents();
	}

	void ScriptAutoExposureSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AutoExposureSettings", (void*)&ScriptAutoExposureSettings::InternalAutoExposureSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHistogramLog2Min", (void*)&ScriptAutoExposureSettings::InternalGetHistogramLog2Min);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHistogramLog2Min", (void*)&ScriptAutoExposureSettings::InternalSetHistogramLog2Min);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHistogramLog2Max", (void*)&ScriptAutoExposureSettings::InternalGetHistogramLog2Max);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHistogramLog2Max", (void*)&ScriptAutoExposureSettings::InternalSetHistogramLog2Max);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHistogramPctLow", (void*)&ScriptAutoExposureSettings::InternalGetHistogramPctLow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHistogramPctLow", (void*)&ScriptAutoExposureSettings::InternalSetHistogramPctLow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHistogramPctHigh", (void*)&ScriptAutoExposureSettings::InternalGetHistogramPctHigh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHistogramPctHigh", (void*)&ScriptAutoExposureSettings::InternalSetHistogramPctHigh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalGetMinEyeAdaptation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMinEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalSetMinEyeAdaptation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalGetMaxEyeAdaptation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalSetMaxEyeAdaptation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedDown);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedDown);

	}

	MonoObject* ScriptAutoExposureSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAutoExposureSettings::InternalAutoExposureSettings(MonoObject* scriptObject)
	{
		TShared<AutoExposureSettings> nativeObject = B3DMakeShared<AutoExposureSettings>();
		ScriptObjectWrapper::Create<ScriptAutoExposureSettings>(nativeObject, scriptObject);
	}

	float ScriptAutoExposureSettings::InternalGetHistogramLog2Min(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramLog2Min;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramLog2Min(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramLog2Min = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramLog2Max(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramLog2Max;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramLog2Max(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramLog2Max = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramPctLow(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramPctLow;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramPctLow(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramPctLow = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramPctHigh(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramPctHigh;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramPctHigh(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->HistogramPctHigh = value;
	}

	float ScriptAutoExposureSettings::InternalGetMinEyeAdaptation(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->MinEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetMinEyeAdaptation(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->MinEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::InternalGetMaxEyeAdaptation(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->MaxEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetMaxEyeAdaptation(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->MaxEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->EyeAdaptationSpeedUp;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->EyeAdaptationSpeedUp = value;
	}

	float ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AutoExposureSettings*>(self->GetNativeObject())->EyeAdaptationSpeedDown;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AutoExposureSettings*>(self->GetNativeObject())->EyeAdaptationSpeedDown = value;
	}
}
