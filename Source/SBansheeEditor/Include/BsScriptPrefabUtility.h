//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PrefabUtility. */
	class BS_SCR_BED_EXPORT ScriptPrefabUtility : public ScriptObject <ScriptPrefabUtility>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "PrefabUtility")

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_breakPrefab(ScriptSceneObject* soPtr);
		static void internal_applyPrefab(ScriptSceneObject* soPtr, ScriptPrefab* prefabPtr);
		static void internal_revertPrefab(ScriptSceneObject* soPtr);
		static bool internal_hasPrefabLink(ScriptSceneObject* soPtr);
		static MonoObject* internal_getPrefabParent(ScriptSceneObject* soPtr);
		static MonoString* internal_GetPrefabUUID(ScriptSceneObject* soPtr);
		static void internal_UpdateFromPrefab(ScriptSceneObject* soPtr);
		static void internal_RecordPrefabDiff(ScriptSceneObject* soPtr);

		ScriptPrefabUtility(MonoObject* instance);
	};

	/** @} */
}