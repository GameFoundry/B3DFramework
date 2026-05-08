//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTemporalAASettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptTemporalAASettings::ScriptTemporalAASettings(const TShared<TemporalAASettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptTemporalAASettings::~ScriptTemporalAASettings()
	{
		UnregisterEvents();
	}

	void ScriptTemporalAASettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TemporalAASettings", (void*)&ScriptTemporalAASettings::InternalTemporalAASettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptTemporalAASettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptTemporalAASettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetJitteredPositionCount", (void*)&ScriptTemporalAASettings::InternalGetJitteredPositionCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetJitteredPositionCount", (void*)&ScriptTemporalAASettings::InternalSetJitteredPositionCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSharpness", (void*)&ScriptTemporalAASettings::InternalGetSharpness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSharpness", (void*)&ScriptTemporalAASettings::InternalSetSharpness);

	}

	MonoObject* ScriptTemporalAASettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptTemporalAASettings::InternalTemporalAASettings(MonoObject* scriptObject)
	{
		TShared<TemporalAASettings> nativeObject = B3DMakeShared<TemporalAASettings>();
		ScriptObjectWrapper::Create<ScriptTemporalAASettings>(nativeObject, scriptObject);
	}

	bool ScriptTemporalAASettings::InternalGetEnabled(ScriptTemporalAASettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TemporalAASettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetEnabled(ScriptTemporalAASettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TemporalAASettings*>(self->GetNativeObject())->Enabled = value;
	}

	uint32_t ScriptTemporalAASettings::InternalGetJitteredPositionCount(ScriptTemporalAASettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TemporalAASettings*>(self->GetNativeObject())->JitteredPositionCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetJitteredPositionCount(ScriptTemporalAASettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TemporalAASettings*>(self->GetNativeObject())->JitteredPositionCount = value;
	}

	float ScriptTemporalAASettings::InternalGetSharpness(ScriptTemporalAASettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TemporalAASettings*>(self->GetNativeObject())->Sharpness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTemporalAASettings::InternalSetSharpness(ScriptTemporalAASettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<TemporalAASettings*>(self->GetNativeObject())->Sharpness = value;
	}
}
