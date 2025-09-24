//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIClickable.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUIButton.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"

namespace b3d { struct __GUIContentInterop; }
namespace b3d { class GUIButton; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIButton : public TScriptGUIElementWrapper<GUIButton, ScriptGUIButton, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIButton")

		ScriptGUIButton(GUIButton* nativeObject);
		~ScriptGUIButton();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, __GUIContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, __GUIContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
