//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleGroup.h"
#include "BsScriptNonReflectableWrapper.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleGroup : public TScriptNonReflectableWrapper<GUIToggleGroup, ScriptGUIToggleGroup>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIToggleGroup")

		ScriptGUIToggleGroup(const SPtr<GUIToggleGroup>& nativeObject);
		~ScriptGUIToggleGroup();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, bool allowAllOff);
	};
}
