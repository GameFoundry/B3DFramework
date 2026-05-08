//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptWhiteBalanceSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptWhiteBalanceSettings::ScriptWhiteBalanceSettings(const TShared<WhiteBalanceSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptWhiteBalanceSettings::~ScriptWhiteBalanceSettings()
	{
		UnregisterEvents();
	}

	void ScriptWhiteBalanceSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WhiteBalanceSettings", (void*)&ScriptWhiteBalanceSettings::InternalWhiteBalanceSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTemperature", (void*)&ScriptWhiteBalanceSettings::InternalGetTemperature);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTemperature", (void*)&ScriptWhiteBalanceSettings::InternalSetTemperature);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTint", (void*)&ScriptWhiteBalanceSettings::InternalGetTint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptWhiteBalanceSettings::InternalSetTint);

	}

	MonoObject* ScriptWhiteBalanceSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptWhiteBalanceSettings::InternalWhiteBalanceSettings(MonoObject* scriptObject)
	{
		TShared<WhiteBalanceSettings> nativeObject = B3DMakeShared<WhiteBalanceSettings>();
		ScriptObjectWrapper::Create<ScriptWhiteBalanceSettings>(nativeObject, scriptObject);
	}

	float ScriptWhiteBalanceSettings::InternalGetTemperature(ScriptWhiteBalanceSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<WhiteBalanceSettings*>(self->GetNativeObject())->Temperature;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTemperature(ScriptWhiteBalanceSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<WhiteBalanceSettings*>(self->GetNativeObject())->Temperature = value;
	}

	float ScriptWhiteBalanceSettings::InternalGetTint(ScriptWhiteBalanceSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<WhiteBalanceSettings*>(self->GetNativeObject())->Tint;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptWhiteBalanceSettings::InternalSetTint(ScriptWhiteBalanceSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<WhiteBalanceSettings*>(self->GetNativeObject())->Tint = value;
	}
}
