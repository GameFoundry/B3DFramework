//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUITexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIInputBox. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInputBox : public TScriptGUIElement<ScriptGUIInputBox>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUITextBox")

	private:
		ScriptGUIInputBox(MonoObject* instance, GUIInputBox* inputBox);

		/**	Triggered when the value in the native input box changes. */
		void OnChanged(const String& newValue);

		/**	Triggered when the user confirms input in the native input box. */
		void OnConfirmed();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, bool multiline, MonoString* style, MonoArray* guiOptions);
		static void InternalSetText(ScriptGUIInputBox* nativeInstance, MonoString* text);
		static void InternalGetText(ScriptGUIInputBox* nativeInstance, MonoString** text);
		static void InternalSetTint(ScriptGUIInputBox* nativeInstance, Color* color);

		typedef void(B3D_THUNKCALL* OnChangedThunkDef)(MonoObject*, MonoString*, MonoException**);
		typedef void(B3D_THUNKCALL* OnConfirmedThunkDef)(MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
		static OnConfirmedThunkDef onConfirmedThunk;
	};

	/** @} */
} // namespace bs
