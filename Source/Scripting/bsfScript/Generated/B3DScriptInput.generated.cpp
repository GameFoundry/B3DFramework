//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptInput.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Input/B3DInput.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptPointerEvent.generated.h"
#include "B3DScriptTextInputEvent.generated.h"
#include "B3DScriptButtonEvent.generated.h"

namespace b3d
{
	ScriptInput::OnButtonDownThunkDefinition ScriptInput::OnButtonDownThunk; 
	ScriptInput::OnButtonUpThunkDefinition ScriptInput::OnButtonUpThunk; 
	ScriptInput::OnCharInputThunkDefinition ScriptInput::OnCharInputThunk; 
	ScriptInput::OnPointerMovedThunkDefinition ScriptInput::OnPointerMovedThunk; 
	ScriptInput::OnPointerPressedThunkDefinition ScriptInput::OnPointerPressedThunk; 
	ScriptInput::OnPointerReleasedThunkDefinition ScriptInput::OnPointerReleasedThunk; 
	ScriptInput::OnPointerDoubleClickThunkDefinition ScriptInput::OnPointerDoubleClickThunk; 

	HEvent ScriptInput::OnButtonDownConnection;
	HEvent ScriptInput::OnButtonUpConnection;
	HEvent ScriptInput::OnCharInputConnection;
	HEvent ScriptInput::OnPointerMovedConnection;
	HEvent ScriptInput::OnPointerPressedConnection;
	HEvent ScriptInput::OnPointerReleasedConnection;
	HEvent ScriptInput::OnPointerDoubleClickConnection;

	ScriptInput::ScriptInput()
		:TScriptTypeDefinition()
	{
	}

	void ScriptInput::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAxisValue", (void*)&ScriptInput::InternalGetAxisValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonHeld", (void*)&ScriptInput::InternalIsButtonHeld);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonUp", (void*)&ScriptInput::InternalIsButtonUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsButtonDown", (void*)&ScriptInput::InternalIsButtonDown);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointerPosition", (void*)&ScriptInput::InternalGetPointerPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointerDelta", (void*)&ScriptInput::InternalGetPointerDelta);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonHeld", (void*)&ScriptInput::InternalIsPointerButtonHeld);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonUp", (void*)&ScriptInput::InternalIsPointerButtonUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPointerButtonDown", (void*)&ScriptInput::InternalIsPointerButtonDown);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPointerDoubleClicked", (void*)&ScriptInput::InternalIsPointerDoubleClicked);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMouseSmoothing", (void*)&ScriptInput::InternalSetMouseSmoothing);

		OnButtonDownThunk = (OnButtonDownThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnButtonDown", "ButtonEvent&")->GetThunk();
		OnButtonUpThunk = (OnButtonUpThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnButtonUp", "ButtonEvent&")->GetThunk();
		OnCharInputThunk = (OnCharInputThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnCharInput", "TextInputEvent&")->GetThunk();
		OnPointerMovedThunk = (OnPointerMovedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnPointerMoved", "PointerEvent&")->GetThunk();
		OnPointerPressedThunk = (OnPointerPressedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnPointerPressed", "PointerEvent&")->GetThunk();
		OnPointerReleasedThunk = (OnPointerReleasedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnPointerReleased", "PointerEvent&")->GetThunk();
		OnPointerDoubleClickThunk = (OnPointerDoubleClickThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnPointerDoubleClick", "PointerEvent&")->GetThunk();
	}

	void ScriptInput::StartUp()
	{
		OnButtonDownConnection = Input::Instance().OnButtonDown.Connect(&ScriptInput::OnButtonDown);
		OnButtonUpConnection = Input::Instance().OnButtonUp.Connect(&ScriptInput::OnButtonUp);
		OnCharInputConnection = Input::Instance().OnCharInput.Connect(&ScriptInput::OnCharInput);
		OnPointerMovedConnection = Input::Instance().OnPointerMoved.Connect(&ScriptInput::OnPointerMoved);
		OnPointerPressedConnection = Input::Instance().OnPointerPressed.Connect(&ScriptInput::OnPointerPressed);
		OnPointerReleasedConnection = Input::Instance().OnPointerReleased.Connect(&ScriptInput::OnPointerReleased);
		OnPointerDoubleClickConnection = Input::Instance().OnPointerDoubleClick.Connect(&ScriptInput::OnPointerDoubleClick);
	}
	void ScriptInput::ShutDown()
	{
		OnButtonDownConnection.Disconnect();
		OnButtonUpConnection.Disconnect();
		OnCharInputConnection.Disconnect();
		OnPointerMovedConnection.Disconnect();
		OnPointerPressedConnection.Disconnect();
		OnPointerReleasedConnection.Disconnect();
		OnPointerDoubleClickConnection.Disconnect();
	}

	void ScriptInput::OnButtonDown(const ButtonEvent& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptButtonEvent::Box(p0);
		MonoUtil::InvokeThunk(OnButtonDownThunk, tmpp0);
	}

	void ScriptInput::OnButtonUp(const ButtonEvent& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptButtonEvent::Box(p0);
		MonoUtil::InvokeThunk(OnButtonUpThunk, tmpp0);
	}

	void ScriptInput::OnCharInput(const TextInputEvent& p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptTextInputEvent::Box(p0);
		MonoUtil::InvokeThunk(OnCharInputThunk, tmpp0);
	}

	void ScriptInput::OnPointerMoved(const PointerEvent& p0)
	{
		MonoObject* tmpp0;
		__PointerEventInterop interopp0;
		interopp0 = ScriptPointerEvent::ToInterop(p0);
		tmpp0 = ScriptPointerEvent::Box(interopp0);
		MonoUtil::InvokeThunk(OnPointerMovedThunk, tmpp0);
	}

	void ScriptInput::OnPointerPressed(const PointerEvent& p0)
	{
		MonoObject* tmpp0;
		__PointerEventInterop interopp0;
		interopp0 = ScriptPointerEvent::ToInterop(p0);
		tmpp0 = ScriptPointerEvent::Box(interopp0);
		MonoUtil::InvokeThunk(OnPointerPressedThunk, tmpp0);
	}

	void ScriptInput::OnPointerReleased(const PointerEvent& p0)
	{
		MonoObject* tmpp0;
		__PointerEventInterop interopp0;
		interopp0 = ScriptPointerEvent::ToInterop(p0);
		tmpp0 = ScriptPointerEvent::Box(interopp0);
		MonoUtil::InvokeThunk(OnPointerReleasedThunk, tmpp0);
	}

	void ScriptInput::OnPointerDoubleClick(const PointerEvent& p0)
	{
		MonoObject* tmpp0;
		__PointerEventInterop interopp0;
		interopp0 = ScriptPointerEvent::ToInterop(p0);
		tmpp0 = ScriptPointerEvent::Box(interopp0);
		MonoUtil::InvokeThunk(OnPointerDoubleClickThunk, tmpp0);
	}

	float ScriptInput::InternalGetAxisValue(uint32_t type, uint32_t deviceIndex)
	{
		float tmp__output;
		tmp__output = Input::Instance().GetAxisValue(type, deviceIndex);

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsButtonHeld(ButtonCode keyCode, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsButtonHeld(keyCode, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsButtonUp(ButtonCode keyCode, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsButtonUp(keyCode, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsButtonDown(ButtonCode keyCode, uint32_t deviceIndex)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsButtonDown(keyCode, deviceIndex);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptInput::InternalGetPointerPosition(TVector2<int32_t>* __output)
	{
		TVector2<int32_t> tmp__output;
		tmp__output = Input::Instance().GetPointerPosition();

		*__output = tmp__output;
	}

	void ScriptInput::InternalGetPointerDelta(TVector2<int32_t>* __output)
	{
		TVector2<int32_t> tmp__output;
		tmp__output = Input::Instance().GetPointerDelta();

		*__output = tmp__output;
	}

	bool ScriptInput::InternalIsPointerButtonHeld(PointerEventButton pointerButton)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsPointerButtonHeld(pointerButton);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsPointerButtonUp(PointerEventButton pointerButton)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsPointerButtonUp(pointerButton);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsPointerButtonDown(PointerEventButton pointerButton)
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsPointerButtonDown(pointerButton);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptInput::InternalIsPointerDoubleClicked()
	{
		bool tmp__output;
		tmp__output = Input::Instance().IsPointerDoubleClicked();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptInput::InternalSetMouseSmoothing(bool enabled)
	{
		Input::Instance().SetMouseSmoothing(enabled);
	}
}
