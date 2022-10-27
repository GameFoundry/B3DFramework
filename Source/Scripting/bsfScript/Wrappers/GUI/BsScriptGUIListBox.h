//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUITexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIListBox. */
	class BS_SCR_BE_EXPORT ScriptGUIListBox : public TScriptGUIElement<ScriptGUIListBox>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIListBox")

	private:
		ScriptGUIListBox(MonoObject* instance, GUIListBox* listBox);

		/**	Triggered when the selected index in the native list box changes. */
		void OnSelectionChanged(u32 index, bool enabled);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoArray* elements, bool multiselect, MonoString* style, MonoArray* guiOptions);
		static void InternalSetElements(ScriptGUIListBox* nativeInstance, MonoArray* elements);
		static void InternalSetTint(ScriptGUIListBox* nativeInstance, Color* color);
		static void InternalSelectElement(ScriptGUIListBox* nativeInstance, int idx);
		static void InternalDeselectElement(ScriptGUIListBox* nativeInstance, int idx);
		static MonoArray* InternalGetElementStates(ScriptGUIListBox* nativeInstance);
		static void InternalSetElementStates(ScriptGUIListBox* nativeInstance, MonoArray* states);

		typedef void(BS_THUNKCALL* OnSelectionChangedThunkDef)(MonoObject*, u32, MonoException**);
		static OnSelectionChangedThunkDef onSelectionChangedThunk;
	};

	/** @} */
} // namespace bs
