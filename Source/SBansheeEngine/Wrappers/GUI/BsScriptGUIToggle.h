//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIToggle.  */
	class BS_SCR_BE_EXPORT ScriptGUIToggle : public TScriptGUIElement<ScriptGUIToggle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIToggle")

	private:
		ScriptGUIToggle(MonoObject* instance, GUIToggle* toggle);

		/**	Triggered when the native toggle button is clicked. */
		void onClick();

		/**	Triggered when the native toggle button is hover over. */
		void onHover();

		/**	Triggered when the pointer leaves the native toggle button. */
		void onOut();

		/**	Triggered when the native toggle button is toggled. */
		void onToggled(bool toggled);

		/**	Triggers when the native toggle button is double-clicked. */
		void onDoubleClick();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* content,
			MonoObject* toggleGroup, MonoString* style, MonoArray* guiOptions);
		static void internal_setContent(ScriptGUIToggle* nativeInstance, MonoObject* content);
		static bool internal_getValue(ScriptGUIToggle* nativeInstance);
		static void internal_setValue(ScriptGUIToggle* nativeInstance, bool value);
		static void internal_setTint(ScriptGUIToggle* nativeInstance, Color* color);

		typedef void (BS_THUNKCALL *OnClickThunkDef) (MonoObject*, MonoException**);
		typedef void (BS_THUNKCALL *OnHoverThunkDef) (MonoObject*, MonoException**);
		typedef void (BS_THUNKCALL *OnOutThunkDef) (MonoObject*, MonoException**);
		typedef void (BS_THUNKCALL *OnToggledThunkDef) (MonoObject*, bool toggled, MonoException**);
		typedef void(BS_THUNKCALL *OnDoubleClickThunkDef) (MonoObject*, MonoException**);

		static OnClickThunkDef onClickThunk;
		static OnHoverThunkDef onHoverThunk;
		static OnOutThunkDef onOutThunk;
		static OnToggledThunkDef onToggledThunk;
		static OnDoubleClickThunkDef onDoubleClickThunk;
	};

	/** @} */
}