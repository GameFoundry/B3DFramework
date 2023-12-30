//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	class GUIResizableVerticalScrollBar;
	class GUIResizableHorizontalScrollBar;
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	class GUIScrollBar;

	/**	Interop class between C++ & CLR for GUIScrollBar.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBar : public TScriptGUIInteractable<ScriptGUIScrollBar>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBar")

	private:
		ScriptGUIScrollBar(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static float InternalGetPosition(ScriptGUIElementBase* nativeInstance);
		static void InternalSetPosition(ScriptGUIElementBase* nativeInstance, float percent);
		static float InternalGetHandleSize(ScriptGUIElementBase* nativeInstance);
		static void InternalSetHandleSize(ScriptGUIElementBase* nativeInstance, float percent);
	};

	/**	Interop class between C++ & CLR for GUIScrollBarH.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBarH : public TScriptGUIInteractable<ScriptGUIScrollBarH>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBarH")

	private:
		ScriptGUIScrollBarH(MonoObject* instance, GUIHorizontalScrollBar* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(B3D_THUNKCALL* OnScrolledThunkDef)(MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIScrollBarV.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBarV : public TScriptGUIInteractable<ScriptGUIScrollBarV>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollBarV")

	private:
		ScriptGUIScrollBarV(MonoObject* instance, GUIVerticalScrollBar* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(B3D_THUNKCALL* OnScrolledThunkDef)(MonoObject*, float, MonoException**);
		static OnScrolledThunkDef onScrolledThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarH.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIResizeableScrollBarH : public TScriptGUIInteractable<ScriptGUIResizeableScrollBarH>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIResizeableScrollBarH")

	private:
		ScriptGUIResizeableScrollBarH(MonoObject* instance, GUIResizableHorizontalScrollBar* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(B3D_THUNKCALL* OnScrollOrResizeThunkDef)(MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/**	Interop class between C++ & CLR for GUIResizeableScrollBarV.  */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIResizeableScrollBarV : public TScriptGUIInteractable<ScriptGUIResizeableScrollBarV>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIResizeableScrollBarV")

	private:
		ScriptGUIResizeableScrollBarV(MonoObject* instance, GUIResizableVerticalScrollBar* scrollBar);

		/**	Triggers when the user scrolls the scroll bar. */
		void OnScroll(float position, float size);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions);

		typedef void(B3D_THUNKCALL* OnScrollOrResizeThunkDef)(MonoObject*, float, float, MonoException**);
		static OnScrollOrResizeThunkDef onScrollOrResizeThunk;
	};

	/** @} */
} // namespace bs
