//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptEditorInput.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsDebug.h"
#include "BsInput.h"
#include "BsScriptVector2I.h"

namespace BansheeEngine
{
	ScriptEditorInput::OnButtonEventThunkDef ScriptEditorInput::OnButtonPressedThunk;
	ScriptEditorInput::OnButtonEventThunkDef ScriptEditorInput::OnButtonReleasedThunk;
	ScriptEditorInput::OnCharInputEventThunkDef ScriptEditorInput::OnCharInputThunk;
	ScriptEditorInput::OnPointerEventThunkDef ScriptEditorInput::OnPointerPressedThunk;
	ScriptEditorInput::OnPointerEventThunkDef ScriptEditorInput::OnPointerReleasedThunk;
	ScriptEditorInput::OnPointerEventThunkDef ScriptEditorInput::OnPointerMovedThunk;
	ScriptEditorInput::OnPointerEventThunkDef ScriptEditorInput::OnPointerDoubleClickThunk;

	HEvent ScriptEditorInput::OnButtonPressedConn;
	HEvent ScriptEditorInput::OnButtonReleasedConn;
	HEvent ScriptEditorInput::OnCharInputConn;
	HEvent ScriptEditorInput::OnPointerPressedConn;
	HEvent ScriptEditorInput::OnPointerReleasedConn;
	HEvent ScriptEditorInput::OnPointerMovedConn;
	HEvent ScriptEditorInput::OnPointerDoubleClickConn;

	ScriptEditorInput::ScriptEditorInput(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorInput::initRuntimeData()
	{
		OnButtonPressedThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonDown", "ButtonCode,int,bool")->getThunk();
		OnButtonReleasedThunk = (OnButtonEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerButtonUp", "ButtonCode,int,bool")->getThunk();
		OnCharInputThunk = (OnCharInputEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerCharInput", "int,bool")->getThunk();
		OnPointerPressedThunk = (OnPointerEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerPointerPressed", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerReleasedThunk = (OnPointerEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerPointerReleased", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerMovedThunk = (OnPointerEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerPointerMove", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
		OnPointerDoubleClickThunk = (OnPointerEventThunkDef)metaData.scriptClass->getMethodExact("Internal_TriggerPointerDoubleClick", "Vector2I,Vector2I,PointerButton,bool,bool,bool,single,bool")->getThunk();
	}

	void ScriptEditorInput::startUp()
	{
		Input& input = Input::instance();

		OnButtonPressedConn = input.onButtonDown.connect(&ScriptEditorInput::onButtonDown);
		OnButtonReleasedConn = input.onButtonUp.connect(&ScriptEditorInput::onButtonUp);
		OnCharInputConn = input.onCharInput.connect(&ScriptEditorInput::onCharInput);
		OnPointerPressedConn = input.onPointerPressed.connect(&ScriptEditorInput::onPointerPressed);
		OnPointerReleasedConn = input.onPointerReleased.connect(&ScriptEditorInput::onPointerReleased);
		OnPointerMovedConn = input.onPointerMoved.connect(&ScriptEditorInput::onPointerMoved);
		OnPointerDoubleClickConn = input.onPointerDoubleClick.connect(&ScriptEditorInput::onPointerDoubleClick);
	}

	void ScriptEditorInput::shutDown()
	{
		OnButtonPressedConn.disconnect();
		OnButtonReleasedConn.disconnect();
		OnCharInputConn.disconnect();
		OnPointerPressedConn.disconnect();
		OnPointerReleasedConn.disconnect();
		OnPointerMovedConn.disconnect();
		OnPointerDoubleClickConn.disconnect();
	}

	void ScriptEditorInput::onButtonDown(const ButtonEvent& ev)
	{
		MonoUtil::invokeThunk(OnButtonPressedThunk, ev.buttonCode, ev.deviceIdx, ev.isUsed());
	}

	void ScriptEditorInput::onButtonUp(const ButtonEvent& ev)
	{
		MonoUtil::invokeThunk(OnButtonReleasedThunk, ev.buttonCode, ev.deviceIdx, ev.isUsed());
	}

	void ScriptEditorInput::onCharInput(const TextInputEvent& ev)
	{
		MonoUtil::invokeThunk(OnCharInputThunk, ev.textChar, ev.isUsed());
	}

	void ScriptEditorInput::onPointerMoved(const PointerEvent& ev)
	{
		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerMovedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.isUsed());
	}

	void ScriptEditorInput::onPointerPressed(const PointerEvent& ev)
	{
		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerPressedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.isUsed());
	}

	void ScriptEditorInput::onPointerReleased(const PointerEvent& ev)
	{
		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerReleasedThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.isUsed());
	}

	void ScriptEditorInput::onPointerDoubleClick(const PointerEvent& ev)
	{
		MonoObject* screenPos = ScriptVector2I::box(ev.screenPos);
		MonoObject* delta = ScriptVector2I::box(ev.delta);

		MonoUtil::invokeThunk(OnPointerDoubleClickThunk, screenPos, delta,
			ev.button, ev.shift, ev.control, ev.alt, ev.mouseWheelScrollAmount, ev.isUsed());
	}
}