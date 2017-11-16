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

	/**	Interop class between C++ & CLR for GUIListBoxField. */
	class BS_SCR_BED_EXPORT ScriptGUIListBoxField : public TScriptGUIElement<ScriptGUIListBoxField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIListBoxField")

	private:
		/**
		 * Triggered when the value in the native list box selection changes.
		 *
		 * @param[in]	newIndex	New selection index.
		 */
		void onSelectionChanged(UINT32 newIndex);

		ScriptGUIListBoxField(MonoObject* instance, GUIListBoxField* listBoxField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoArray* elements, bool multiselect, 
			MonoObject* title, UINT32 titleWidth, MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_setElements(ScriptGUIListBoxField* nativeInstance, MonoArray* elements);
		static UINT32 internal_getValue(ScriptGUIListBoxField* nativeInstance);
		static void internal_setValue(ScriptGUIListBoxField* nativeInstance, UINT32 value);
		static void internal_selectElement(ScriptGUIListBoxField* nativeInstance, int idx);
		static void internal_deselectElement(ScriptGUIListBoxField* nativeInstance, int idx);
		static MonoArray* internal_getElementStates(ScriptGUIListBoxField* nativeInstance);
		static void internal_setElementStates(ScriptGUIListBoxField* nativeInstance, MonoArray* states);
		static void internal_setTint(ScriptGUIListBoxField* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL *OnSelectionChangedThunkDef) (MonoObject*, UINT32, MonoException**);

		static OnSelectionChangedThunkDef onSelectionChangedThunk;
	};

	/** @} */
}