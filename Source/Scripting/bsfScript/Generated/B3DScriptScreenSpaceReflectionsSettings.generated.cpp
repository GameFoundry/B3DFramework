//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptScreenSpaceReflectionsSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptScreenSpaceReflectionsSettings::ScriptScreenSpaceReflectionsSettings(const TShared<ScreenSpaceReflectionsSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptScreenSpaceReflectionsSettings::~ScriptScreenSpaceReflectionsSettings()
	{
		UnregisterEvents();
	}

	void ScriptScreenSpaceReflectionsSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenSpaceReflectionsSettings", (void*)&ScriptScreenSpaceReflectionsSettings::InternalScreenSpaceReflectionsSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::InternalGetMaxRoughness);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxRoughness", (void*)&ScriptScreenSpaceReflectionsSettings::InternalSetMaxRoughness);

	}

	MonoObject* ScriptScreenSpaceReflectionsSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptScreenSpaceReflectionsSettings::InternalScreenSpaceReflectionsSettings(MonoObject* scriptObject)
	{
		TShared<ScreenSpaceReflectionsSettings> nativeObject = B3DMakeShared<ScreenSpaceReflectionsSettings>();
		ScriptObjectWrapper::Create<ScriptScreenSpaceReflectionsSettings>(nativeObject, scriptObject);
	}

	bool ScriptScreenSpaceReflectionsSettings::InternalGetEnabled(ScriptScreenSpaceReflectionsSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetEnabled(ScriptScreenSpaceReflectionsSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Enabled = value;
	}

	uint32_t ScriptScreenSpaceReflectionsSettings::InternalGetQuality(ScriptScreenSpaceReflectionsSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetQuality(ScriptScreenSpaceReflectionsSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Quality = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetIntensity(ScriptScreenSpaceReflectionsSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetIntensity(ScriptScreenSpaceReflectionsSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->Intensity = value;
	}

	float ScriptScreenSpaceReflectionsSettings::InternalGetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->MaxRoughness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceReflectionsSettings::InternalSetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScreenSpaceReflectionsSettings*>(self->GetNativeObject())->MaxRoughness = value;
	}
}
