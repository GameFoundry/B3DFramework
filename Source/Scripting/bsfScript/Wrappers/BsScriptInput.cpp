//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsInput.h"
#include "Wrappers/BsScriptVector2I.h"
#include "BsPlayInEditor.h"

namespace bs
{
	ScriptInput::OnButtonEventThunkDef ScriptInput::OnButtonPressedThunk;
	ScriptInput::OnButtonEventThunkDef ScriptInput::OnButtonReleasedThunk;
	ScriptInput::OnCharInputEventThunkDef ScriptInput::OnCharInputThunk;
	ScriptInput::OnPointerEventThunkDef ScriptInput::OnPointerPressedThunk;
	ScriptInput::OnPointerEventThunkDef ScriptInput::OnPointerReleasedThunk;
	ScriptInput::OnPointerEventThunkDef ScriptInput::OnPointerMovedThunk;
	ScriptInput::OnPointerEventThunkDef ScriptInput::OnPointerDoubleClickThunk;

	HEvent ScriptInput::OnButtonPressedConn;
	HEvent ScriptInput::OnButtonReleasedConn;
	HEvent ScriptInput::OnCharInputConn;
	HEvent ScriptInput::OnPointerPressedConn;
	HEvent ScriptInput::OnPointerReleasedConn;
	HEvent ScriptInput::OnPointerMovedConn;
	HEvent ScriptInput::OnPointerDoubleClickConn;

	ScriptInput::ScriptInput(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptInput::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptInput::internal_isButtonHeld);
		metaData.scriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptInput::internal_isButtonDown);
		metaData.scriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptInput::internal_isButtonUp);
		metaData.scriptClass->AddInternalCall("Internal_IsPointerButtonHeld", (void*)&ScriptInput::internal_isPointerButtonHeld);
		metaData.scriptClass->AddInternalCall("Internal_IsPointerButtonDown", (void*)&ScriptInput::internal_isPointerButtonDown);
		metaData.scriptClass->AddInternalCall("Internal_IsPointerButtonUp", (void*)&ScriptInput::internal_isPointerButtonUp);
		metaData.scriptClass->AddInternalCall("Internal_IsPointerDoubleClicked", (void*)&ScriptInput::internal_isPointerDoubleClicked);
		metaData.scriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptInput::internal_getAxisValue);
		metaData.scriptClass->AddInternalCall("Internal_GetPointerPosition", (void*)&ScriptInput::internal_getPointerPosition);
		metaData.scriptClass->AddInternalCall("Internal_GetPointerDelta", (void*)&ScriptInput::internal_getPointerDelta);

		OnButtonPressedThunk = (OnButtonEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerButtonDown", "ButtonCode,int,bool")->getThunk();
		OnButtonReleasedThunk = (OnButtonEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerButtonUp", "ButtonCode,int,bool")->getThunk();
		OnCharInputThunk = (OnCharInputEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerCharInput", "int,bool")->getThunk();
		OnPointerPressedThunk = (OnPointerEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerPointerPressed", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerReleasedThunk = (OnPointerEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerPointerReleased", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerMovedThunk = (OnPointerEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerPointerMove", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerDoubleClickThunk = (OnPointerEventThunkDef)metaData.scriptClass->GetMethodExact("Internal_TriggerPointerDoubleClick", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
	}

	void ScriptInput::StartUp()
	{
		Input& input = Input::instance();

		OnButtonPressedConn = input.onButtonDown.Connect(&ScriptInput::onButtonDown);
		OnButtonReleasedConn = input.onButtonUp.Connect(&ScriptInput::onButtonUp);
		OnCharInputConn = input.onCharInput.Connect(&ScriptInput::onCharInput);
		OnPointerPressedConn = input.onPointerPressed.Connect(&ScriptInput::onPointerPressed);
		OnPointerReleasedConn = input.onPointerReleased.Connect(&ScriptInput::onPointerReleased);
		OnPointerMovedConn = input.onPointerMoved.Connect(&ScriptInput::onPointerMoved);
		OnPointerDoubleClickConn = input.onPointerDoubleClick.Connect(&ScriptInput::onPointerDoubleClick);
	}

	void ScriptInput::ShutDown()
	{
		OnButtonPressedConn.Disconnect();
		OnButtonReleasedConn.Disconnect();
		OnCharInputConn.Disconnect();
		OnPointerPressedConn.Disconnect();
		OnPointerReleasedConn.Disconnect();
		OnPointerMovedConn.Disconnect();
		OnPointerDoubleClickConn.Disconnect();
	}

	void ScriptInput::OnButtonDown(const ButtonEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoUtil::invokeThunk(OnButtonPressedThunk, ev.buttonCode, ev.deviceIdx, ev.IsUsed());
	}

	void ScriptInput::OnButtonUp(const ButtonEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoUtil::invokeThunk(OnButtonReleasedThunk, ev.buttonCode, ev.deviceIdx, ev.IsUsed());
	}

	void ScriptInput::OnCharInput(const TextInputEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoUtil::invokeThunk(OnCharInputThunk, ev.textChar, ev.IsUsed());
	}

	void ScriptInput::OnPointerMoved(const PointerEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerMovedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.IsUsed());
	}

	void ScriptInput::OnPointerPressed(const PointerEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerPressedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.IsUsed());
	}

	void ScriptInput::OnPointerReleased(const PointerEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerReleasedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.IsUsed());
	}

	void ScriptInput::OnPointerDoubleClick(const PointerEvent& ev)
	{
		if (PlayInEditor::instance().GetState() != PlayInEditorState::Playing)
			return;

		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerDoubleClickThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.IsUsed());
	}

	bool ScriptInput::internal_isButtonHeld(ButtonCode code, UINT32 deviceIdx)
	{
		return Input::Instance().IsButtonHeld(code, deviceIdx);
	}

	bool ScriptInput::internal_isButtonDown(ButtonCode code, UINT32 deviceIdx)
	{
		return Input::Instance().IsButtonDown(code, deviceIdx);
	}

	bool ScriptInput::internal_isButtonUp(ButtonCode code, UINT32 deviceIdx)
	{
		return Input::Instance().IsButtonUp(code, deviceIdx);
	}

	bool ScriptInput::internal_isPointerButtonHeld(PointerEventButton code)
	{
		return Input::Instance().IsPointerButtonHeld(code);
	}

	bool ScriptInput::internal_isPointerButtonDown(PointerEventButton code)
	{
		return Input::Instance().IsPointerButtonDown(code);
	}

	bool ScriptInput::internal_isPointerButtonUp(PointerEventButton code)
	{
		return Input::Instance().IsPointerButtonUp(code);
	}

	bool ScriptInput::internal_isPointerDoubleClicked()
	{
		return Input::Instance().IsPointerDoubleClicked();
	}

	float ScriptInput::internal_getAxisValue(UINT32 axisType, UINT32 deviceIdx)
	{
		return Input::Instance().GetAxisValue(axisType, deviceIdx);
	}

	void ScriptInput::internal_getPointerPosition(Vector2I* position)
	{
		*position = Input::instance().GetPointerPosition();
	}

	void ScriptInput::internal_getPointerDelta(Vector2I* position)
	{
		*position = Input::instance().GetPointerDelta();
	}
}
