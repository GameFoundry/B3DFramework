//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptChromaticAberrationSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	ScriptChromaticAberrationSettings::ScriptChromaticAberrationSettings(MonoObject* managedInstance, const SPtr<ChromaticAberrationSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptChromaticAberrationSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_ChromaticAberrationSettings", (void*)&ScriptChromaticAberrationSettings::InternalChromaticAberrationSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetFringeTexture", (void*)&ScriptChromaticAberrationSettings::InternalGetFringeTexture);
		metaData.ScriptClass->AddInternalCall("Internal_SetFringeTexture", (void*)&ScriptChromaticAberrationSettings::InternalSetFringeTexture);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptChromaticAberrationSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptChromaticAberrationSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptChromaticAberrationSettings::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptChromaticAberrationSettings::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetShiftAmount", (void*)&ScriptChromaticAberrationSettings::InternalGetShiftAmount);
		metaData.ScriptClass->AddInternalCall("Internal_SetShiftAmount", (void*)&ScriptChromaticAberrationSettings::InternalSetShiftAmount);

	}

	MonoObject* ScriptChromaticAberrationSettings::Create(const SPtr<ChromaticAberrationSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptChromaticAberrationSettings>()) ScriptChromaticAberrationSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptChromaticAberrationSettings::InternalChromaticAberrationSettings(MonoObject* managedInstance)
	{
		SPtr<ChromaticAberrationSettings> nativeObject = B3DMakeShared<ChromaticAberrationSettings>();
		new (B3DAllocate<ScriptChromaticAberrationSettings>())ScriptChromaticAberrationSettings(managedInstance, nativeObject);
	}

	MonoObject* ScriptChromaticAberrationSettings::InternalGetFringeTexture(ScriptChromaticAberrationSettings* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = self->GetInternal()->FringeTexture;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetFringeTexture(ScriptChromaticAberrationSettings* self, MonoObject* value)
	{
		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetHandle());
		self->GetInternal()->FringeTexture = tmpvalue;
	}

	bool ScriptChromaticAberrationSettings::InternalGetEnabled(ScriptChromaticAberrationSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetEnabled(ScriptChromaticAberrationSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	ChromaticAberrationType ScriptChromaticAberrationSettings::InternalGetType(ScriptChromaticAberrationSettings* self)
	{
		ChromaticAberrationType tmp__output;
		tmp__output = self->GetInternal()->Type;

		ChromaticAberrationType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetType(ScriptChromaticAberrationSettings* self, ChromaticAberrationType value)
	{
		self->GetInternal()->Type = value;
	}

	float ScriptChromaticAberrationSettings::InternalGetShiftAmount(ScriptChromaticAberrationSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->ShiftAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::InternalSetShiftAmount(ScriptChromaticAberrationSettings* self, float value)
	{
		self->GetInternal()->ShiftAmount = value;
	}
}
