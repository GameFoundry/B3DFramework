//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIElement.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"

namespace b3d { struct __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop; }
namespace b3d { class GUIElement; }
namespace b3d { struct __TSize2_TUnitValue_int32_t__LogicalPixel__Interop; }
namespace b3d { struct __TVector2_TUnitValue_int32_t__LogicalPixel__Interop; }
namespace b3d { struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d { struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElement : public TScriptGUIElementWrapper<GUIElement, ScriptGUIElement>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIElement")

		ScriptGUIElement(GUIElement* nativeObject);
		~ScriptGUIElement();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetPosition(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* x, TUnitValue<int32_t, LogicalPixel>* y);
		static void InternalSetPosition0(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position);
		static void InternalSetWidth(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* width);
		static void InternalSetFlexibleWidth(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* minWidth, TUnitValue<int32_t, LogicalPixel>* maxWidth);
		static void InternalSetHeight(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* height);
		static void InternalSetFlexibleHeight(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* minHeight, TUnitValue<int32_t, LogicalPixel>* maxHeight);
		static void InternalSetSize(ScriptGUIElement* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size);
		static void InternalResetSizeConstraints(ScriptGUIElement* self);
		static void InternalSetHidden(ScriptGUIElement* self, bool hidden);
		static void InternalSetActive(ScriptGUIElement* self, bool active);
		static void InternalSetDisabled(ScriptGUIElement* self, bool disabled);
		static void InternalCalculateSizeInLayout(ScriptGUIElement* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* __output);
		static void InternalCalculatePositionRelativeTo(ScriptGUIElement* self, MonoObject* relativeTo, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* __output);
		static void InternalCalculateAbsoluteBoundsRelativeTo(ScriptGUIElement* self, MonoObject* relativeTo, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalCalculateAbsoluteBounds(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalCalculateScreenBounds(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalWidgetToElementSpace(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* point, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* __output);
		static void InternalElementToWidgetSpace(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* point, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalWidgetToElementSpace0(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* __output);
		static void InternalElementToWidgetSpace0(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalDestroy(ScriptGUIElement* self);
		static void InternalUpdateLayoutIfDirty(ScriptGUIElement* self);
		static MonoObject* InternalGetParent(ScriptGUIElement* self);
		static bool InternalIsHidden(ScriptGUIElement* self);
		static bool InternalIsActive(ScriptGUIElement* self);
		static bool InternalIsDisabled(ScriptGUIElement* self);
	};
}
