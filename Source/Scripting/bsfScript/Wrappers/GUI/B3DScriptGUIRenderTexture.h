//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "GUI/B3DGUIRenderTexture.h"

#ifndef B3D_CODEGEN
#include "Generated/B3DScriptGUIElement.generated.h"
#include "Generated/B3DScriptGUIInteractable.generated.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIRenderTexture. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIRenderTexture : public TScriptGUIElementWrapper<GUIRenderTexture, ScriptGUIRenderTexture, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIRenderTexture")

		ScriptGUIRenderTexture(GUIRenderTexture* nativeObject);

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

		/** Returns the native object that is being wrapped. */
		GUIRenderTexture* GetNativeObject() const { return static_cast<GUIRenderTexture*>(mNativeObject); }

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, ScriptRenderTexture* texture, bool transparent, MonoString* style, MonoArray* guiOptions);
		static void InternalSetTexture(ScriptGUIRenderTexture* self, ScriptRenderTexture* texture);
	};

	/** @} */
} // namespace b3d
#endif
