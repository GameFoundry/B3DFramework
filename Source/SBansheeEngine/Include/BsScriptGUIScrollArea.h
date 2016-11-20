//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"
#include "BsGUIScrollArea.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIScrollArea.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollArea : public TScriptGUIElement<ScriptGUIScrollArea>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIScrollArea")

		/**
		 * Initializes the interop object by providing it with the interop object for the internal layout held by the
		 * scroll area.
		 */
		 void initialize(ScriptGUIScrollAreaLayout* layout);

	private:
		friend class ScriptGUIScrollAreaLayout;

		ScriptGUIScrollArea(MonoObject* instance, GUIScrollArea* scrollArea);

		/** @copydoc TScriptGUIElement::destroy */
		void destroy() override;

		/**
		 * Called when the child script GUI layout gets destroyed. Notifies this object that it shouldn't use it anymore.
		 */
		void notifyLayoutDestroyed();

		ScriptGUIScrollAreaLayout* mLayout;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, ScrollBarType vertBarType, ScrollBarType horzBarType, 
			MonoString* scrollBarStyle, MonoString* scrollAreaStyle, MonoArray* guiOptions);
		static void internal_getContentBounds(ScriptGUIScrollArea* nativeInstance, Rect2I* bounds);
		static float internal_getHorzScroll(ScriptGUIScrollArea* nativeInstance);
		static void internal_setHorzScroll(ScriptGUIScrollArea* nativeInstance, float value);
		static float internal_getVertScroll(ScriptGUIScrollArea* nativeInstance);
		static void internal_setVertScroll(ScriptGUIScrollArea* nativeInstance, float value);
		static int internal_getScrollBarWidth(ScriptGUIScrollArea* nativeInstance);
	};

	/** @} */
}