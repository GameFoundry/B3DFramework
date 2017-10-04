//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIRenderTexture. */
	class BS_SCR_BE_EXPORT ScriptGUIRenderTexture : public TScriptGUIElement < ScriptGUIRenderTexture >
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIRenderTexture")

	private:
		ScriptGUIRenderTexture(MonoObject* instance, GUIRenderTexture* texture);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, ScriptRenderTexture* texture, bool transparent,
			MonoString* style, MonoArray* guiOptions);
		static void internal_setTexture(ScriptGUIRenderTexture* nativeInstance, ScriptRenderTexture* texture);
		static void internal_setTint(ScriptGUIRenderTexture* nativeInstance, Color* color);
	};

	/** @} */
}