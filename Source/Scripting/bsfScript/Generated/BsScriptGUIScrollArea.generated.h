//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIInteractable.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILayout.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIScrollArea.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIScrollArea.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { struct __GUIOptionInterop; }
namespace bs { class GUIScrollArea; }
namespace bs { struct __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop; }
namespace bs
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
