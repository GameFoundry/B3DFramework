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

	class GUIScrollBar;

	/**	Interop class between C++ & CLR for GUIScrollBar.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollBar : public TScriptGUIElement<ScriptGUIScrollBar>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBar")

	private:
		ScriptGUIScrollBar(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static float InternalGetPosition(ScriptGUIElementBaseTBase* nativeInstance);
		static void InternalSetPosition(ScriptGUIElementBaseTBase* nativeInstance, float percent);
		static float InternalGetHandleSize(ScriptGUIElementBaseTBase* nativeInstance);
		static void InternalSetHandleSize(ScriptGUIElementBaseTBase* nativeInstance, float percent);
		static void InternalSetTint(ScriptGUIElementBaseTBase* nativeInstance, Color* color);
	};

	/**	Interop class between C++ & CLR for GUIScrollBarH.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollBarH : public TScriptGUIElement<ScriptGUIScrollBarH>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBarH")

	private:
		ScriptGUIScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(BS_THUNKCALL* OnScrolledThunkDef)(MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIScrollBarV.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollBarV : public TScriptGUIElement<ScriptGUIScrollBarV>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBarV")

	private:
		ScriptGUIScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(BS_THUNKCALL* OnScrolledThunkDef)(MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarH.  */
	class BS_SCR_BE_EXPORT ScriptGUIResizeableScrollBarH : public TScriptGUIElement<ScriptGUIResizeableScrollBarH>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIResizeableScrollBarH")

	private:
		ScriptGUIResizeableScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(BS_THUNKCALL* OnScrollOrResizeThunkDef)(MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarV.  */
	class BS_SCR_BE_EXPORT ScriptGUIResizeableScrollBarV : public TScriptGUIElement<ScriptGUIResizeableScrollBarV>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIResizeableScrollBarV")

	private:
		ScriptGUIResizeableScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(BS_THUNKCALL* OnScrollOrResizeThunkDef)(MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/** @} */
} // namespace bs
