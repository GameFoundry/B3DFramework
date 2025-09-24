//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIInteractable.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"

namespace b3d { class GUITexture; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d { struct __GUITextureContentsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUITexture : public TScriptGUIElementWrapper<GUITexture, ScriptGUITexture, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUITexture")

		ScriptGUITexture(GUITexture* nativeObject);
		~ScriptGUITexture();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetImage(ScriptGUITexture* self, MonoObject* image);
		static void InternalCreate(MonoObject* scriptObject, __GUITextureContentsInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, __GUITextureContentsInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
