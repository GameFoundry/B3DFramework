//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradingSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptColorGradingSettings::ScriptColorGradingSettings(const SPtr<ColorGradingSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptColorGradingSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSaturation", (void*)&ScriptColorGradingSettings::InternalGetSaturation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSaturation", (void*)&ScriptColorGradingSettings::InternalSetSaturation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContrast", (void*)&ScriptColorGradingSettings::InternalGetContrast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContrast", (void*)&ScriptColorGradingSettings::InternalSetContrast);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGain", (void*)&ScriptColorGradingSettings::InternalGetGain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGain", (void*)&ScriptColorGradingSettings::InternalSetGain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOffset", (void*)&ScriptColorGradingSettings::InternalGetOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOffset", (void*)&ScriptColorGradingSettings::InternalSetOffset);

	}

	MonoObject* ScriptColorGradingSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptColorGradingSettings::InternalGetSaturation(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		tmp__output = static_cast<ColorGradingSettings*>(self->GetNativeObject())->Saturation;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetSaturation(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradingSettings*>(self->GetNativeObject())->Saturation = *value;
	}

	void ScriptColorGradingSettings::InternalGetContrast(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		tmp__output = static_cast<ColorGradingSettings*>(self->GetNativeObject())->Contrast;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetContrast(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradingSettings*>(self->GetNativeObject())->Contrast = *value;
	}

	void ScriptColorGradingSettings::InternalGetGain(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		tmp__output = static_cast<ColorGradingSettings*>(self->GetNativeObject())->Gain;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetGain(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradingSettings*>(self->GetNativeObject())->Gain = *value;
	}

	void ScriptColorGradingSettings::InternalGetOffset(ScriptColorGradingSettings* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		tmp__output = static_cast<ColorGradingSettings*>(self->GetNativeObject())->Offset;

		*__output = tmp__output;


	}

	void ScriptColorGradingSettings::InternalSetOffset(ScriptColorGradingSettings* self, TVector3<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ColorGradingSettings*>(self->GetNativeObject())->Offset = *value;
	}
}
