//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptVirtualInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsVirtualInput.h"
#include "BsScriptVirtualButton.h"
#include "BsScriptInputConfiguration.h"
#include "BsPlayInEditorManager.h"

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

	void ScriptVirtualInput::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetKeyConfig", &ScriptVirtualInput::internal_getKeyConfig);
		metaData.scriptClass->addInternalCall("Internal_SetKeyConfig", &ScriptVirtualInput::internal_setKeyConfig);
		metaData.scriptClass->addInternalCall("Internal_IsButtonHeld", &ScriptVirtualInput::internal_isButtonHeld);
		metaData.scriptClass->addInternalCall("Internal_IsButtonDown", &ScriptVirtualInput::internal_isButtonDown);
		metaData.scriptClass->addInternalCall("Internal_IsButtonUp", &ScriptVirtualInput::internal_isButtonUp);
		metaData.scriptClass->addInternalCall("Internal_GetAxisValue", &ScriptVirtualInput::internal_getAxisValue);

		OnButtonUpThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonDown", "VirtualButton,int")->getThunk();
		OnButtonDownThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonUp", "VirtualButton,int")->getThunk();
		OnButtonHeldThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonHeld", "VirtualButton,int")->getThunk();
	}

	void ScriptVirtualInput::startUp()
	{
		VirtualInput& input = VirtualInput::instance();

		OnButtonPressedConn = input.onButtonDown.connect(&ScriptVirtualInput::onButtonDown);
		OnButtonReleasedConn = input.onButtonUp.connect(&ScriptVirtualInput::onButtonUp);
		OnButtonHeldConn = input.onButtonHeld.connect(&ScriptVirtualInput::onButtonHeld);
	}

	void ScriptVirtualInput::shutDown()
	{
		OnButtonPressedConn.disconnect();
		OnButtonReleasedConn.disconnect();
		OnButtonHeldConn.disconnect();
	}

	void ScriptVirtualInput::onButtonDown(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditorManager::instance().getState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonDownThunk, virtualButton, deviceIdx);
	}

	void ScriptVirtualInput::onButtonUp(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditorManager::instance().getState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonUpThunk, virtualButton, deviceIdx);
	}

	void ScriptVirtualInput::onButtonHeld(const VirtualButton& btn, UINT32 deviceIdx)
	{
		if (PlayInEditorManager::instance().getState() != PlayInEditorState::Playing)
			return;

		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonHeldThunk, virtualButton, deviceIdx);
	}

	MonoObject* ScriptVirtualInput::internal_getKeyConfig()
	{
		SPtr<InputConfiguration> inputConfig = VirtualInput::instance().getConfiguration();

		ScriptInputConfiguration* scriptInputConfig = ScriptInputConfiguration::getScriptInputConfig(inputConfig);
		if (scriptInputConfig == nullptr)
			scriptInputConfig = ScriptInputConfiguration::createScriptInputConfig(inputConfig);

		return scriptInputConfig->getManagedInstance();
	}

	void ScriptVirtualInput::internal_setKeyConfig(MonoObject* keyConfig)
	{
		ScriptInputConfiguration* inputConfig = ScriptInputConfiguration::toNative(keyConfig);

		VirtualInput::instance().setConfiguration(inputConfig->getInternalValue());
	}

	bool ScriptVirtualInput::internal_isButtonHeld(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::instance().isButtonHeld(*btn, deviceIdx);
	}

	bool ScriptVirtualInput::internal_isButtonDown(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::instance().isButtonDown(*btn, deviceIdx);
	}

	bool ScriptVirtualInput::internal_isButtonUp(VirtualButton* btn, UINT32 deviceIdx)
	{
		return VirtualInput::instance().isButtonUp(*btn, deviceIdx);
	}

	float ScriptVirtualInput::internal_getAxisValue(VirtualAxis* axis, UINT32 deviceIdx)
	{
		return VirtualInput::instance().getAxisValue(*axis, deviceIdx);
	}
}