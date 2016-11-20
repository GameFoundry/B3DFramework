//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptEditorVirtualInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsVirtualInput.h"
#include "BsScriptVirtualButton.h"

namespace bs
{
	ScriptEditorVirtualInput::OnButtonEventThunkDef ScriptEditorVirtualInput::OnButtonUpThunk;
	ScriptEditorVirtualInput::OnButtonEventThunkDef ScriptEditorVirtualInput::OnButtonDownThunk;
	ScriptEditorVirtualInput::OnButtonEventThunkDef ScriptEditorVirtualInput::OnButtonHeldThunk;

	HEvent ScriptEditorVirtualInput::OnButtonPressedConn;
	HEvent ScriptEditorVirtualInput::OnButtonReleasedConn;
	HEvent ScriptEditorVirtualInput::OnButtonHeldConn;

	ScriptEditorVirtualInput::ScriptEditorVirtualInput(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorVirtualInput::initRuntimeData()
	{
		OnButtonUpThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonDown", "VirtualButton,int")->getThunk();
		OnButtonDownThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonUp", "VirtualButton,int")->getThunk();
		OnButtonHeldThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonHeld", "VirtualButton,int")->getThunk();
	}

	void ScriptEditorVirtualInput::startUp()
	{
		VirtualInput& input = VirtualInput::instance();

		OnButtonPressedConn = input.onButtonDown.connect(&ScriptEditorVirtualInput::onButtonDown);
		OnButtonReleasedConn = input.onButtonUp.connect(&ScriptEditorVirtualInput::onButtonUp);
		OnButtonHeldConn = input.onButtonHeld.connect(&ScriptEditorVirtualInput::onButtonHeld);
	}

	void ScriptEditorVirtualInput::shutDown()
	{
		OnButtonPressedConn.disconnect();
		OnButtonReleasedConn.disconnect();
		OnButtonHeldConn.disconnect();
	}

	void ScriptEditorVirtualInput::onButtonDown(const VirtualButton& btn, UINT32 deviceIdx)
	{
		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonDownThunk, virtualButton, deviceIdx);
	}

	void ScriptEditorVirtualInput::onButtonUp(const VirtualButton& btn, UINT32 deviceIdx)
	{
		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonUpThunk, virtualButton, deviceIdx);
	}

	void ScriptEditorVirtualInput::onButtonHeld(const VirtualButton& btn, UINT32 deviceIdx)
	{
		MonoObject* virtualButton = ScriptVirtualButton::box(btn);
		MonoUtil::invokeThunk(OnButtonHeldThunk, virtualButton, deviceIdx);
	}
}