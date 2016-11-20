//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsInputFwd.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Input. */
	class BS_SCR_BE_EXPORT ScriptInput : public ScriptObject<ScriptInput>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Input")

		/**	Registers internal callbacks. Must be called on scripting system load. */
		static void startUp();

		/**	Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void shutDown();
	private:
		ScriptInput(MonoObject* instance);

		/**	Triggered when the specified button is pressed. */
		static void onButtonDown(const ButtonEvent& ev);

		/**	Triggered when the specified button is released. */
		static void onButtonUp(const ButtonEvent& ev);

		/**	Triggered when the specified character is entered. */
		static void onCharInput(const TextInputEvent& ev);

		/**	Triggered when the pointer is moved. */
		static void onPointerMoved(const PointerEvent& ev);

		/**	Triggered when a pointer button is pressed. */
		static void onPointerPressed(const PointerEvent& ev);

		/**	Triggered when a pointer button is released. */
		static void onPointerReleased(const PointerEvent& ev);

		/**	Triggered when a pointer button is double-clicked. */
		static void onPointerDoubleClick(const PointerEvent& ev);

		static HEvent OnButtonPressedConn;
		static HEvent OnButtonReleasedConn;
		static HEvent OnCharInputConn;
		static HEvent OnPointerPressedConn;
		static HEvent OnPointerReleasedConn;
		static HEvent OnPointerMovedConn;
		static HEvent OnPointerDoubleClickConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_isButtonHeld(ButtonCode code, UINT32 deviceIdx);
		static bool internal_isButtonDown(ButtonCode code, UINT32 deviceIdx);
		static bool internal_isButtonUp(ButtonCode code, UINT32 deviceIdx);
		static bool internal_isPointerButtonHeld(PointerEventButton code);
		static bool internal_isPointerButtonDown(PointerEventButton code);
		static bool internal_isPointerButtonUp(PointerEventButton code);
		static bool internal_isPointerDoubleClicked();
		static float internal_getAxisValue(UINT32 axisType, UINT32 deviceIdx);
		static void internal_getPointerPosition(Vector2I* position);
		static void internal_getPointerDelta(Vector2I* position);

		typedef void(__stdcall *OnButtonEventThunkDef) (ButtonCode, UINT32, bool, MonoException**);
		typedef void(__stdcall *OnCharInputEventThunkDef) (UINT32, bool, MonoException**);
		typedef void(__stdcall *OnPointerEventThunkDef) (MonoObject*, MonoObject*, PointerEventButton,
			bool, bool, bool, float, bool, MonoException**);

		static OnButtonEventThunkDef OnButtonPressedThunk;
		static OnButtonEventThunkDef OnButtonReleasedThunk;
		static OnCharInputEventThunkDef OnCharInputThunk;
		static OnPointerEventThunkDef OnPointerPressedThunk;
		static OnPointerEventThunkDef OnPointerReleasedThunk;
		static OnPointerEventThunkDef OnPointerMovedThunk;
		static OnPointerEventThunkDef OnPointerDoubleClickThunk;
	};

	/** @} */
}