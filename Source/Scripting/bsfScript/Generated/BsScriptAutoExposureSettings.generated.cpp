//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		metaData.ScriptClass->AddInternalCall("Internal_AutoExposureSettings", (void*)&ScriptAutoExposureSettings::InternalAutoExposureSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetHistogramLog2Min", (void*)&ScriptAutoExposureSettings::InternalGetHistogramLog2Min);
		metaData.ScriptClass->AddInternalCall("Internal_SetHistogramLog2Min", (void*)&ScriptAutoExposureSettings::InternalSetHistogramLog2Min);
		metaData.ScriptClass->AddInternalCall("Internal_GetHistogramLog2Max", (void*)&ScriptAutoExposureSettings::InternalGetHistogramLog2Max);
		metaData.ScriptClass->AddInternalCall("Internal_SetHistogramLog2Max", (void*)&ScriptAutoExposureSettings::InternalSetHistogramLog2Max);
		metaData.ScriptClass->AddInternalCall("Internal_GetHistogramPctLow", (void*)&ScriptAutoExposureSettings::InternalGetHistogramPctLow);
		metaData.ScriptClass->AddInternalCall("Internal_SetHistogramPctLow", (void*)&ScriptAutoExposureSettings::InternalSetHistogramPctLow);
		metaData.ScriptClass->AddInternalCall("Internal_GetHistogramPctHigh", (void*)&ScriptAutoExposureSettings::InternalGetHistogramPctHigh);
		metaData.ScriptClass->AddInternalCall("Internal_SetHistogramPctHigh", (void*)&ScriptAutoExposureSettings::InternalSetHistogramPctHigh);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalGetMinEyeAdaptation);
		metaData.ScriptClass->AddInternalCall("Internal_SetMinEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalSetMinEyeAdaptation);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalGetMaxEyeAdaptation);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxEyeAdaptation", (void*)&ScriptAutoExposureSettings::InternalSetMaxEyeAdaptation);
		metaData.ScriptClass->AddInternalCall("Internal_GetEyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedUp);
		metaData.ScriptClass->AddInternalCall("Internal_SetEyeAdaptationSpeedUp", (void*)&ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedUp);
		metaData.ScriptClass->AddInternalCall("Internal_GetEyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedDown);
		metaData.ScriptClass->AddInternalCall("Internal_SetEyeAdaptationSpeedDown", (void*)&ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedDown);

	}

	MonoObject* ScriptAutoExposureSettings::Create(const SPtr<AutoExposureSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAutoExposureSettings>()) ScriptAutoExposureSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptAutoExposureSettings::InternalAutoExposureSettings(MonoObject* managedInstance)
	{
		SPtr<AutoExposureSettings> nativeObject = B3DMakeShared<AutoExposureSettings>();
		new (B3DAllocate<ScriptAutoExposureSettings>())ScriptAutoExposureSettings(managedInstance, nativeObject);
	}

	float ScriptAutoExposureSettings::InternalGetHistogramLog2Min(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HistogramLog2Min;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramLog2Min(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->HistogramLog2Min = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramLog2Max(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HistogramLog2Max;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramLog2Max(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->HistogramLog2Max = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramPctLow(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HistogramPctLow;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramPctLow(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->HistogramPctLow = value;
	}

	float ScriptAutoExposureSettings::InternalGetHistogramPctHigh(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->HistogramPctHigh;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetHistogramPctHigh(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->HistogramPctHigh = value;
	}

	float ScriptAutoExposureSettings::InternalGetMinEyeAdaptation(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->MinEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetMinEyeAdaptation(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->MinEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::InternalGetMaxEyeAdaptation(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->MaxEyeAdaptation;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetMaxEyeAdaptation(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->MaxEyeAdaptation = value;
	}

	float ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->EyeAdaptationSpeedUp;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->EyeAdaptationSpeedUp = value;
	}

	float ScriptAutoExposureSettings::InternalGetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->EyeAdaptationSpeedDown;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAutoExposureSettings::InternalSetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self, float value)
	{
		self->GetInternal()->EyeAdaptationSpeedDown = value;
	}
}
