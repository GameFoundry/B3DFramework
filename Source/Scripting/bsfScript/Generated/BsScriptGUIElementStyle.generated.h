//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "../../../Foundation/bsfEngine/Utility/BsRectOffset.h"
#include "../../../Foundation/bsfEngine/2D/BsTextSprite.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "../../../Foundation/bsfEngine/2D/BsTextSprite.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"

namespace bs { struct GUIElementStyle; }
namespace bs { struct __GUIElementStateStyleInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptGUIElementStyle : public TScriptReflectable<ScriptGUIElementStyle, GUIElementStyle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUIElementStyle")

		ScriptGUIElementStyle(MonoObject* managedInstance, const SPtr<GUIElementStyle>& value);

		static MonoObject* create(const SPtr<GUIElementStyle>& value);

	private:
		static void Internal_GUIElementStyle(MonoObject* managedInstance);
		static void Internal_addSubStyle(ScriptGUIElementStyle* thisPtr, MonoString* guiType, MonoString* styleName);
		static MonoObject* Internal_getfont(ScriptGUIElementStyle* thisPtr);
		static void Internal_setfont(ScriptGUIElementStyle* thisPtr, MonoObject* value);
		static uint32_t Internal_getfontSize(ScriptGUIElementStyle* thisPtr);
		static void Internal_setfontSize(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static TextHorzAlign Internal_gettextHorzAlign(ScriptGUIElementStyle* thisPtr);
		static void Internal_settextHorzAlign(ScriptGUIElementStyle* thisPtr, TextHorzAlign value);
		static TextVertAlign Internal_gettextVertAlign(ScriptGUIElementStyle* thisPtr);
		static void Internal_settextVertAlign(ScriptGUIElementStyle* thisPtr, TextVertAlign value);
		static GUIImagePosition Internal_getimagePosition(ScriptGUIElementStyle* thisPtr);
		static void Internal_setimagePosition(ScriptGUIElementStyle* thisPtr, GUIImagePosition value);
		static bool Internal_getwordWrap(ScriptGUIElementStyle* thisPtr);
		static void Internal_setwordWrap(ScriptGUIElementStyle* thisPtr, bool value);
		static void Internal_getnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setnormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_gethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_sethover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setactive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setfocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setfocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setnormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_gethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_sethoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setactiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setfocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void Internal_setfocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void Internal_getborder(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void Internal_setborder(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void Internal_getmargins(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void Internal_setmargins(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void Internal_getcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void Internal_setcontentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void Internal_getpadding(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void Internal_setpadding(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static uint32_t Internal_getwidth(ScriptGUIElementStyle* thisPtr);
		static void Internal_setwidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t Internal_getheight(ScriptGUIElementStyle* thisPtr);
		static void Internal_setheight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t Internal_getminWidth(ScriptGUIElementStyle* thisPtr);
		static void Internal_setminWidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t Internal_getmaxWidth(ScriptGUIElementStyle* thisPtr);
		static void Internal_setmaxWidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t Internal_getminHeight(ScriptGUIElementStyle* thisPtr);
		static void Internal_setminHeight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t Internal_getmaxHeight(ScriptGUIElementStyle* thisPtr);
		static void Internal_setmaxHeight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static bool Internal_getfixedWidth(ScriptGUIElementStyle* thisPtr);
		static void Internal_setfixedWidth(ScriptGUIElementStyle* thisPtr, bool value);
		static bool Internal_getfixedHeight(ScriptGUIElementStyle* thisPtr);
		static void Internal_setfixedHeight(ScriptGUIElementStyle* thisPtr, bool value);
	};
}
