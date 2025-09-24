//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIInteractable.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUILayout.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "../../../Foundation/Engine/GUI/B3DGUIScrollArea.h"
#include "../../../Foundation/Engine/GUI/B3DGUIScrollArea.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"

namespace b3d { class GUIScrollArea; }
namespace b3d { struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollArea : public TScriptGUIElementWrapper<GUIScrollArea, ScriptGUIScrollArea, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIScrollArea")

		ScriptGUIScrollArea(GUIScrollArea* nativeObject);
		~ScriptGUIScrollArea();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetLayout(ScriptGUIScrollArea* self);
		static void InternalScrollUp(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels);
		static void InternalScrollDown(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels);
		static void InternalScrollLeft(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels);
		static void InternalScrollRight(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels);
		static void InternalScrollUp0(ScriptGUIScrollArea* self, float percent);
		static void InternalScrollDown0(ScriptGUIScrollArea* self, float percent);
		static void InternalScrollLeft0(ScriptGUIScrollArea* self, float percent);
		static void InternalScrollRight0(ScriptGUIScrollArea* self, float percent);
		static void InternalScrollToVertical(ScriptGUIScrollArea* self, float pct);
		static void InternalScrollToHorizontal(ScriptGUIScrollArea* self, float pct);
		static float InternalGetVerticalScroll(ScriptGUIScrollArea* self);
		static float InternalGetHorizontalScroll(ScriptGUIScrollArea* self);
		static void InternalGetContentBounds(ScriptGUIScrollArea* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output);
		static void InternalSetEnableCulling(ScriptGUIScrollArea* self, bool enable);
		static void InternalGetScrollBarSize(ScriptGUIScrollArea* self, TUnitValue<int32_t, LogicalPixel>* __output);
		static void InternalCreate(MonoObject* scriptObject, GUIScrollAreaContent* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, GUIScrollAreaContent* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
