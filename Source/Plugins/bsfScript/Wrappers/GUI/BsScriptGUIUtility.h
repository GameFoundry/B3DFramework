//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "Math/BsVector2I.h"
#include "Math/BsRect2I.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUILayoutUtility. */
	class BS_SCR_BE_EXPORT ScriptGUILayoutUtility : public ScriptObject<ScriptGUILayoutUtility>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIUtility")

		ScriptGUILayoutUtility();

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CalculateOptimalSize(ScriptGUIElementBaseTBase* guiElement, Vector2I* output);
		static void internal_CalculateBounds(ScriptGUIElementBaseTBase* guiElement, ScriptGUILayout* relativeTo, Rect2I* output);
		static void internal_CalculateTextBounds(MonoString* text, ScriptFont* fontPtr, int fontSize, Vector2I* output);
	};

	/** @} */
}