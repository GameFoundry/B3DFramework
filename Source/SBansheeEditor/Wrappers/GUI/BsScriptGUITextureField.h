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

	/**	Interop class between C++ & CLR for GUITextureField. */
	class BS_SCR_BED_EXPORT ScriptGUITextureField : public TScriptGUIElement <ScriptGUITextureField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUITextureField")

	private:
		/**
		 * Triggered when the value in the native texture field changes.
		 *
		 * @param[in]	newHandle	Handle of the new texture.
		 */
		void onChanged(const WeakResourceHandle<Texture>& newHandle);

		/** Retrieves a managed instance of the specified native texture. Will return null if one doesn't exist. */
		static MonoObject* nativeToManagedResource(const HTexture& instance);

		ScriptGUITextureField(MonoObject* instance, GUITextureField* textureField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/

		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUITextureField* nativeInstance, MonoObject** output);
		static void internal_setValue(ScriptGUITextureField* nativeInstance, MonoObject* value);
		static void internal_getValueRef(ScriptGUITextureField* nativeInstance, MonoObject** output);
		static void internal_setValueRef(ScriptGUITextureField* nativeInstance, MonoObject* value);
		static void internal_setTint(ScriptGUITextureField* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL *OnChangedThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};

	/** @} */
}