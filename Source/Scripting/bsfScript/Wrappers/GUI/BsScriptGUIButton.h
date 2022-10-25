//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "Image/BsColor.h"

namespace bs
{
	struct __GUIContentInterop;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIButton. */
	class BS_SCR_BE_EXPORT ScriptGUIButton : public TScriptGUIElement<ScriptGUIButton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIButton")

	private:
		ScriptGUIButton(MonoObject* instance, GUIButton* button);

		/**	Triggers when the GUI button is clicked. */
		void OnClick();

		/**	Triggers when the GUI button is double-clicked. */
		void OnDoubleClick();

		/**	Triggers when the GUI button is hovered over. */
		void OnHover();

		/**	Triggers when the pointer leaves the GUI button. */
		void OnOut();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, MonoArray* guiOptions);
		static void InternalSetContent(ScriptGUIButton* nativeInstance, __GUIContentInterop* content);
		static void InternalSetTint(ScriptGUIButton* nativeInstance, Color* color);

		typedef void(BS_THUNKCALL* OnClickThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnDoubleClickThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnHoverThunkDef)(MonoObject*, MonoException**);
		typedef void(BS_THUNKCALL* OnOutThunkDef)(MonoObject*, MonoException**);

		static OnClickThunkDef onClickThunk;
		static OnDoubleClickThunkDef onDoubleClickThunk;
		static OnHoverThunkDef onHoverThunk;
		static OnOutThunkDef onOutThunk;
	};

	/** @} */
} // namespace bs
