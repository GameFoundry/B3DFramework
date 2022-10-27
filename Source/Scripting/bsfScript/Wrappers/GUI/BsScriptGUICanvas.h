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

	/**	Interop class between C++ & CLR for GUICanvas. */
	class BS_SCR_BE_EXPORT ScriptGUICanvas : public TScriptGUIElement<ScriptGUICanvas>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUICanvas")

	private:
		ScriptGUICanvas(MonoObject* instance, GUICanvas* canvas);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void InternalDrawLine(ScriptGUICanvas* nativeInstance, Vector2I* a, Vector2I* b, Color* color, u8 depth);
		static void InternalDrawPolyLine(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawTexture(ScriptGUICanvas* nativeInstance, ScriptSpriteTexture* texture, Rect2I* area, TextureScaleMode scaleMode, Color* color, u8 depth);
		static void InternalDrawTriangleStrip(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawTriangleList(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawText(ScriptGUICanvas* nativeInstance, MonoString* text, Vector2I* position, ScriptFont* font, u32 size, Color* color, u8 depth);
		static void InternalClear(ScriptGUICanvas* nativeInstance);
	};

	/** @} */
} // namespace bs
