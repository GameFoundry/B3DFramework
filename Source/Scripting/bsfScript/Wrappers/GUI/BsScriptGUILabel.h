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

	/**	Interop class between C++ & CLR for GUILabel. */
	class BS_SCR_BE_EXPORT ScriptGUILabel : public TScriptGUIElement<ScriptGUILabel>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUILabel")

	private:
		ScriptGUILabel(MonoObject* instance, GUILabel* label);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, MonoArray* guiOptions);
		static void InternalSetContent(ScriptGUILabel* nativeInstance, __GUIContentInterop* content);
		static void InternalSetTint(ScriptGUILabel* nativeInstance, Color* color);
	};

	/** @} */
} // namespace bs
