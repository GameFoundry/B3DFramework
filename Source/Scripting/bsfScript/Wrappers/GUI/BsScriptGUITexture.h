//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUITexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUITexture. */
	class BS_SCR_BE_EXPORT ScriptGUITexture : public TScriptGUIElement<ScriptGUITexture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUITexture")

	private:
		ScriptGUITexture(MonoObject* instance, GUITexture* texture);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoObject* texture, TextureScaleMode scale, bool transparent, MonoString* style, MonoArray* guiOptions);
		static void InternalSetTexture(ScriptGUITexture* nativeInstance, MonoObject* texture);
		static void InternalSetTint(ScriptGUITexture* nativeInstance, Color* color);
	};

	/** @} */
} // namespace bs
