//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/GUI/B3DGUIPanel.h"
#include "../../../Foundation/Core/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Core/GUI/B3DGUIUnits.h"

namespace b3d { class GUIWidget; }
namespace b3d { struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d { struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIWidget : public TScriptGameObjectWrapper<GUIWidget, ScriptGUIWidget>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIWidget")

		ScriptGUIWidget(const TGameObjectHandle<GUIWidget>& nativeObject);
		~ScriptGUIWidget();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetPanel(ScriptGUIWidget* self);
		static void InternalSetDepth(ScriptGUIWidget* self, uint8_t depth);
		static uint8_t InternalGetDepth(ScriptGUIWidget* self);
		static bool InternalInBounds(ScriptGUIWidget* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position);
		static void InternalGetBounds(ScriptGUIWidget* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
	};
}
