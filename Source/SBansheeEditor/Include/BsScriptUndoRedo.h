//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class UndoRedo;
	class ScriptCmdManaged;

	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for UndoRedo. */
	class BS_SCR_BED_EXPORT ScriptUndoRedo : public ScriptObject <ScriptUndoRedo>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "UndoRedo");

		/** Creates a new managed UndoRedo stack. */
		static MonoObject* create();

		/**	Creates the globally accessible undo/redo stack. */
		static void startUp();

		/** Cleans up any data related to the global undo/redo stack. */
		static void shutDown();

	private:
		ScriptUndoRedo(MonoObject* instance, const SPtr<UndoRedo>& undoRedo);

		SPtr<UndoRedo> mUndoRedo;

		static ScriptUndoRedo* sGlobalUndoRedo;
		static HEvent sDomainLoadConn;
		
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_Undo(ScriptUndoRedo* thisPtr);
		static void internal_Redo(ScriptUndoRedo* thisPtr);
		static void internal_RegisterCommand(ScriptUndoRedo* thisPtr, ScriptCmdManaged* command);
		static void internal_PushGroup(ScriptUndoRedo* thisPtr, MonoString* name);
		static void internal_PopGroup(ScriptUndoRedo* thisPtr, MonoString* name);
		static void internal_Clear(ScriptUndoRedo* thisPtr);
		static UINT32 internal_GetTopCommandId(ScriptUndoRedo* thisPtr);
		static void internal_PopCommand(ScriptUndoRedo* thisPtr, UINT32 id);
		static void internal_RecordSO(ScriptSceneObject* soPtr, bool recordHierarchy, MonoString* description);
		static MonoObject* internal_CloneSO(ScriptSceneObject* soPtr, MonoString* description);
		static MonoArray* internal_CloneSOMulti(MonoArray* soPtrs, MonoString* description);
		static MonoObject* internal_Instantiate(ScriptPrefab* prefabPtr, MonoString* description);
		static MonoObject* internal_CreateSO(MonoString* name, MonoString* description);
		static void internal_DeleteSO(ScriptSceneObject* soPtr, MonoString* description);
		static void internal_ReparentSO(ScriptSceneObject* soPtr, ScriptSceneObject* parentSOPtr, MonoString* description);
		static void internal_ReparentSOMulti(MonoArray* soPtrs, ScriptSceneObject* parentSOPtr, MonoString* description);
		static void internal_BreakPrefab(ScriptSceneObject* soPtr, MonoString* description);
	};

	/** @} */
}