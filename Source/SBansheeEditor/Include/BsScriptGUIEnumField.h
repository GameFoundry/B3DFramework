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

	/**
	 * Interop class between C++ & CLR for GUIListBoxField, with some specific functionality meant for displaying managed
	 * enumeration values.
	 */
	class BS_SCR_BED_EXPORT ScriptGUIEnumField : public TScriptGUIElement<ScriptGUIEnumField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIEnumField")

	private:
		/**
		 * Triggered when the value in the native list box selection changes.
		 *
		 * @param[in]	instance	Managed GUIEnumField instance.
		 * @param[in]	newIndex	Index of the selected element.
		 * @param[in]	enabled		Determines whether the element at the selection index was enabled or disabled.
		 */
		static void onSelectionChanged(MonoObject* instance, UINT64 newIndex, bool enabled);

		ScriptGUIEnumField(MonoObject* instance, GUIListBoxField* listBoxField, const Vector<UINT64>& values);

		Vector<UINT64> mValues;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoArray* names, MonoArray* values, bool multiselect,
			MonoObject* title, UINT32 titleWidth, MonoString* style, MonoArray* guiOptions, bool withTitle);

		static UINT64 internal_getValue(ScriptGUIEnumField* nativeInstance);
		static void internal_setValue(ScriptGUIEnumField* nativeInstance, UINT64 value);
		static void internal_setTint(ScriptGUIEnumField* nativeInstance, Color* color);
		static void internal_selectElement(ScriptGUIEnumField* nativeInstance, int idx);
		static void internal_deselectElement(ScriptGUIEnumField* nativeInstance, int idx);
		static MonoArray* internal_getElementStates(ScriptGUIEnumField* nativeInstance);
		static void internal_setElementStates(ScriptGUIEnumField* nativeInstance, MonoArray* states);

		typedef void(__stdcall *OnSelectionChangedThunkDef) (MonoObject*, UINT64, MonoException**);

		static OnSelectionChangedThunkDef onSelectionChangedThunk;
	};

	/** @} */
}