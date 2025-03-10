//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIInteractable.h"

namespace bs
{
	class ScriptGUILayoutWrapperBase;
	struct __TVector2_TUnitValue_int32_t__LogicalPixel__Interop;
	struct __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop;
	struct __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop;
	struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop;
	struct __TSize2_TUnitValue_int32_t__LogicalPixel__Interop;
}

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for GUIElement. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIElement : public TScriptGUIElementWrapper<GUIElement, ScriptGUIElement>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIElement")

		ScriptGUIElement();

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct) { return nullptr; }
	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalDestroy(ScriptGUIElementWrapper* self);
		static void InternalSetHidden(ScriptGUIElementWrapper* self, bool visible);
		static bool InternalGetHidden(ScriptGUIElementWrapper* self);
		static void InternalSetActive(ScriptGUIElementWrapper* self, bool active);
		static bool InternalGetActive(ScriptGUIElementWrapper* self);
		static void InternalSetDisabled(ScriptGUIElementWrapper* self, bool disabled);
		static bool InternalGetDisabled(ScriptGUIElementWrapper* self);
		static MonoObject* InternalGetParent(ScriptGUIElementWrapper* self);
		static void InternalCalculateAbsoluteBounds(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* bounds);
		static void InternalGetLayoutCalculatedSize(ScriptGUIElementWrapper* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size);
		static void InternalGetScreenBounds(ScriptGUIElementWrapper* self, Area2I* bounds);
		static void InternalCalculatePositionRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position);
		static void InternalCalculateAbsoluteBoundsRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, Area2I* bounds);
		static void InternalSetPosition2(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position);
		static void InternalSetWidth2(ScriptGUIElementWrapper* self, GUILogicalUnit* width);
		static void InternalSetHeight2(ScriptGUIElementWrapper* self, GUILogicalUnit* height);
		static void InternalSetSize2(ScriptGUIElementWrapper* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size);
		static void InternalSetFlexibleWidth(ScriptGUIElementWrapper* self, u32 minWidth, u32 maxWidth);
		static void InternalSetFlexibleHeight(ScriptGUIElementWrapper* self, u32 minHeight, u32 maxHeight);
		static void InternalResetSizeConstraints(ScriptGUIElementWrapper* self);
		static void InternalWidgetToElementSpace0(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* outPosition);
		static void InternalElementToWidgetSpace0(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* outPosition);
		static void InternalWidgetToElementSpace1(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* outArea);
		static void InternalElementToWidgetSpace1(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* outArea);
	};

	/** @} */
} // namespace bs
