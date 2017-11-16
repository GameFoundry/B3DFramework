//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIVector4Field. */
	class BS_SCR_BED_EXPORT ScriptGUIVector4Field : public TScriptGUIElement<ScriptGUIVector4Field>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIVector4Field")

	private:
		/**
		 * Triggered when the value in the native vector field changes.
		 *
		 * @param[in]	newValue	New vector value.
		 */
		void onChanged(const Vector4& newValue);

		/** Triggered when the user confirms input in the native vector field. */
		void onConfirmed();

		ScriptGUIVector4Field(MonoObject* instance, GUIVector4Field* vector4Field);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIVector4Field* nativeInstance, Vector4* output);
		static void internal_setValue(ScriptGUIVector4Field* nativeInstance, Vector4* value);
		static void internal_hasInputFocus(ScriptGUIVector4Field* nativeInstance, bool* output);
		static void internal_setTint(ScriptGUIVector4Field* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL *OnChangedThunkDef) (MonoObject*, MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL *OnConfirmedThunkDef) (MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
}