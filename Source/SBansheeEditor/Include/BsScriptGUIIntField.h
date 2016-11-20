//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIIntField. */
	class BS_SCR_BED_EXPORT ScriptGUIIntField : public TScriptGUIElement<ScriptGUIIntField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIIntField")

	private:
		/**
		 * Triggered when the value in the native int field changes.
		 *
		 * @param[in]	instance	Managed GUIIntField instance.
		 * @param[in]	newValue	New field value.
		 */
		static void onChanged(MonoObject* instance, INT32 newValue);

		/**
		 * Triggered when the user confirms input in the native int field.
		 *
		 * @param[in]	instance	Managed GUIIntField instance.
		 */
		static void onConfirmed(MonoObject* instance);

		ScriptGUIIntField(MonoObject* instance, GUIIntField* intField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static INT32 internal_getValue(ScriptGUIIntField* nativeInstance);
		static INT32 internal_setValue(ScriptGUIIntField* nativeInstance, INT32 value);
		static bool internal_hasInputFocus(ScriptGUIIntField* nativeInstance);
		static void internal_setRange(ScriptGUIIntField* nativeInstance, INT32 min, INT32 max);
		static void internal_setTint(ScriptGUIIntField* nativeInstance, Color* color);
		static void internal_setStep(ScriptGUIIntField* nativeInstance, INT32 step);
		static INT32 internal_getStep(ScriptGUIIntField* nativeInstance);

		typedef void (__stdcall *OnChangedThunkDef) (MonoObject*, INT32, MonoException**);
		typedef void(__stdcall *OnConfirmedThunkDef) (MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
}