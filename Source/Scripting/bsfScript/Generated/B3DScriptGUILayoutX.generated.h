//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUILayout.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUILayoutX.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"

namespace b3d { class GUILayoutX; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutX : public TScriptGUIElementWrapper<GUILayoutX, ScriptGUILayoutX, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayoutX")

		ScriptGUILayoutX(GUILayoutX* nativeObject);
		~ScriptGUILayoutX();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
