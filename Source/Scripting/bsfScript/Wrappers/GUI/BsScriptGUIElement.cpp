//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
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
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVisible", (void*)&ScriptGUIElement::InternalSetVisible);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVisible", (void*)&ScriptGUIElement::InternalGetVisible);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::InternalSetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::InternalGetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::InternalSetDisabled);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::InternalGetDisabled);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptGUIElement::InternalGetBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptGUIElement::InternalSetBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVisibleBounds", (void*)&ScriptGUIElement::InternalGetVisibleBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScreenBounds", (void*)&ScriptGUIElement::InternalGetScreenBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateBoundsRelativeTo", (void*)&ScriptGUIElement::InternalCalculateBoundsRelativeTo);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::InternalSetPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::InternalSetWidth);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::InternalSetHeight);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::InternalSetFlexibleWidth);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::InternalSetFlexibleHeight);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResetDimensions", (void*)&ScriptGUIElement::InternalResetDimensions);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::InternalGetParent);
}

void ScriptGUIElement::InternalDestroy(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->Destroy();
}

void ScriptGUIElement::InternalSetVisible(ScriptGUIElementWrapper* self, bool visible)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetVisible(visible);
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

bool ScriptGUIElement::InternalGetVisible(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsVisible();
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

void ScriptGUIElement::InternalGetBounds(ScriptGUIElementWrapper* self, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = self->GetNativeObject()->CalculateBoundsRelativeTo();
}

void ScriptGUIElement::InternalSetBounds(ScriptGUIElementWrapper* self, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetPosition(bounds->X, bounds->Y);
	self->GetNativeObject()->SetWidth(bounds->Width);
	self->GetNativeObject()->SetHeight(bounds->Height);
}

void ScriptGUIElement::InternalGetVisibleBounds(ScriptGUIElementWrapper* self, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = self->GetNativeObject()->CalculateBoundsRelativeTo();
}

void ScriptGUIElement::InternalGetScreenBounds(ScriptGUIElementWrapper* self, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = self->GetNativeObject()->GetScreenBounds();
}

void ScriptGUIElement::InternalCalculateBoundsRelativeTo(ScriptGUIElementWrapper* self, ScriptGUIElementWrapper* relativeTo, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
	{
		*bounds = Rect2I();
		return;
	}

	GUIElement* relativeToElement = nullptr;
	if(relativeTo != nullptr)
	{
		if(!relativeTo->IsNativeObjectValid())
		{
			*bounds = Rect2I();
			return;
		}

		relativeToElement = relativeTo->GetNativeObject();
	}

	*bounds = self->GetNativeObject()->CalculateBoundsRelativeTo(relativeToElement);
}

void ScriptGUIElement::InternalSetPosition(ScriptGUIElementWrapper* self, i32 x, i32 y)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetPosition(x, y);
}

void ScriptGUIElement::InternalSetWidth(ScriptGUIElementWrapper* self, u32 width)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetWidth(width);
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

void ScriptGUIElement::InternalSetFlexibleHeight(ScriptGUIElementWrapper* self, u32 minHeight, u32 maxHeight)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetFlexibleHeight(minHeight, maxHeight);
}

void ScriptGUIElement::InternalResetDimensions(ScriptGUIElementWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->ResetDimensions();
}
