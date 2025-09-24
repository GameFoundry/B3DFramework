//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIInteractable.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"
#include "../../../Foundation/Engine/GUI/B3DGUICanvas.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/Utility/B3DEnums.h"

namespace b3d { class GUICanvas; }
namespace b3d { struct __TVector2_TUnitValue_int32_t__LogicalPixel__Interop; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d { struct __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUICanvas : public TScriptGUIElementWrapper<GUICanvas, ScriptGUICanvas, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUICanvas")

		ScriptGUICanvas(GUICanvas* nativeObject);
		~ScriptGUICanvas();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalDrawLine(ScriptGUICanvas* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* a, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* b, Color* color, uint8_t depth);
		static void InternalDrawPolyLine(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth);
		static void InternalDrawImage(ScriptGUICanvas* self, MonoObject* image, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, Color* color, TextureScaleMode scaleMode, uint8_t depth);
		static void InternalDrawTriangleStrip(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth);
		static void InternalDrawTriangleList(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth);
		static void InternalDrawText(ScriptGUICanvas* self, MonoString* text, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position, MonoObject* font, float size, Color* color, uint8_t depth);
		static void InternalClear(ScriptGUICanvas* self);
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
