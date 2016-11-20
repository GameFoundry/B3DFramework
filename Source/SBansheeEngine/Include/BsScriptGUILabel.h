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

	/**	Interop class between C++ & CLR for GUILabel. */
	class BS_SCR_BE_EXPORT ScriptGUILabel : public TScriptGUIElement<ScriptGUILabel>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUILabel")

	private:
		ScriptGUILabel(MonoObject* instance, GUILabel* label);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* content, MonoString* style, MonoArray* guiOptions);
		static void internal_setContent(ScriptGUILabel* nativeInstance, MonoObject* content);
		static void internal_setTint(ScriptGUILabel* nativeInstance, Color* color);
	};

	/** @} */
}