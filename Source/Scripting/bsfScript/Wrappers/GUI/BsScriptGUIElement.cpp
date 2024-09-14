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

	*bounds = self->GetNativeObject()->GetBoundsRelativeTo();
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

	*bounds = self->GetNativeObject()->GetBoundsRelativeTo();
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

ScriptGUIInteractableWrapperBase::OnFocusChangedThunkDef ScriptGUIInteractableWrapperBase::OnFocusGainedThunk;
ScriptGUIInteractableWrapperBase::OnFocusChangedThunkDef ScriptGUIInteractableWrapperBase::OnFocusLostThunk;

void ScriptGUIInteractableWrapperBase::RegisterEvents()
{
	GetNativeObject()->OnFocusChanged.Connect(std::bind(&ScriptGUIInteractableWrapperBase::OnFocusChanged, this, std::placeholders::_1));
	ScriptGUIElementWrapper::RegisterEvents();
}

void ScriptGUIInteractableWrapperBase::OnFocusChanged(bool focus)
{
	MonoObject* const scriptObject = GetScriptObject();

	if(focus)
		MonoUtil::InvokeThunk(OnFocusGainedThunk, scriptObject);
	else
		MonoUtil::InvokeThunk(OnFocusLostThunk, scriptObject);
}

ScriptGUIInteractable::ScriptGUIInteractable()
	: TScriptGUIElementWrapper(nullptr)
{
}

void ScriptGUIInteractable::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIInteractable::InternalSetFocus);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBlocking", (void*)&ScriptGUIInteractable::InternalGetBlocking);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBlocking", (void*)&ScriptGUIInteractable::InternalSetBlocking);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAcceptsKeyFocus", (void*)&ScriptGUIInteractable::InternalGetAcceptsKeyFocus);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAcceptsKeyFocus", (void*)&ScriptGUIInteractable::InternalSetAcceptsKeyFocus);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIInteractable::InternalSetContextMenu);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptGUIInteractable::InternalGetStyle);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStyle", (void*)&ScriptGUIInteractable::InternalSetStyle);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIInteractable::InternalSetTint);

	OnFocusGainedThunk = (OnFocusChangedThunkDef)sInteropMetaData.ScriptClass->GetMethod("Internal_OnFocusGained", 0)->GetThunk();
	OnFocusLostThunk = (OnFocusChangedThunkDef)sInteropMetaData.ScriptClass->GetMethod("Internal_OnFocusLost", 0)->GetThunk();
}

void ScriptGUIInteractable::InternalSetFocus(ScriptGUIInteractableWrapperBase* self, bool focus)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetFocus(focus, true);
}

bool ScriptGUIInteractable::InternalGetBlocking(ScriptGUIInteractableWrapperBase* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return !self->GetNativeObject()->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
}

void ScriptGUIInteractable::InternalSetBlocking(ScriptGUIInteractableWrapperBase* self, bool blocking)
{
	if(!self->IsNativeObjectValid())
		return;

	GUIInteractable* const nativeObject = self->GetNativeObject();

	GUIElementOptions options = nativeObject->GetOptionFlags();
	if(blocking)
		options.Unset(GUIElementOption::ClickThrough);
	else
		options.Set(GUIElementOption::ClickThrough);

	nativeObject->SetOptionFlags(options);
}

bool ScriptGUIInteractable::InternalGetAcceptsKeyFocus(ScriptGUIInteractableWrapperBase* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
}

void ScriptGUIInteractable::InternalSetAcceptsKeyFocus(ScriptGUIInteractableWrapperBase* self, bool accepts)
{
	if(!self->IsNativeObjectValid())
		return;

	GUIInteractable* const nativeObject = self->GetNativeObject();

	GUIElementOptions options = nativeObject->GetOptionFlags();
	if(accepts)
		options.Set(GUIElementOption::AcceptsKeyFocus);
	else
		options.Unset(GUIElementOption::AcceptsKeyFocus);

	nativeObject->SetOptionFlags(options);
}

void ScriptGUIInteractable::InternalSetTint(ScriptGUIInteractableWrapperBase* self, Color* tint)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetTint(*tint);
}

void ScriptGUIInteractable::InternalSetContextMenu(ScriptGUIInteractableWrapperBase* self, ScriptContextMenu* contextMenu)
{
	if(!self->IsNativeObjectValid())
		return;

	SPtr<GUIContextMenu> nativeContextMenu;
	if(contextMenu != nullptr)
		nativeContextMenu = contextMenu->GetInternal();

	self->GetNativeObject()->SetContextMenu(nativeContextMenu);
}

MonoString* ScriptGUIInteractable::InternalGetStyle(ScriptGUIInteractableWrapperBase* self)
{
	if(!self->IsNativeObjectValid())
		return MonoUtil::StringToMono(StringUtil::kBlank);

	return MonoUtil::StringToMono(self->GetNativeObject()->GetStyleSheetClass());
}

void ScriptGUIInteractable::InternalSetStyle(ScriptGUIInteractableWrapperBase* self, MonoString* style)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetStyleSheetClass(MonoUtil::MonoToString(style));
}
