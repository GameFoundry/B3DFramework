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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUILabel")

	private:
		ScriptGUILabel(MonoObject* instance, GUILabel* label);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, 
			MonoArray* guiOptions);
		static void internal_setContent(ScriptGUILabel* nativeInstance, __GUIContentInterop* content);
		static void internal_setTint(ScriptGUILabel* nativeInstance, Color* color);
	};

	/** @} */
}