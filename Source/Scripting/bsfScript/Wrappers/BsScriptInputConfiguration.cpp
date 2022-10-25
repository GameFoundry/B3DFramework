//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptInputConfiguration.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsVirtualInput.h"

namespace bs
{
	Map<u64, ScriptInputConfiguration*> ScriptInputConfiguration::ScriptInputConfigurations;

	ScriptInputConfiguration::ScriptInputConfiguration(MonoObject* instance, const SPtr<InputConfiguration>& inputConfig)
		: ScriptObject(instance), mInputConfig(inputConfig)
	{
		mGCHandle = MonoUtil::NewWeakGcHandle(instance);

		u64 configId = (u64)inputConfig.get();
		ScriptInputConfigurations[configId] = this;
	}

	void ScriptInputConfiguration::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptInputConfiguration::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_RegisterButton", (void*)&ScriptInputConfiguration::InternalRegisterButton);
		metaData.ScriptClass->AddInternalCall("Internal_UnregisterButton", (void*)&ScriptInputConfiguration::InternalUnregisterButton);
		metaData.ScriptClass->AddInternalCall("Internal_RegisterAxis", (void*)&ScriptInputConfiguration::InternalRegisterAxis);
		metaData.ScriptClass->AddInternalCall("Internal_UnregisterAxis", (void*)&ScriptInputConfiguration::InternalUnregisterAxis);
		metaData.ScriptClass->AddInternalCall("Internal_SetRepeatInterval", (void*)&ScriptInputConfiguration::InternalSetRepeatInterval);
		metaData.ScriptClass->AddInternalCall("Internal_GetRepeatInterval", (void*)&ScriptInputConfiguration::InternalGetRepeatInterval);
	}

	MonoObject* ScriptInputConfiguration::GetManagedInstance() const
	{
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);
	}

	ScriptInputConfiguration* ScriptInputConfiguration::GetScriptInputConfig(const SPtr<InputConfiguration>& inputConfig)
	{
		u64 configId = (u64)inputConfig.get();

		auto iterFind = ScriptInputConfigurations.find(configId);
		if(iterFind != ScriptInputConfigurations.end())
			return iterFind->second;

		return nullptr;
	}

	ScriptInputConfiguration* ScriptInputConfiguration::CreateScriptInputConfig(const SPtr<InputConfiguration>& inputConfig)
	{
		MonoObject* instance = metaData.ScriptClass->CreateInstance(false);

		ScriptInputConfiguration* nativeInstance = new(bs_alloc<ScriptInputConfiguration>()) ScriptInputConfiguration(instance, inputConfig);
		return nativeInstance;
	}

	void ScriptInputConfiguration::InternalCreateInstance(MonoObject* object)
	{
		SPtr<InputConfiguration> inputConfig = VirtualInput::CreateConfiguration();

		new(bs_alloc<ScriptInputConfiguration>()) ScriptInputConfiguration(object, inputConfig);
	}

	void ScriptInputConfiguration::InternalRegisterButton(ScriptInputConfiguration* thisPtr, MonoString* name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	{
		String nameStr = MonoUtil::MonoToString(name);

		thisPtr->GetInternalValue()->RegisterButton(nameStr, buttonCode, modifiers, repeatable);
	}

	void ScriptInputConfiguration::InternalUnregisterButton(ScriptInputConfiguration* thisPtr, MonoString* name)
	{
		String nameStr = MonoUtil::MonoToString(name);

		thisPtr->GetInternalValue()->UnregisterButton(nameStr);
	}

	void ScriptInputConfiguration::InternalRegisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name, InputAxis type, float deadZone, float sensitivity, bool invert)
	{
		String nameStr = MonoUtil::MonoToString(name);

		VIRTUAL_AXIS_DESC axisDesc;
		axisDesc.Type = (u32)type;
		axisDesc.DeadZone = deadZone;
		axisDesc.Invert = invert;
		axisDesc.Sensitivity = sensitivity;

		thisPtr->GetInternalValue()->RegisterAxis(nameStr, axisDesc);
	}

	void ScriptInputConfiguration::InternalUnregisterAxis(ScriptInputConfiguration* thisPtr, MonoString* name)
	{
		String nameStr = MonoUtil::MonoToString(name);

		thisPtr->GetInternalValue()->UnregisterAxis(nameStr);
	}

	void ScriptInputConfiguration::InternalSetRepeatInterval(ScriptInputConfiguration* thisPtr, u64 milliseconds)
	{
		thisPtr->GetInternalValue()->SetRepeatInterval(milliseconds);
	}

	u64 ScriptInputConfiguration::InternalGetRepeatInterval(ScriptInputConfiguration* thisPtr)
	{
		return thisPtr->GetInternalValue()->GetRepeatInterval();
	}

	void ScriptInputConfiguration::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
	{
		u64 configId = (u64)mInputConfig.get();
		ScriptInputConfigurations.erase(configId);

		ScriptObject::OnManagedInstanceDeletedInternal(assemblyRefresh);
	}

	ScriptVirtualAxis::ScriptVirtualAxis(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptVirtualAxis::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_InitVirtualAxis", (void*)&ScriptVirtualAxis::InternalInitVirtualAxis);
	}

	u32 ScriptVirtualAxis::InternalInitVirtualAxis(MonoString* name)
	{
		String nameStr = MonoUtil::MonoToString(name);

		VirtualAxis vb(nameStr);
		return vb.AxisIdentifier;
	}
} // namespace bs
