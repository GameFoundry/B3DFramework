//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptChromaticAberrationSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Image/B3DTexture.h"

namespace b3d
{
	ScriptChromaticAberrationSettings::ScriptChromaticAberrationSettings(const TShared<ChromaticAberrationSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptChromaticAberrationSettings::~ScriptChromaticAberrationSettings()
	{
		UnregisterEvents();
	}

	void ScriptChromaticAberrationSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ChromaticAberrationSettings", (void*)&ScriptChromaticAberrationSettings::InternalChromaticAberrationSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFringeTexture", (void*)&ScriptChromaticAberrationSettings::InternalGetFringeTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFringeTexture", (void*)&ScriptChromaticAberrationSettings::InternalSetFringeTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptChromaticAberrationSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptChromaticAberrationSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptChromaticAberrationSettings::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptChromaticAberrationSettings::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShiftAmount", (void*)&ScriptChromaticAberrationSettings::InternalGetShiftAmount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShiftAmount", (void*)&ScriptChromaticAberrationSettings::InternalSetShiftAmount);

	}

	MonoObject* ScriptChromaticAberrationSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptChromaticAberrationSettings::InternalChromaticAberrationSettings(MonoObject* scriptObject)
	{
		TShared<ChromaticAberrationSettings> nativeObject = B3DMakeShared<ChromaticAberrationSettings>();
		ScriptObjectWrapper::Create<ScriptChromaticAberrationSettings>(nativeObject, scriptObject);
	}

	MonoObject* ScriptChromaticAberrationSettings::InternalGetFringeTexture(ScriptChromaticAberrationSettings* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->FringeTexture;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetFringeTexture(ScriptChromaticAberrationSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetNativeObject());
		static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->FringeTexture = tmpvalue;
	}

	bool ScriptChromaticAberrationSettings::InternalGetEnabled(ScriptChromaticAberrationSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetEnabled(ScriptChromaticAberrationSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->Enabled = value;
	}

	ChromaticAberrationType ScriptChromaticAberrationSettings::InternalGetType(ScriptChromaticAberrationSettings* self)
	{
		ChromaticAberrationType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->Type;

		ChromaticAberrationType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetType(ScriptChromaticAberrationSettings* self, ChromaticAberrationType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->Type = value;
	}

	float ScriptChromaticAberrationSettings::InternalGetShiftAmount(ScriptChromaticAberrationSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->ShiftAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetShiftAmount(ScriptChromaticAberrationSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ChromaticAberrationSettings*>(self->GetNativeObject())->ShiftAmount = value;
	}
}
