//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"
#include "BsGUITexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIInputBox. */
	class BS_SCR_BE_EXPORT ScriptGUIInputBox : public TScriptGUIElement<ScriptGUIInputBox>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUITextBox")

	private:
		ScriptGUIInputBox(MonoObject* instance, GUIInputBox* inputBox);

		/**	Triggered when the value in the native input box changes. */
		static void onChanged(MonoObject* instance, const WString& newValue);

		/**	Triggered when the user confirms input in the native input box. */
		static void onConfirmed(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, bool multiline, MonoString* style, MonoArray* guiOptions);
		static void internal_setText(ScriptGUIInputBox* nativeInstance, MonoString* text);
		static void internal_getText(ScriptGUIInputBox* nativeInstance, MonoString** text);
		static void internal_setTint(ScriptGUIInputBox* nativeInstance, Color* color);

		typedef void(__stdcall *OnChangedThunkDef) (MonoObject*, MonoString*, MonoException**);
		typedef void(__stdcall *OnConfirmedThunkDef) (MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
}