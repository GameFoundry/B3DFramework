//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUIScrollArea.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIScrollArea.  */
	class BS_SCR_BE_EXPORT ScriptGUIScrollArea : public TScriptGUIElement<ScriptGUIScrollArea>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIScrollArea")

		/**
		 * Initializes the interop object by providing it with the interop object for the internal layout held by the
		 * scroll area.
		 */
		void Initialize(ScriptGUIScrollAreaLayout* layout);

	private:
		friend class ScriptGUIScrollAreaLayout;

		ScriptGUIScrollArea(MonoObject* instance, GUIScrollArea* scrollArea);

		/** @copydoc TScriptGUIElement::destroy */
		void Destroy();

		/**
		 * Called when the child script GUI layout gets destroyed. Notifies this object that it shouldn't use it anymore.
		 */
		void NotifyLayoutDestroyed();

		ScriptGUIScrollAreaLayout* mLayout;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, ScrollBarType vertBarType, ScrollBarType horzBarType, MonoString* scrollBarStyle, MonoString* scrollAreaStyle, MonoArray* guiOptions);
		static void InternalGetContentBounds(ScriptGUIScrollArea* nativeInstance, Rect2I* bounds);
		static float InternalGetHorzScroll(ScriptGUIScrollArea* nativeInstance);
		static void InternalSetHorzScroll(ScriptGUIScrollArea* nativeInstance, float value);
		static float InternalGetVertScroll(ScriptGUIScrollArea* nativeInstance);
		static void InternalSetVertScroll(ScriptGUIScrollArea* nativeInstance, float value);
		static int InternalGetScrollBarWidth(ScriptGUIScrollArea* nativeInstance);
	};

	/** @} */
} // namespace bs
