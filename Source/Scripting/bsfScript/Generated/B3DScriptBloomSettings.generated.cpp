//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptBloomSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptColor.generated.h"

namespace b3d
{
	ScriptBloomSettings::ScriptBloomSettings(const TShared<BloomSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptBloomSettings::~ScriptBloomSettings()
	{
		UnregisterEvents();
	}

	void ScriptBloomSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BloomSettings", (void*)&ScriptBloomSettings::InternalBloomSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptBloomSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptBloomSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptBloomSettings::InternalGetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptBloomSettings::InternalSetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetThreshold", (void*)&ScriptBloomSettings::InternalGetThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetThreshold", (void*)&ScriptBloomSettings::InternalSetThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptBloomSettings::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptBloomSettings::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTint", (void*)&ScriptBloomSettings::InternalGetTint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptBloomSettings::InternalSetTint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilterSize", (void*)&ScriptBloomSettings::InternalGetFilterSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilterSize", (void*)&ScriptBloomSettings::InternalSetFilterSize);

	}

	MonoObject* ScriptBloomSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptBloomSettings::InternalBloomSettings(MonoObject* scriptObject)
	{
		TShared<BloomSettings> nativeObject = B3DMakeShared<BloomSettings>();
		ScriptObjectWrapper::Create<ScriptBloomSettings>(nativeObject, scriptObject);
	}

	bool ScriptBloomSettings::InternalGetEnabled(ScriptBloomSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetEnabled(ScriptBloomSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->Enabled = value;
	}

	uint32_t ScriptBloomSettings::InternalGetQuality(ScriptBloomSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetQuality(ScriptBloomSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->Quality = value;
	}

	float ScriptBloomSettings::InternalGetThreshold(ScriptBloomSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->Threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetThreshold(ScriptBloomSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->Threshold = value;
	}

	float ScriptBloomSettings::InternalGetIntensity(ScriptBloomSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetIntensity(ScriptBloomSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->Intensity = value;
	}

	void ScriptBloomSettings::InternalGetTint(ScriptBloomSettings* self, Color* __output)
	{
		Color tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->Tint;

		*__output = tmp__output;


	}

	void ScriptBloomSettings::InternalSetTint(ScriptBloomSettings* self, Color* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->Tint = *value;
	}

	float ScriptBloomSettings::InternalGetFilterSize(ScriptBloomSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<BloomSettings*>(self->GetNativeObject())->FilterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptBloomSettings::InternalSetFilterSize(ScriptBloomSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<BloomSettings*>(self->GetNativeObject())->FilterSize = value;
	}
}
