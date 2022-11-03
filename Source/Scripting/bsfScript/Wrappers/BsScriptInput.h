//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Input/BsInputFwd.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Input. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptInput : public ScriptObject<ScriptInput>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Input")

		/**	Registers internal callbacks. Must be called on scripting system load. */
		static void StartUp();

		/**	Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void ShutDown();

	private:
		ScriptInput(MonoObject* instance);

		/**	Triggered when the specified button is pressed. */
		static void OnButtonDown(const ButtonEvent& ev);

		/**	Triggered when the specified button is released. */
		static void OnButtonUp(const ButtonEvent& ev);

		/**	Triggered when the specified character is entered. */
		static void OnCharInput(const TextInputEvent& ev);

		/**	Triggered when the pointer is moved. */
		static void OnPointerMoved(const PointerEvent& ev);

		/**	Triggered when a pointer button is pressed. */
		static void OnPointerPressed(const PointerEvent& ev);

		/**	Triggered when a pointer button is released. */
		static void OnPointerReleased(const PointerEvent& ev);

		/**	Triggered when a pointer button is double-clicked. */
		static void OnPointerDoubleClick(const PointerEvent& ev);

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
		static bool InternalIsButtonHeld(ButtonCode code, u32 deviceIdx);
		static bool InternalIsButtonDown(ButtonCode code, u32 deviceIdx);
		static bool InternalIsButtonUp(ButtonCode code, u32 deviceIdx);
		static bool InternalIsPointerButtonHeld(PointerEventButton code);
		static bool InternalIsPointerButtonDown(PointerEventButton code);
		static bool InternalIsPointerButtonUp(PointerEventButton code);
		static bool InternalIsPointerDoubleClicked();
		static float InternalGetAxisValue(u32 axisType, u32 deviceIdx);
		static void InternalGetPointerPosition(Vector2I* position);
		static void InternalGetPointerDelta(Vector2I* position);

		typedef void(B3D_THUNKCALL* OnButtonEventThunkDef)(ButtonCode, u32, bool, MonoException**);
		typedef void(B3D_THUNKCALL* OnCharInputEventThunkDef)(u32, bool, MonoException**);
		typedef void(B3D_THUNKCALL* OnPointerEventThunkDef)(MonoObject*, MonoObject*, PointerEventButton, bool, bool, bool, float, bool, MonoException**);

		static OnButtonEventThunkDef OnButtonPressedThunk;
		static OnButtonEventThunkDef OnButtonReleasedThunk;
		static OnCharInputEventThunkDef OnCharInputThunk;
		static OnPointerEventThunkDef OnPointerPressedThunk;
		static OnPointerEventThunkDef OnPointerReleasedThunk;
		static OnPointerEventThunkDef OnPointerMovedThunk;
		static OnPointerEventThunkDef OnPointerDoubleClickThunk;
	};

	/** @} */
} // namespace bs
