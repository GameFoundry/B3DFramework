//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ScriptGUILayoutWrapperBase;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIContextMenu. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptContextMenu : public ScriptObject<ScriptContextMenu>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ContextMenu")

		/**	Returns the internal native context menu object. */
		SPtr<GUIContextMenu> GetInternal() const { return mContextMenu; }

	private:
		ScriptContextMenu(MonoObject* instance);

		/**
		 * Triggered when an item in the context menu is clicked.
		 *
		 * @param[in]	idx		Sequential index of the item that was clicked.
		 */
		void OnContextMenuItemTriggered(u32 idx);

		SPtr<GUIContextMenu> mContextMenu;
		u32 mGCHandle = 0;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(B3D_THUNKCALL* OnEntryTriggeredThunkDef)(MonoObject*, u32 callbackIdx, MonoException**);
		static OnEntryTriggeredThunkDef onEntryTriggered;

		static void InternalCreateInstance(MonoObject* instance);
		static void InternalOpen(ScriptContextMenu* instance, Vector2I* position, ScriptGUILayoutWrapperBase* layoutPtr);
		static void InternalAddItem(ScriptContextMenu* instance, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut);
		static void InternalAddSeparator(ScriptContextMenu* instance, MonoString* path);
		static void InternalSetLocalizedName(ScriptContextMenu* instance, MonoString* label, ScriptLocString* name);
	};

	/** @} */
} // namespace bs
