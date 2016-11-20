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

	/**	Interop class between C++ & CLR for GUISceneTreeView. */
	class BS_SCR_BED_EXPORT ScriptGUISceneTreeView : public TScriptGUIElement<ScriptGUISceneTreeView>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUISceneTreeView")

	private:
		ScriptGUISceneTreeView(MonoObject* instance, GUISceneTreeView* treeView);
		~ScriptGUISceneTreeView();

		/** Triggered when the native scene tree view modifies the scene. */
		void sceneModified();

		/** Triggered when a resource is dragged and dropped over the native scene tree view. */
		void resourceDropped(const HSceneObject& parent, const Vector<Path>& resourcePaths);

		HEvent mOnModifiedConn;
		HEvent mOnResourceDroppedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void internal_update(ScriptGUISceneTreeView* thisPtr);
		static void internal_cutSelection(ScriptGUISceneTreeView* thisPtr);
		static void internal_copySelection(ScriptGUISceneTreeView* thisPtr);
		static void internal_pasteToSelection(ScriptGUISceneTreeView* thisPtr);
		static void internal_duplicateSelection(ScriptGUISceneTreeView* thisPtr);
		static void internal_deleteSelection(ScriptGUISceneTreeView* thisPtr);
		static void internal_renameSelection(ScriptGUISceneTreeView* thisPtr);

		typedef void(__stdcall *OnModifiedThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *OnResourceDroppedThunkDef) (MonoObject*, MonoObject*, MonoArray*, MonoException**);

		static OnModifiedThunkDef onModifiedThunk;
		static OnResourceDroppedThunkDef onResourceDroppedThunk;
	};

	/** @} */
}