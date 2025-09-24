//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISpace.h"

namespace b3d { class GUIFixedSpace; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIFixedSpace : public TScriptGUIElementWrapper<GUIFixedSpace, ScriptGUIFixedSpace>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIFixedSpace")

		ScriptGUIFixedSpace(GUIFixedSpace* nativeObject);
		~ScriptGUIFixedSpace();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetSize(ScriptGUIFixedSpace* self, TUnitValue<int32_t, LogicalPixel>* __output);
		static void InternalSetSize(ScriptGUIFixedSpace* self, TUnitValue<int32_t, LogicalPixel>* size);
		static void InternalCreate(MonoObject* scriptObject, TUnitValue<int32_t, LogicalPixel>* size);
	};
}
