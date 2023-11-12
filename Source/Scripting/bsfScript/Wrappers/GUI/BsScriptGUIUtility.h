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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutUtility : public ScriptObject<ScriptGUILayoutUtility>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIUtility")

		ScriptGUILayoutUtility();

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCalculateOptimalSize(ScriptGUIElementBaseTBase* guiElement, Vector2I* output);
		static void InternalCalculateBounds(ScriptGUIElementBaseTBase* guiElement, ScriptGUILayout* relativeTo, Rect2I* output);
		static void InternalCalculateTextBounds(MonoString* text, ScriptFont* fontPtr, float fontSize, Vector2I* output);
	};

	/** @} */
} // namespace bs
