//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptChromaticAberrationSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

using namespace bs;
ScriptChromaticAberrationSettings::ScriptChromaticAberrationSettings(MonoObject* managedInstance, const SPtr<ChromaticAberrationSettings>& value)
	: TScriptReflectable(managedInstance, value)
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
	new(B3DAllocate<ScriptChromaticAberrationSettings>()) ScriptChromaticAberrationSettings(managedInstance, value);
	return managedInstance;
}

void ScriptChromaticAberrationSettings::InternalChromaticAberrationSettings(MonoObject* managedInstance)
{
	SPtr<ChromaticAberrationSettings> instance = B3DMakeShared<ChromaticAberrationSettings>();
	new(B3DAllocate<ScriptChromaticAberrationSettings>()) ScriptChromaticAberrationSettings(managedInstance, instance);
}

MonoObject* ScriptChromaticAberrationSettings::InternalGetFringeTexture(ScriptChromaticAberrationSettings* thisPtr)
{
	ResourceHandle<Texture> tmp__output;
	tmp__output = thisPtr->GetInternal()->FringeTexture;

	MonoObject* __output;
	ScriptRRefBase* script__output;
	script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
	if(script__output != nullptr)
		__output = script__output->GetManagedInstance();
	else
		__output = nullptr;

	return __output;
}

void ScriptChromaticAberrationSettings::InternalSetFringeTexture(ScriptChromaticAberrationSettings* thisPtr, MonoObject* value)
{
	ResourceHandle<Texture> tmpvalue;
	ScriptRRefBase* scriptvalue;
	scriptvalue = ScriptRRefBase::ToNative(value);
	if(scriptvalue != nullptr)
		tmpvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
	thisPtr->GetInternal()->FringeTexture = tmpvalue;
}

bool ScriptChromaticAberrationSettings::InternalGetEnabled(ScriptChromaticAberrationSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->Enabled;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptChromaticAberrationSettings::InternalSetEnabled(ScriptChromaticAberrationSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->Enabled = value;
}

ChromaticAberrationType ScriptChromaticAberrationSettings::InternalGetType(ScriptChromaticAberrationSettings* thisPtr)
{
	ChromaticAberrationType tmp__output;
	tmp__output = thisPtr->GetInternal()->Type;

	ChromaticAberrationType __output;
	__output = tmp__output;

	return __output;
}

void ScriptChromaticAberrationSettings::InternalSetType(ScriptChromaticAberrationSettings* thisPtr, ChromaticAberrationType value)
{
	thisPtr->GetInternal()->Type = value;
}

float ScriptChromaticAberrationSettings::InternalGetShiftAmount(ScriptChromaticAberrationSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->ShiftAmount;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptChromaticAberrationSettings::InternalSetShiftAmount(ScriptChromaticAberrationSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->ShiftAmount = value;
}
