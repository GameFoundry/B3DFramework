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

	/**	Interop class between C++ & CLR for browse dialog methods in Platform. */
	class BS_SCR_BED_EXPORT ScriptBrowseDialog : public ScriptObject<ScriptBrowseDialog>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "BrowseDialog")

	private:

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_OpenFile(MonoString* defaultFolder, MonoString* filterList, bool allowMultiselect, 
			MonoArray** outPaths);
		static bool internal_OpenFolder(MonoString* defaultFolder, MonoString** outPath);
		static bool internal_SaveFile(MonoString* defaultFolder, MonoString* filterList, MonoString** outPath);
	};

	/** @} */
}