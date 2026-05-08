//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/GUI/B3DGUIToggleGroup.h"
#include "B3DScriptNonReflectableWrapper.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleGroup : public TScriptNonReflectableWrapper<GUIToggleGroup, ScriptGUIToggleGroup>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIToggleGroup")

		ScriptGUIToggleGroup(const TShared<GUIToggleGroup>& nativeObject);
		~ScriptGUIToggleGroup();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, bool allowAllOff);
	};
}
