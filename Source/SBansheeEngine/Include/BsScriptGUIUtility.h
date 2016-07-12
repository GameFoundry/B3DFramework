//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptGUIElement.h"
#include "BsVector2I.h"
#include "BsRect2I.h"

namespace BansheeEngine
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