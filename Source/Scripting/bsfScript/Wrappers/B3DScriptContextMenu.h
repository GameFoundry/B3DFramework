//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptNonReflectableWrapper.h"

namespace b3d
{
	class ScriptGUILayoutWrapperBase;
	struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop;


	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIContextMenu. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptContextMenu : public TScriptNonReflectableWrapper<GUIContextMenu, ScriptContextMenu>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ContextMenu")

		ScriptContextMenu(const TShared<GUIContextMenu>& nativeObject);

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
		static void InternalOpen(ScriptContextMenu* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position, ScriptGUILayoutWrapperBase* layoutPtr);
		static void InternalAddItem(ScriptContextMenu* self, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut);
		static void InternalAddSeparator(ScriptContextMenu* self, MonoString* path);
		static void InternalSetLocalizedName(ScriptContextMenu* self, MonoString* label, ScriptLocString* name);
	};

	/** @} */
} // namespace b3d
