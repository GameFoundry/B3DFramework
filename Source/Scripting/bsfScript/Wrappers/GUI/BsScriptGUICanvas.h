//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUICanvas.h"

namespace bs
{
	class ScriptSpriteImage;
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUICanvas. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUICanvas : public TScriptGUIElementWrapper<GUICanvas, ScriptGUICanvas, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUICanvas")

		ScriptGUICanvas(GUICanvas* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

		/** Returns the native object that is being wrapped. */
		GUICanvas* GetNativeObject() const { return static_cast<GUICanvas*>(mNativeObject); }
	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);
		static void InternalDrawLine(ScriptGUICanvas* self, Vector2I* a, Vector2I* b, Color* color, u8 depth);
		static void InternalDrawPolyLine(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawImage(ScriptGUICanvas* self, ScriptSpriteImage* texture, Rect2I* area, TextureScaleMode scaleMode, Color* color, u8 depth);
		static void InternalDrawTriangleStrip(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawTriangleList(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth);
		static void InternalDrawText(ScriptGUICanvas* self, MonoString* text, Vector2I* position, ScriptFont* font, float size, Color* color, u8 depth);
		static void InternalClear(ScriptGUICanvas* self);
	};

	/** @} */
} // namespace bs
