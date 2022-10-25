//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	struct __GUIContentInterop;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIToggle.  */
	class BS_SCR_BE_EXPORT ScriptGUIToggle : public TScriptGUIElement<ScriptGUIToggle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIToggle")

	private:
		ScriptGUIToggle(MonoObject* instance, GUIToggle* toggle);

		/**	Triggered when the native toggle button is clicked. */
		void OnClick();

		/**	Triggered when the native toggle button is hover over. */
		void OnHover();

		/**	Triggered when the pointer leaves the native toggle button. */
		void OnOut();

		/**	Triggered when the native toggle button is toggled. */
		void OnToggled(bool toggled);

		/**	Triggers when the native toggle button is double-clicked. */
		void OnDoubleClick();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoObject* toggleGroup, MonoString* style, MonoArray* guiOptions);
		static void InternalSetContent(ScriptGUIToggle* nativeInstance, __GUIContentInterop* content);
		static bool InternalGetValue(ScriptGUIToggle* nativeInstance);
		static void InternalSetValue(ScriptGUIToggle* nativeInstance, bool value);
		static void InternalSetTint(ScriptGUIToggle* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL* OnClickThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnHoverThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnOutThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnToggledThunkDef)(MonoObject*, bool toggled, MonoException**);
		typedef void(BS_THUNKCALL* OnDoubleClickThunkDef)(MonoObject*, MonoException**);

		static OnClickThunkDef onClickThunk;
		static OnHoverThunkDef onHoverThunk;
		static OnOutThunkDef onOutThunk;
		static OnToggledThunkDef onToggledThunk;
		static OnDoubleClickThunkDef onDoubleClickThunk;
	};

	/** @} */
} // namespace bs
