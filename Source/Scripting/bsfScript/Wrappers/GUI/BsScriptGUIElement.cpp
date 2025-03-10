//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptTArea2.generated.h"
#include "BsScriptTSize2.generated.h"
#include "BsScriptTVector2.generated.h"
#include "GUI/BsGUIInteractable.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "Wrappers/BsScriptContextMenu.h"
#include "GUI/BsGUIInteractable.h"

using namespace std::placeholders;

using namespace bs;

ScriptGUIElement::ScriptGUIElement()
	: TScriptGUIElementWrapper(nullptr)
{
}

void ScriptGUIElement::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::InternalDestroy);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHidden", (void*)&ScriptGUIElement::InternalSetHidden);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHidden", (void*)&ScriptGUIElement::InternalGetHidden);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::InternalSetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::InternalGetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::InternalSetDisabled);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::InternalGetDisabled);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateAbsoluteBounds", (void*)&ScriptGUIElement::InternalCalculateAbsoluteBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayoutCalculatedSize", (void*)&ScriptGUIElement::InternalGetLayoutCalculatedSize);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScreenBounds", (void*)&ScriptGUIElement::InternalGetScreenBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculatePositionRelativeTo", (void*)&ScriptGUIElement::InternalCalculatePositionRelativeTo);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateAbsoluteBoundsRelativeTo", (void*)&ScriptGUIElement::InternalCalculateAbsoluteBoundsRelativeTo);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::InternalSetPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition2", (void*)&ScriptGUIElement::InternalSetPosition2);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::InternalSetWidth);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWidth2", (void*)&ScriptGUIElement::InternalSetWidth2);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::InternalSetHeight);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHeight2", (void*)&ScriptGUIElement::InternalSetHeight2);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptGUIElement::InternalSetSize);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize2", (void*)&ScriptGUIElement::InternalSetSize2);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::InternalSetFlexibleWidth);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::InternalSetFlexibleHeight);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResetSizeConstraints", (void*)&ScriptGUIElement::InternalResetSizeConstraints);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::InternalGetParent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_WidgetToElementSpace0", (void*)&ScriptGUIElement::InternalWidgetToElementSpace0);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_ElementToWidgetSpace0", (void*)&ScriptGUIElement::InternalElementToWidgetSpace0);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_WidgetToElementSpace1", (void*)&ScriptGUIElement::InternalWidgetToElementSpace1);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_ElementToWidgetSpace1", (void*)&ScriptGUIElement::InternalElementToWidgetSpace1);
}

void ScriptGUIElement::InternalDestroy(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->Destroy();
}

void ScriptGUIElement::InternalSetHidden(ScriptGUIElementWrapper* self, bool hidden)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetHidden(hidden);
}

void ScriptGUIElement::InternalSetActive(ScriptGUIElementWrapper* self, bool enabled)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetActive(enabled);
}

void ScriptGUIElement::InternalSetDisabled(ScriptGUIElementWrapper* self, bool disabled)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetDisabled(disabled);
}

bool ScriptGUIElement::InternalGetHidden(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsHidden();
}

bool ScriptGUIElement::InternalGetActive(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsActive();
}

bool ScriptGUIElement::InternalGetDisabled(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsDisabled();
}

MonoObject* ScriptGUIElement::InternalGetParent(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	GUIElement* const parent = self->GetNativeObject()->GetParent();
	if(parent == nullptr)
		return nullptr;

	// Note: This should be calling GetOrCreateScriptObject, but for the time being we don't support ad-hoc script object creation for
	// GUI elements. Instead script can only access script objects it has itself created. We can easily change this in the future, by
	// adding RTTI IDs to all GUI elements, and then registering them in a lookup similar to other reflectable types.
	ScriptGUIElementWrapper* const parentScriptObjectWrapper = static_cast<ScriptGUIElementWrapper*>(parent->GetScriptObjectWrapper());
	if(parentScriptObjectWrapper == nullptr)
		return nullptr;

	return parentScriptObjectWrapper->GetScriptObject();
}

void ScriptGUIElement::InternalCalculateAbsoluteBounds(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop();
		return;
	}

	*bounds = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(self->GetNativeObject()->CalculateAbsoluteBoundsRelativeTo());
}

void ScriptGUIElement::InternalGetLayoutCalculatedSize(ScriptGUIElementWrapper* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size)
{
	if(!self->IsNativeObjectValid())
	{
		*size = __TSize2_TUnitValue_int32_t__LogicalPixel__Interop();
		return;
	}

	*size = ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::ToInterop(self->GetNativeObject()->CalculateSizeInLayout());
}

void ScriptGUIElement::InternalGetScreenBounds(ScriptGUIElementWrapper* self, Area2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Area2I();
		return;
	}

	*bounds = self->GetNativeObject()->CalculateScreenBounds();
}

void ScriptGUIElement::InternalCalculatePositionRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position)
{
	if(!self->IsNativeObjectValid())
	{
		*position = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(GUILogicalPoint::kZero);
		return;
	}

	GUIElement* relativeToElement = nullptr;
	if(relativeTo != nullptr)
	{
		if(!relativeTo->IsNativeObjectValid())
		{
			*position = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(GUILogicalPoint::kZero);
			return;
		}

		relativeToElement = relativeTo->GetNativeObject();
	}

	*position = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(self->GetNativeObject()->CalculatePositionRelativeTo(relativeToElement));
}

void ScriptGUIElement::InternalCalculateAbsoluteBoundsRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, Area2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Area2I();
		return;
	}

	GUIElement* relativeToElement = nullptr;
	if(relativeTo != nullptr)
	{
		if(!relativeTo->IsNativeObjectValid())
		{
			*bounds = Area2I();
			return;
		}

		relativeToElement = relativeTo->GetNativeObject();
	}

	*bounds = self->GetNativeObject()->CalculateAbsoluteBoundsRelativeTo(relativeToElement).To<i32, u32>();
}

void ScriptGUIElement::InternalSetPosition(ScriptGUIElementWrapper* self, i32 x, i32 y)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetPosition(x, y);
}

void ScriptGUIElement::InternalSetPosition2(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetPosition(ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*position));
}

void ScriptGUIElement::InternalSetWidth(ScriptGUIElementWrapper* self, u32 width)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetWidth(width);
}

void ScriptGUIElement::InternalSetWidth2(ScriptGUIElementWrapper* self, GUILogicalUnit* width)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetWidth(*width);
}

void ScriptGUIElement::InternalSetFlexibleWidth(ScriptGUIElementWrapper* self, u32 minWidth, u32 maxWidth)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetFlexibleWidth(minWidth, maxWidth);
}

void ScriptGUIElement::InternalSetHeight(ScriptGUIElementWrapper* self, u32 height)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetHeight(height);
}

void ScriptGUIElement::InternalSetHeight2(ScriptGUIElementWrapper* self, GUILogicalUnit* height)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetHeight(*height);
}

void ScriptGUIElement::InternalSetSize(ScriptGUIElementWrapper* self, Size2UI* size)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetSize(size->Width, size->Height);
}

void ScriptGUIElement::InternalSetSize2(ScriptGUIElementWrapper* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetSize(ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*size));
}

void ScriptGUIElement::InternalSetFlexibleHeight(ScriptGUIElementWrapper* self, u32 minHeight, u32 maxHeight)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetFlexibleHeight(minHeight, maxHeight);
}

void ScriptGUIElement::InternalResetSizeConstraints(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->ResetSizeConstraints();
}

void ScriptGUIElement::InternalWidgetToElementSpace0(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* outPosition)
{
	if(!self->IsNativeObjectValid())
	{
		*outPosition = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(GUILogicalPoint::kZero);
		return;
	}

	const GUILogicalPoint output = self->GetNativeObject()->WidgetToElementSpace(ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(*position));
	*outPosition = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(output);
}

void ScriptGUIElement::InternalElementToWidgetSpace0(ScriptGUIElementWrapper* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* outPosition)
{
	if(!self->IsNativeObjectValid())
	{
		*outPosition = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(GUIPhysicalPoint::kZero);
		return;
	}

	const GUIPhysicalPoint output = self->GetNativeObject()->ElementToWidgetSpace(ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*position));
	*outPosition = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(output);
}

void ScriptGUIElement::InternalWidgetToElementSpace1(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* outArea)
{
	if(!self->IsNativeObjectValid())
	{
		*outArea = ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::ToInterop(GUILogicalArea::kEmpty);
		return;
	}

	const GUILogicalArea output = self->GetNativeObject()->WidgetToElementSpace(ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::FromInterop(*area));
	*outArea = ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::ToInterop(output);
}

void ScriptGUIElement::InternalElementToWidgetSpace1(ScriptGUIElementWrapper* self, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* outArea)
{
	if(!self->IsNativeObjectValid())
	{
		*outArea = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(GUIPhysicalArea::kEmpty);
		return;
	}

	const GUIPhysicalArea output = self->GetNativeObject()->ElementToWidgetSpace(ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::FromInterop(*area));
	*outArea = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(output);
}
