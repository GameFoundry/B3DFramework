//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "../../../Foundation/bsfEngine/2D/BsTextSprite.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "../../../Foundation/bsfEngine/2D/BsTextSprite.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "../../../Foundation/bsfEngine/Utility/BsRectOffset.h"

namespace bs { struct GUIElementStyle; }
namespace bs { struct __GUIElementStateStyleInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElementStyle : public TScriptReflectable<ScriptGUIElementStyle, GUIElementStyle>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIElementStyle")

		ScriptGUIElementStyle(MonoObject* managedInstance, const SPtr<GUIElementStyle>& value);

		static MonoObject* Create(const SPtr<GUIElementStyle>& value);

	private:
		static void InternalGUIElementStyle(MonoObject* managedInstance);
		static void InternalAddSubStyle(ScriptGUIElementStyle* thisPtr, MonoString* guiType, MonoString* styleName);
		static MonoObject* InternalGetFont(ScriptGUIElementStyle* thisPtr);
		static void InternalSetFont(ScriptGUIElementStyle* thisPtr, MonoObject* value);
		static float InternalGetFontSize(ScriptGUIElementStyle* thisPtr);
		static void InternalSetFontSize(ScriptGUIElementStyle* thisPtr, float value);
		static GUIHorizontalTextAlignment InternalGetTextHorzAlign(ScriptGUIElementStyle* thisPtr);
		static void InternalSetTextHorzAlign(ScriptGUIElementStyle* thisPtr, GUIHorizontalTextAlignment value);
		static GUIVerticalTextAlignment InternalGetTextVertAlign(ScriptGUIElementStyle* thisPtr);
		static void InternalSetTextVertAlign(ScriptGUIElementStyle* thisPtr, GUIVerticalTextAlignment value);
		static GUIImagePosition InternalGetImagePosition(ScriptGUIElementStyle* thisPtr);
		static void InternalSetImagePosition(ScriptGUIElementStyle* thisPtr, GUIImagePosition value);
		static bool InternalGetWordWrap(ScriptGUIElementStyle* thisPtr);
		static void InternalSetWordWrap(ScriptGUIElementStyle* thisPtr, bool value);
		static void InternalGetNormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetNormal(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetActive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetActive(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetFocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetFocused(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetFocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetFocusedHover(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetNormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetNormalOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetActiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetActiveOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetFocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetFocusedOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetFocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* __output);
		static void InternalSetFocusedHoverOn(ScriptGUIElementStyle* thisPtr, __GUIElementStateStyleInterop* value);
		static void InternalGetBorder(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void InternalSetBorder(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void InternalGetMargins(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void InternalSetMargins(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void InternalGetContentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void InternalSetContentOffset(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static void InternalGetPadding(ScriptGUIElementStyle* thisPtr, RectOffset* __output);
		static void InternalSetPadding(ScriptGUIElementStyle* thisPtr, RectOffset* value);
		static uint32_t InternalGetWidth(ScriptGUIElementStyle* thisPtr);
		static void InternalSetWidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t InternalGetHeight(ScriptGUIElementStyle* thisPtr);
		static void InternalSetHeight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t InternalGetMinWidth(ScriptGUIElementStyle* thisPtr);
		static void InternalSetMinWidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t InternalGetMaxWidth(ScriptGUIElementStyle* thisPtr);
		static void InternalSetMaxWidth(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t InternalGetMinHeight(ScriptGUIElementStyle* thisPtr);
		static void InternalSetMinHeight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static uint32_t InternalGetMaxHeight(ScriptGUIElementStyle* thisPtr);
		static void InternalSetMaxHeight(ScriptGUIElementStyle* thisPtr, uint32_t value);
		static bool InternalGetFixedWidth(ScriptGUIElementStyle* thisPtr);
		static void InternalSetFixedWidth(ScriptGUIElementStyle* thisPtr, bool value);
		static bool InternalGetFixedHeight(ScriptGUIElementStyle* thisPtr);
		static void InternalSetFixedHeight(ScriptGUIElementStyle* thisPtr, bool value);
	};
}
