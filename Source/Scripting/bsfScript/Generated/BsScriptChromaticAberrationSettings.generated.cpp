//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_ChromaticAberrationSettings", (void*)&ScriptChromaticAberrationSettings::Internal_ChromaticAberrationSettings);
		metaData.scriptClass->AddInternalCall("Internal_getfringeTexture", (void*)&ScriptChromaticAberrationSettings::Internal_getfringeTexture);
		metaData.scriptClass->AddInternalCall("Internal_setfringeTexture", (void*)&ScriptChromaticAberrationSettings::Internal_setfringeTexture);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptChromaticAberrationSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptChromaticAberrationSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_gettype", (void*)&ScriptChromaticAberrationSettings::Internal_gettype);
		metaData.scriptClass->AddInternalCall("Internal_settype", (void*)&ScriptChromaticAberrationSettings::Internal_settype);
		metaData.scriptClass->AddInternalCall("Internal_getshiftAmount", (void*)&ScriptChromaticAberrationSettings::Internal_getshiftAmount);
		metaData.scriptClass->AddInternalCall("Internal_setshiftAmount", (void*)&ScriptChromaticAberrationSettings::Internal_setshiftAmount);

	}

	MonoObject* ScriptChromaticAberrationSettings::create(const SPtr<ChromaticAberrationSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptChromaticAberrationSettings>()) ScriptChromaticAberrationSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptChromaticAberrationSettings::Internal_ChromaticAberrationSettings(MonoObject* managedInstance)
	{
		SPtr<ChromaticAberrationSettings> instance = bs_shared_ptr_new<ChromaticAberrationSettings>();
		new (bs_alloc<ScriptChromaticAberrationSettings>())ScriptChromaticAberrationSettings(managedInstance, instance);
	}

	MonoObject* ScriptChromaticAberrationSettings::Internal_getfringeTexture(ScriptChromaticAberrationSettings* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetInternal()->fringeTexture;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptChromaticAberrationSettings::Internal_setfringeTexture(ScriptChromaticAberrationSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->fringeTexture = tmpvalue;
	}

	bool ScriptChromaticAberrationSettings::Internal_getenabled(ScriptChromaticAberrationSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::Internal_setenabled(ScriptChromaticAberrationSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	ChromaticAberrationType ScriptChromaticAberrationSettings::Internal_gettype(ScriptChromaticAberrationSettings* thisPtr)
	{
		ChromaticAberrationType tmp__output;
		tmp__output = thisPtr->GetInternal()->type;

		ChromaticAberrationType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::Internal_settype(ScriptChromaticAberrationSettings* thisPtr, ChromaticAberrationType value)
	{
		thisPtr->GetInternal()->type = value;
	}

	float ScriptChromaticAberrationSettings::Internal_getshiftAmount(ScriptChromaticAberrationSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->shiftAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptChromaticAberrationSettings::Internal_setshiftAmount(ScriptChromaticAberrationSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->shiftAmount = value;
	}
}
