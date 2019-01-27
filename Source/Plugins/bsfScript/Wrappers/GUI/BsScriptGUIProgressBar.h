//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIProgressBar.  */
	class BS_SCR_BE_EXPORT ScriptGUIProgressBar : public TScriptGUIElement <ScriptGUIProgressBar>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIProgressBar")

	private:
		ScriptGUIProgressBar(MonoObject* instance, GUIProgressBar* progressBar);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void internal_setPercent(ScriptGUIProgressBar* nativeInstance, float percent);
		static float internal_getPercent(ScriptGUIProgressBar* nativeInstance);
		static void internal_setTint(ScriptGUIProgressBar* nativeInstance, Color* color);
	};

	/** @} */
}