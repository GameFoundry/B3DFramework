//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"
#include "BsColor.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIButton. */
	class BS_SCR_BE_EXPORT ScriptGUIButton : public TScriptGUIElement<ScriptGUIButton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIButton")

	private:
		ScriptGUIButton(MonoObject* instance, GUIButton* button);

		/**	Triggers when the GUI button is clicked. */
		static void onClick(MonoObject* instance);

		/**	Triggers when the GUI button is double-clicked. */
		static void onDoubleClick(MonoObject* instance);

		/**	Triggers when the GUI button is hovered over. */
		static void onHover(MonoObject* instance);

		/**	Triggers when the pointer leaves the GUI button. */
		static void onOut(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* content, MonoString* style, MonoArray* guiOptions);
		static void internal_setContent(ScriptGUIButton* nativeInstance, MonoObject* content);
		static void internal_setTint(ScriptGUIButton* nativeInstance, Color* color);

		typedef void (__stdcall *OnClickThunkDef) (MonoObject*, MonoException**);
		typedef void (__stdcall *OnDoubleClickThunkDef) (MonoObject*, MonoException**);
		typedef void (__stdcall *OnHoverThunkDef) (MonoObject*, MonoException**);
		typedef void (__stdcall *OnOutThunkDef) (MonoObject*, MonoException**);

		static OnClickThunkDef onClickThunk;
		static OnDoubleClickThunkDef onDoubleClickThunk;
		static OnHoverThunkDef onHoverThunk;
		static OnOutThunkDef onOutThunk;
	};

	/** @} */
}