//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptGUIElementStateStyle.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIElementStyle. */
	class BS_SCR_BE_EXPORT ScriptGUIElementStyle : public ScriptObject<ScriptGUIElementStyle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElementStyle")

		~ScriptGUIElementStyle();

		/**	Returns the wrapped GUIElementStyle instance. */
		GUIElementStyle& getInternalValue() { return mElementStyle; }

		/**	Creates a new managed object containing a copy of the provided style. */
		static MonoObject* create(const GUIElementStyle& style);

	private:
		/**	Creates the interop object with a default style. */
		ScriptGUIElementStyle(MonoObject* instance);

		/**	Creates the interop object referencing an existing style. */
		ScriptGUIElementStyle(MonoObject* instance, const GUIElementStyle& externalStyle);

		GUIElementStyle mElementStyle;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance);
		static void internal_addSubStyle(ScriptGUIElementStyle* nativeInstance, MonoString* guiType, MonoString* styleName);

		static void internal_GetFont(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetFont(ScriptGUIElementStyle* nativeInstance, MonoObject* value);

		static void internal_GetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign*value);
		static void internal_SetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign value);
		static void internal_GetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign* value);
		static void internal_SetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign value);
		static void internal_GetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition* value);
		static void internal_SetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition value);
		static void internal_GetWordWrap(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetWordWrap(ScriptGUIElementStyle* nativeInstance, bool value);

		static void internal_GetNormal(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetNormal(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetHover(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetHover(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetActive(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetActive(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetFocused(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetFocused(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);

		static void internal_GetNormalOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetNormalOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetHoverOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetHoverOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetActiveOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetActiveOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_GetFocusedOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);
		static void internal_SetFocusedOn(ScriptGUIElementStyle* nativeInstance, ScriptGUIElementStateStyleStruct* value);

		static void internal_GetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_GetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_GetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value);

		static void internal_GetWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool value);
		static void internal_GetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool value);
	};

	/** @} */
}