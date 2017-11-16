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

	/**	Interop class between C++ & CLR for GUIResourceField. */
	class BS_SCR_BED_EXPORT ScriptGUIResourceField : public TScriptGUIElement<ScriptGUIResourceField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIResourceField")

	private:
		/**
		 * Triggered when the value in the native resource field changes.
		 *
		 * @param[in]	newHandle	Weak handle of the newly selected resource.
		 */
		void onChanged(const WeakResourceHandle<Resource>& newHandle);

		/** Retrieves a managed instance of the specified native resource. Will return null if one doesn't exist. */
		static MonoObject* nativeToManagedResource(const HResource& instance);

		ScriptGUIResourceField(MonoObject* instance, GUIResourceField* resourceField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoReflectionType* type, MonoObject* title, 
			UINT32 titleWidth, MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIResourceField* nativeInstance, MonoObject** output);
		static void internal_setValue(ScriptGUIResourceField* nativeInstance, MonoObject* value);
		static void internal_getValueRef(ScriptGUIResourceField* nativeInstance, MonoObject** output);
		static void internal_setValueRef(ScriptGUIResourceField* nativeInstance, MonoObject* value);
		static void internal_setTint(ScriptGUIResourceField* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL *OnChangedThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};

	/** @} */
}