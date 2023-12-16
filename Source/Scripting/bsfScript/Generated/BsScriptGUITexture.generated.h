//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"

namespace bs { class GUITexture; }
namespace bs { struct __GUITextureContentsInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUITexture : public TScriptGUIElement<ScriptGUITexture>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUITexture")

		ScriptGUITexture(MonoObject* managedInstance, GUITexture* value);

	private:
		static void InternalSetImage(ScriptGUITexture* thisPtr, MonoObject* image);
		static void InternalCreate(MonoObject* managedInstance, __GUITextureContentsInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, __GUITextureContentsInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* options);
	};
}
