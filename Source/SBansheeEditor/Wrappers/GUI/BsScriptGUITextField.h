//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUITextField. */
	class BS_SCR_BED_EXPORT ScriptGUITextField : public TScriptGUIElement<ScriptGUITextField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUITextField")

	private:
		/**
		 * Triggered when the value in the native text field changes.
		 *
		 * @param[in]	newValue	New string value.
		 */
		void onChanged(const WString& newValue);

		/** Triggered when the user confirms input in the native text field. */
		void onConfirmed();

		ScriptGUITextField(MonoObject* instance, GUITextField* textField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, bool multiline, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUITextField* nativeInstance, MonoString** output);
		static void internal_setValue(ScriptGUITextField* nativeInstance, MonoString* value);
		static void internal_hasInputFocus(ScriptGUITextField* nativeInstance, bool* output);
		static void internal_setTint(ScriptGUITextField* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL *OnChangedThunkDef) (MonoObject*, MonoString*, MonoException**);
		typedef void(BS_THUNKCALL *OnConfirmedThunkDef) (MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
}