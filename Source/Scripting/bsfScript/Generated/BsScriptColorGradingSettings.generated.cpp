//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradingSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptColorGradingSettings::ScriptColorGradingSettings(MonoObject* managedInstance, const SPtr<ColorGradingSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptColorGradingSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetSaturation", (void*)&ScriptColorGradingSettings::InternalGetSaturation);
		metaData.ScriptClass->AddInternalCall("Internal_SetSaturation", (void*)&ScriptColorGradingSettings::InternalSetSaturation);
		metaData.ScriptClass->AddInternalCall("Internal_GetContrast", (void*)&ScriptColorGradingSettings::InternalGetContrast);
		metaData.ScriptClass->AddInternalCall("Internal_SetContrast", (void*)&ScriptColorGradingSettings::InternalSetContrast);
		metaData.ScriptClass->AddInternalCall("Internal_GetGain", (void*)&ScriptColorGradingSettings::InternalGetGain);
		metaData.ScriptClass->AddInternalCall("Internal_SetGain", (void*)&ScriptColorGradingSettings::InternalSetGain);
		metaData.ScriptClass->AddInternalCall("Internal_GetOffset", (void*)&ScriptColorGradingSettings::InternalGetOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetOffset", (void*)&ScriptColorGradingSettings::InternalSetOffset);

	}

	MonoObject* ScriptColorGradingSettings::Create(const SPtr<ColorGradingSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptColorGradingSettings>()) ScriptColorGradingSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptColorGradingSettings::InternalGetSaturation(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Saturation;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetSaturation(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Saturation = *value;
	}

	void ScriptColorGradingSettings::InternalGetContrast(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Contrast;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetContrast(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Contrast = *value;
	}

	void ScriptColorGradingSettings::InternalGetGain(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Gain;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetGain(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Gain = *value;
	}

	void ScriptColorGradingSettings::InternalGetOffset(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Offset;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetOffset(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		self->GetInternal()->Offset = *value;
	}
}
