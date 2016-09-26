//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIContextMenu. */
	class BS_SCR_BE_EXPORT ScriptContextMenu : public ScriptObject < ScriptContextMenu >
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ContextMenu")

		/**	Returns the internal native context menu object. */
		SPtr<GUIContextMenu> getInternal() const { return mContextMenu; }

	private:
		ScriptContextMenu(MonoObject* instance);

		/**
		 * Triggered when an item in the context menu is clicked.
		 *
		 * @param[in]	idx		Sequential index of the item that was clicked.
		 */
		void onContextMenuItemTriggered(UINT32 idx);

		SPtr<GUIContextMenu> mContextMenu;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *OnEntryTriggeredThunkDef) (MonoObject*, UINT32 callbackIdx, MonoException**);
		static OnEntryTriggeredThunkDef onEntryTriggered;

		static void internal_CreateInstance(MonoObject* instance);
		static void internal_Open(ScriptContextMenu* instance, Vector2I* position, ScriptGUILayout* layoutPtr);
		static void internal_AddItem(ScriptContextMenu* instance, MonoString* path, UINT32 callbackIdx, ShortcutKey* shortcut);
		static void internal_AddSeparator(ScriptContextMenu* instance, MonoString* path);
		static void internal_SetLocalizedName(ScriptContextMenu* instance, MonoString* label, ScriptHString* name);
	};

	/** @} */
}