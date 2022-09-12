//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptVirtualInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsVirtualInput.h"
#include "Wrappers/BsScriptVirtualButton.h"
#include "Wrappers/BsScriptInputConfiguration.h"
#include "BsPlayInEditor.h"

namespace bs
{
	ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonUpThunk;
	ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonDownThunk;
	ScriptVirtualInput::OnButtonEventThunkDef ScriptVirtualInput::OnButtonHeldThunk;

	HEvent ScriptVirtualInput::OnButtonPressedConn;
	HEvent ScriptVirtualInput::OnButtonReleasedConn;
	HEvent ScriptVirtualInput::OnButtonHeldConn;

	ScriptVirtualInput::ScriptVirtualInput(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVirtualInput::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetKeyConfig", (void*)&ScriptVirtualInput::internal_getKeyConfig);
		metaData.scriptClass->AddInternalCall("Internal_SetKeyConfig", (void*)&ScriptVirtualInput::internal_setKeyConfig);
		metaData.scriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptVirtualInput::internal_isButtonHeld);
		metaData.scriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptVirtualInput::internal_isButtonDown);
		metaData.scriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptVirtualInput::internal_isButtonUp);
		metaData.scriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptVirtualInput::internal_getAxisValue);

		OnButtonUpThunk = (OnButtonEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerButtonDown", "VirtualButton,int")->getThunk();
		OnButtonDownThunk = (OnButtonEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerButtonUp", "VirtualButton,int")->getThunk();
		OnButtonHeldThunk = (OnButtonEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerButtonHeld", "VirtualButton,int")->getThunk();
	}

	void ScriptVirtualInput::StartUp()
	{
		VirtualInput& input = VirtualInput::instance();

		OnButtonPressedConn = input.onButtonDown.Connect(&ScriptVirtualInput::onButtonDown);
		OnButtonReleasedConn = input.onButtonUp.Connect(&ScriptVirtualInput::onButtonUp);
		OnButtonHeldConn = input.onButtonHeld.Connect(&ScriptVirtualInput::onButtonHeld);
	}

	void ScriptVirtualInput::ShutDown()
	{
		OnButtonPressedConn.Disconnect();
		OnButtonReleasedConn.Disconnect();
		OnButtonHeldConn.Disconnect();
	}

	void ScriptVirtualInput::OnButtonDown(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonDownThunk, virtualButton, deviceIdx);
	}

	void ScriptVirtualInput::OnButtonUp(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonUpThunk, virtualButton, deviceIdx);
	}

	void ScriptVirtualInput::OnButtonHeld(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonHeldThunk, virtualButton, deviceIdx);
	}

	MonoObject* ScriptVirtualInput::internal_getKeyConfig()
	{
		SPtr<InputConfiguration> inputConfig = VirtualInput::instance().GetConfiguration();

		ScriptInputConfiguration* scriptInputConfig = ScriptInputConfiguration::getScriptInputConfig(inputConfig);
		if (scriptInputConfig == nullptr)
			scriptInputConfig = ScriptInputConfiguration::createScriptInputConfig(inputConfig);

		return scriptInputConfig->GetManagedInstance();
	}

	void ScriptVirtualInput::internal_setKeyConfig(MonoObject* keyConfig)
	{
		ScriptInputConfiguration* inputConfig = ScriptInputConfiguration::toNative(keyConfig);

		VirtualInput::instance().SetConfiguration(inputConfig->GetInternalValue());
	}

	bool ScriptVirtualInput::internal_isButtonHeld(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::Instance().IsButtonHeld(*btn, deviceIdx);
	}

	bool ScriptVirtualInput::internal_isButtonDown(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::Instance().IsButtonDown(*btn, deviceIdx);
	}

	bool ScriptVirtualInput::internal_isButtonUp(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::Instance().IsButtonUp(*btn, deviceIdx);
	}

	float ScriptVirtualInput::internal_getAxisValue(VirtualAxis* axis, UINT32 deviceIdx)
	{
		return VirtualInput::Instance().GetAxisValue(*axis, deviceIdx);
	}
}
