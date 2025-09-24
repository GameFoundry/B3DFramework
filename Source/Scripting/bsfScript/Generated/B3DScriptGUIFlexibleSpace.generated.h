//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISpace.h"

namespace b3d { class GUIFlexibleSpace; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIFlexibleSpace : public TScriptGUIElementWrapper<GUIFlexibleSpace, ScriptGUIFlexibleSpace>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIFlexibleSpace")

		ScriptGUIFlexibleSpace(GUIFlexibleSpace* nativeObject);
		~ScriptGUIFlexibleSpace();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject);
	};
}
