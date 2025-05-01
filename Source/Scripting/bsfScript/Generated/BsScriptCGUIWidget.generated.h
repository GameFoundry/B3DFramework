//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIPanel.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"

namespace bs { class CGUIWidget; }
namespace bs { struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace bs { struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIWidget : public TScriptGameObjectWrapper<CGUIWidget, ScriptGUIWidget>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIWidget")

		ScriptGUIWidget(const GameObjectHandle<CGUIWidget>& nativeObject);
		~ScriptGUIWidget();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetPanel(ScriptGUIWidget* self);
		static uint8_t InternalGetDepth(ScriptGUIWidget* self);
		static void InternalSetDepth(ScriptGUIWidget* self, uint8_t depth);
		static bool InternalInBounds(ScriptGUIWidget* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position);
		static void InternalGetBounds(ScriptGUIWidget* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
	};
}
