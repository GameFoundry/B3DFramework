//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElement.h"
#include "BsGUITexture.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUITexture. */
	class BS_SCR_BE_EXPORT ScriptGUITexture : public TScriptGUIElement<ScriptGUITexture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUITexture")

	private:
		ScriptGUITexture(MonoObject* instance, GUITexture* texture);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* texture, 
			TextureScaleMode scale, bool transparent, MonoString* style, MonoArray* guiOptions);
		static void internal_setTexture(ScriptGUITexture* nativeInstance, MonoObject* texture);
		static void internal_setTint(ScriptGUITexture* nativeInstance, Color* color);
	};

	/** @} */
}