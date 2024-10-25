//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptNonReflectableWrapper.h"

namespace bs
{
	class ScriptGUILayoutWrapperBase;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIContextMenu. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptContextMenu : public TScriptNonReflectableWrapper<GUIContextMenu, ScriptContextMenu>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ContextMenu")

		ScriptContextMenu(const SPtr<GUIContextMenu>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);
	private:

		/**
		 * Triggered when an item in the context menu is clicked.
		 *
		 * @param[in]	idx		Sequential index of the item that was clicked.
		 */
		void OnContextMenuItemTriggered(u32 idx);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(B3D_THUNKCALL* OnEntryTriggeredThunkDef)(MonoObject*, u32 callbackIdx, MonoException**);
		static OnEntryTriggeredThunkDef onEntryTriggered;

		static void InternalCreateInstance(MonoObject* scriptObject);
		static void InternalOpen(ScriptContextMenu* self, Vector2I* position, ScriptGUILayoutWrapperBase* layoutPtr);
		static void InternalAddItem(ScriptContextMenu* self, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut);
		static void InternalAddSeparator(ScriptContextMenu* self, MonoString* path);
		static void InternalSetLocalizedName(ScriptContextMenu* self, MonoString* label, ScriptLocString* name);
	};

	/** @} */
} // namespace bs
