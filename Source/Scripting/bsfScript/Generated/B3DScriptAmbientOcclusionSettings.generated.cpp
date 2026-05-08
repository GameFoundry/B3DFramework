//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAmbientOcclusionSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptAmbientOcclusionSettings::ScriptAmbientOcclusionSettings(const TShared<AmbientOcclusionSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAmbientOcclusionSettings::~ScriptAmbientOcclusionSettings()
	{
		UnregisterEvents();
	}

	void ScriptAmbientOcclusionSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AmbientOcclusionSettings", (void*)&ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptAmbientOcclusionSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptAmbientOcclusionSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRadius", (void*)&ScriptAmbientOcclusionSettings::InternalGetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRadius", (void*)&ScriptAmbientOcclusionSettings::InternalSetRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBias", (void*)&ScriptAmbientOcclusionSettings::InternalGetBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBias", (void*)&ScriptAmbientOcclusionSettings::InternalSetBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalGetFadeDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFadeDistance", (void*)&ScriptAmbientOcclusionSettings::InternalSetFadeDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalGetFadeRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFadeRange", (void*)&ScriptAmbientOcclusionSettings::InternalSetFadeRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptAmbientOcclusionSettings::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptAmbientOcclusionSettings::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPower", (void*)&ScriptAmbientOcclusionSettings::InternalGetPower);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPower", (void*)&ScriptAmbientOcclusionSettings::InternalSetPower);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptAmbientOcclusionSettings::InternalGetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptAmbientOcclusionSettings::InternalSetQuality);

	}

	MonoObject* ScriptAmbientOcclusionSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAmbientOcclusionSettings::InternalAmbientOcclusionSettings(MonoObject* scriptObject)
	{
		TShared<AmbientOcclusionSettings> nativeObject = B3DMakeShared<AmbientOcclusionSettings>();
		ScriptObjectWrapper::Create<ScriptAmbientOcclusionSettings>(nativeObject, scriptObject);
	}

	bool ScriptAmbientOcclusionSettings::InternalGetEnabled(ScriptAmbientOcclusionSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetEnabled(ScriptAmbientOcclusionSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Enabled = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetRadius(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Radius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetRadius(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Radius = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetBias(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Bias;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetBias(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Bias = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeDistance(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->FadeDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeDistance(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->FadeDistance = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetFadeRange(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->FadeRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetFadeRange(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->FadeRange = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetIntensity(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Intensity;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetIntensity(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Intensity = value;
	}

	float ScriptAmbientOcclusionSettings::InternalGetPower(ScriptAmbientOcclusionSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Power;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetPower(ScriptAmbientOcclusionSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Power = value;
	}

	uint32_t ScriptAmbientOcclusionSettings::InternalGetQuality(ScriptAmbientOcclusionSettings* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Quality;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAmbientOcclusionSettings::InternalSetQuality(ScriptAmbientOcclusionSettings* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AmbientOcclusionSettings*>(self->GetNativeObject())->Quality = value;
	}
}
