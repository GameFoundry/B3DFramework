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

	/**	Interop class between C++ & CLR for EditorUtility. */
	class BS_SCR_BED_EXPORT ScriptEditorUtility : public ScriptObject <ScriptEditorUtility>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "EditorUtility")

	private:
		ScriptEditorUtility(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CalculateBounds(MonoObject* so, AABox* bounds);
		static void internal_CalculateBoundsArray(MonoArray* objects, AABox* bounds);
		static MonoArray* internal_FindDependencies(MonoObject* resource, bool recursive);
		static bool internal_IsInternal(ScriptSceneObject* soPtr);
		static void internal_CalculateArrayCenter(MonoArray* objects, Vector3* center);
	};

	/** @} */
}