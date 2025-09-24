//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIToggleable.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUIToggle.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"
#include "../../../Foundation/Engine/GUI/B3DGUIToggleable.h"

namespace b3d { class GUIToggle; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d { struct __GUIToggleContentInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggle : public TScriptGUIElementWrapper<GUIToggle, ScriptGUIToggle, ScriptGUIToggleableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIToggle")

		ScriptGUIToggle(GUIToggle* nativeObject);
		~ScriptGUIToggle();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, __GUIToggleContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, __GUIToggleContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
