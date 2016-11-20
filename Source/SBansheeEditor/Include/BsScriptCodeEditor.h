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

	/**	Interop class between C++ & CLR for CodeEditorManager. */
	class BS_SCR_BED_EXPORT ScriptCodeEditor : public ScriptObject<ScriptCodeEditor>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "CodeEditor")

	private:
		ScriptCodeEditor(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetActiveEditor(CodeEditorType type);
		static CodeEditorType internal_GetActiveEditor();
		static MonoArray* internal_GetAvailableEditors();
		static void internal_OpenFile(MonoString* path, UINT32 line);
		static void internal_SyncSolution();
	};

	/** @} */
}