//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Input/BsInputConfiguration.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for VirtualInput. */
	class BS_SCR_BE_EXPORT ScriptVirtualInput : public ScriptObject<ScriptVirtualInput>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "VirtualInput")

		/**	Must be called on library load. Hooks up necessary callbacks. */
		static void StartUp();

		/**	Must be called before library shutdown. Releases previously hooked callbacks. */
		static void ShutDown();

	private:
		/**
		 * Triggered whenever a virtual button is pressed.
		 *
		 * @param[in]	btn			Virtual button that was pressed.
		 * @param[in]	deviceIdx	Index of the device the button was pressed on.
		 */
		static void OnButtonDown(const VirtualButton& btn, u32 deviceIdx);

		/**
		 * Triggered whenever a virtual button is released.
		 *
		 * @param[in]	btn			Virtual button that was released.
		 * @param[in]	deviceIdx	Index of the device the button was released on.
		 */
		static void OnButtonUp(const VirtualButton& btn, u32 deviceIdx);

		/**
		 * Triggered every frame while a virtual button is held down.
		 *
		 * @param[in]	btn			Virtual button that is being held.
		 * @param[in]	deviceIdx	Index of the device the button is held.
		 */
		static void OnButtonHeld(const VirtualButton& btn, u32 deviceIdx);

		static HEvent OnButtonPressedConn;
		static HEvent OnButtonReleasedConn;
		static HEvent OnButtonHeldConn;

		ScriptVirtualInput(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/

		static MonoObject* InternalGetKeyConfig();
		static void InternalSetKeyConfig(MonoObject* keyConfig);
		static bool InternalIsButtonHeld(VirtualButton* btn, u32 deviceIdx);
		static bool InternalIsButtonDown(VirtualButton* btn, u32 deviceIdx);
		static bool InternalIsButtonUp(VirtualButton* btn, u32 deviceIdx);
		static float InternalGetAxisValue(VirtualAxis* axis, u32 deviceIdx);

		typedef void(BS_THUNKCALL* OnButtonEventThunkDef)(MonoObject*, u32, MonoException**);

		static OnButtonEventThunkDef OnButtonUpThunk;
		static OnButtonEventThunkDef OnButtonDownThunk;
		static OnButtonEventThunkDef OnButtonHeldThunk;
	};

	/** @} */
} // namespace bs
