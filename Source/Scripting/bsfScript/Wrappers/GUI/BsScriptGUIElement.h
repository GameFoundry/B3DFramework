//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIInteractable.h"

namespace bs
{
	class ScriptGUILayoutWrapperBase;
}

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for GUIElement. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElement : public TScriptGUIElementWrapper<GUIElement, ScriptGUIElement>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIElement")

		ScriptGUIElement();

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct) { return nullptr; }
	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalDestroy(ScriptGUIElementWrapper* self);
		static void InternalSetVisible(ScriptGUIElementWrapper* self, bool visible);
		static bool InternalGetVisible(ScriptGUIElementWrapper* self);
		static void InternalSetActive(ScriptGUIElementWrapper* self, bool active);
		static bool InternalGetActive(ScriptGUIElementWrapper* self);
		static void InternalSetDisabled(ScriptGUIElementWrapper* self, bool disabled);
		static bool InternalGetDisabled(ScriptGUIElementWrapper* self);
		static MonoObject* InternalGetParent(ScriptGUIElementWrapper* self);
		static void InternalGetBounds(ScriptGUIElementWrapper* self, Rect2I* bounds);
		static void InternalSetBounds(ScriptGUIElementWrapper* self, Rect2I* bounds);
		static void InternalGetVisibleBounds(ScriptGUIElementWrapper* self, Rect2I* bounds);
		static void InternalGetScreenBounds(ScriptGUIElementWrapper* self, Rect2I* bounds);
		static void InternalCalculateBoundsRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, Rect2I* bounds);
		static void InternalSetPosition(ScriptGUIElementWrapper* self, i32 x, i32 y);
		static void InternalSetWidth(ScriptGUIElementWrapper* self, u32 width);
		static void InternalSetHeight(ScriptGUIElementWrapper* self, u32 height);
		static void InternalSetFlexibleWidth(ScriptGUIElementWrapper* self, u32 minWidth, u32 maxWidth);
		static void InternalSetFlexibleHeight(ScriptGUIElementWrapper* self, u32 minHeight, u32 maxHeight);
		static void InternalResetDimensions(ScriptGUIElementWrapper* self);
	};

	/** @} */
} // namespace bs
