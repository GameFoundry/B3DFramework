//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"

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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIScrollBar")

	private:
		ScriptGUIScrollBar(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static float internal_GetPosition(ScriptGUIElementBaseTBase* nativeInstance);
		static void internal_SetPosition(ScriptGUIElementBaseTBase* nativeInstance, float percent);
		static float internal_GetHandleSize(ScriptGUIElementBaseTBase* nativeInstance);
		static void internal_SetHandleSize(ScriptGUIElementBaseTBase* nativeInstance, float percent);
		static void internal_SetTint(ScriptGUIElementBaseTBase* nativeInstance, Color* color);
	};

	/**	Interop class between C++ & CLR for GUIScrollBarH.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollBarH : public TScriptGUIElement<ScriptGUIScrollBarH>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIScrollBarH")

	private:
		ScriptGUIScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void onScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(__stdcall *OnScrolledThunkDef) (MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIScrollBarV.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollBarV : public TScriptGUIElement<ScriptGUIScrollBarV>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIScrollBarV")

	private:
		ScriptGUIScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void onScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(__stdcall *OnScrolledThunkDef) (MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarH.  */
	class BS_SCR_BE_EXPORT ScriptGUIResizeableScrollBarH : public TScriptGUIElement<ScriptGUIResizeableScrollBarH>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIResizeableScrollBarH")

	private:
		ScriptGUIResizeableScrollBarH(MonoObject* instance, GUIScrollBarHorz* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void onScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(__stdcall *OnScrollOrResizeThunkDef) (MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarV.  */
	class BS_SCR_BE_EXPORT ScriptGUIResizeableScrollBarV : public TScriptGUIElement<ScriptGUIResizeableScrollBarV>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIResizeableScrollBarV")

	private:
		ScriptGUIResizeableScrollBarV(MonoObject* instance, GUIScrollBarVert* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void onScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(__stdcall *OnScrollOrResizeThunkDef) (MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/** @} */
}