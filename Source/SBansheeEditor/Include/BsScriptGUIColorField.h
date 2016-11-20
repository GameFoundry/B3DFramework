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

	/**	Interop class between C++ & CLR for GUIColorField. */
	class BS_SCR_BED_EXPORT ScriptGUIColorField : public TScriptGUIElement<ScriptGUIColorField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIColorField")

	private:
		ScriptGUIColorField(MonoObject* instance, GUIColorField* colorField);

		/**
		 * Triggered when the user clicks on the native color field.
		 *
		 * @param[in]	instance	Managed GUIColorField instance.
		 */
		static void onClicked(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIColorField* nativeInstance, Color* output);
		static void internal_setValue(ScriptGUIColorField* nativeInstance, Color* value);
		static void internal_setTint(ScriptGUIColorField* nativeInstance, Color* color);

		typedef void(__stdcall *OnClickedThunkDef) (MonoObject*, MonoException**);

		static OnClickedThunkDef onClickedThunk;
	};

	/** @} */
}