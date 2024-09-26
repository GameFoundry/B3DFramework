//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsInput.h"
#include "Generated//BsScriptTVector2I.generated.h"
#include "BsPlayInEditor.h"

using namespace bs;
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
	: ScriptObject(instance)
{}

void ScriptInput::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptInput::InternalIsButtonHeld);
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptInput::InternalIsButtonDown);
	metaData.ScriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptInput::InternalIsButtonUp);
	metaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonHeld", (void*)&ScriptInput::InternalIsPointerButtonHeld);
	metaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonDown", (void*)&ScriptInput::InternalIsPointerButtonDown);
	metaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonUp", (void*)&ScriptInput::InternalIsPointerButtonUp);
	metaData.ScriptClass->AddInternalCall("Internal_IsPointerDoubleClicked", (void*)&ScriptInput::InternalIsPointerDoubleClicked);
	metaData.ScriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptInput::InternalGetAxisValue);
	metaData.ScriptClass->AddInternalCall("Internal_GetPointerPosition", (void*)&ScriptInput::InternalGetPointerPosition);
	metaData.ScriptClass->AddInternalCall("Internal_GetPointerDelta", (void*)&ScriptInput::InternalGetPointerDelta);

	OnButtonPressedThunk = (OnButtonEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerButtonDown", "ButtonCode,int,bool")->GetThunk();
	OnButtonReleasedThunk = (OnButtonEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerButtonUp", "ButtonCode,int,bool")->GetThunk();
	OnCharInputThunk = (OnCharInputEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerCharInput", "int,bool")->GetThunk();
	OnPointerPressedThunk = (OnPointerEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerPointerPressed", "Vector2I,Vector2I,PointerEventButton,bool,bool,bool,single,bool")->GetThunk();
	OnPointerReleasedThunk = (OnPointerEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerPointerReleased", "Vector2I,Vector2I,PointerEventButton,bool,bool,bool,single,bool")->GetThunk();
	OnPointerMovedThunk = (OnPointerEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerPointerMove", "Vector2I,Vector2I,PointerEventButton,bool,bool,bool,single,bool")->GetThunk();
	OnPointerDoubleClickThunk = (OnPointerEventThunkDef)metaData.ScriptClass->GetMethodExact("Internal_TriggerPointerDoubleClick", "Vector2I,Vector2I,PointerEventButton,bool,bool,bool,single,bool")->GetThunk();
}

void ScriptInput::StartUp()
{
	Input& input = Input::Instance();

	OnButtonPressedConn = input.OnButtonDown.Connect(&ScriptInput::OnButtonDown);
	OnButtonReleasedConn = input.OnButtonUp.Connect(&ScriptInput::OnButtonUp);
	OnCharInputConn = input.OnCharInput.Connect(&ScriptInput::OnCharInput);
	OnPointerPressedConn = input.OnPointerPressed.Connect(&ScriptInput::OnPointerPressed);
	OnPointerReleasedConn = input.OnPointerReleased.Connect(&ScriptInput::OnPointerReleased);
	OnPointerMovedConn = input.OnPointerMoved.Connect(&ScriptInput::OnPointerMoved);
	OnPointerDoubleClickConn = input.OnPointerDoubleClick.Connect(&ScriptInput::OnPointerDoubleClick);
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
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoUtil::InvokeThunk(OnButtonPressedThunk, ev.ButtonCode, ev.DeviceIndex, ev.IsUsed);
}

void ScriptInput::OnButtonUp(const ButtonEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoUtil::InvokeThunk(OnButtonReleasedThunk, ev.ButtonCode, ev.DeviceIndex, ev.IsUsed);
}

void ScriptInput::OnCharInput(const TextInputEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoUtil::InvokeThunk(OnCharInputThunk, ev.TextChar, ev.IsUsed);
}

void ScriptInput::OnPointerMoved(const PointerEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* screenPos = ScriptVector2I::Box(ev.ScreenPos);
	MonoObject* delta = ScriptVector2I::Box(ev.Delta);

	MonoUtil::InvokeThunk(OnPointerMovedThunk, screenPos, delta, ev.Button, ev.Shift, ev.Control, ev.Alt, ev.MouseWheelScrollAmount, ev.IsUsed);
}

void ScriptInput::OnPointerPressed(const PointerEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* screenPos = ScriptVector2I::Box(ev.ScreenPos);
	MonoObject* delta = ScriptVector2I::Box(ev.Delta);

	MonoUtil::InvokeThunk(OnPointerPressedThunk, screenPos, delta, ev.Button, ev.Shift, ev.Control, ev.Alt, ev.MouseWheelScrollAmount, ev.IsUsed);
}

void ScriptInput::OnPointerReleased(const PointerEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* screenPos = ScriptVector2I::Box(ev.ScreenPos);
	MonoObject* delta = ScriptVector2I::Box(ev.Delta);

	MonoUtil::InvokeThunk(OnPointerReleasedThunk, screenPos, delta, ev.Button, ev.Shift, ev.Control, ev.Alt, ev.MouseWheelScrollAmount, ev.IsUsed);
}

void ScriptInput::OnPointerDoubleClick(const PointerEvent& ev)
{
	if(PlayInEditor::Instance().GetState() != PlayInEditorState::Playing)
		return;

	MonoObject* screenPos = ScriptVector2I::Box(ev.ScreenPos);
	MonoObject* delta = ScriptVector2I::Box(ev.Delta);

	MonoUtil::InvokeThunk(OnPointerDoubleClickThunk, screenPos, delta, ev.Button, ev.Shift, ev.Control, ev.Alt, ev.MouseWheelScrollAmount, ev.IsUsed);
}

bool ScriptInput::InternalIsButtonHeld(ButtonCode code, u32 deviceIdx)
{
	return Input::Instance().IsButtonHeld(code, deviceIdx);
}

bool ScriptInput::InternalIsButtonDown(ButtonCode code, u32 deviceIdx)
{
	return Input::Instance().IsButtonDown(code, deviceIdx);
}

bool ScriptInput::InternalIsButtonUp(ButtonCode code, u32 deviceIdx)
{
	return Input::Instance().IsButtonUp(code, deviceIdx);
}

bool ScriptInput::InternalIsPointerButtonHeld(PointerEventButton code)
{
	return Input::Instance().IsPointerButtonHeld(code);
}

bool ScriptInput::InternalIsPointerButtonDown(PointerEventButton code)
{
	return Input::Instance().IsPointerButtonDown(code);
}

bool ScriptInput::InternalIsPointerButtonUp(PointerEventButton code)
{
	return Input::Instance().IsPointerButtonUp(code);
}

bool ScriptInput::InternalIsPointerDoubleClicked()
{
	return Input::Instance().IsPointerDoubleClicked();
}

float ScriptInput::InternalGetAxisValue(u32 axisType, u32 deviceIdx)
{
	return Input::Instance().GetAxisValue(axisType, deviceIdx);
}

void ScriptInput::InternalGetPointerPosition(Vector2I* position)
{
	*position = Input::Instance().GetPointerPosition();
}

void ScriptInput::InternalGetPointerDelta(Vector2I* position)
{
	*position = Input::Instance().GetPointerDelta();
}
