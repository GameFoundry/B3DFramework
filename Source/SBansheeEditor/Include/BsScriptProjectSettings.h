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

	/**	Interop class between C++ & CLR for ProjectSettings stored in EditorApplication. */
	class BS_SCR_BED_EXPORT ScriptProjectSettings : public ScriptObject <ScriptProjectSettings>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ProjectSettings")

	private:
		ScriptProjectSettings(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* internal_GetLastOpenScene();
		static void internal_SetLastOpenScene(MonoString* value);

		static void internal_SetFloat(MonoString* name, float value);
		static void internal_SetInt(MonoString* name, int value);
		static void internal_SetBool(MonoString* name, bool value);
		static void internal_SetString(MonoString* name, MonoString* value);

		static float internal_GetFloat(MonoString* name, float defaultValue);
		static int internal_GetInt(MonoString* name, int defaultValue);
		static bool internal_GetBool(MonoString* name, bool defaultValue);
		static MonoString* internal_GetString(MonoString* name, MonoString* defaultValue);

		static bool internal_HasKey(MonoString* name);
		static void internal_DeleteKey(MonoString* name);
		static void internal_DeleteAllKeys();

		static UINT32 internal_GetHash();
		static void internal_Save();
	};

	/** @} */
}