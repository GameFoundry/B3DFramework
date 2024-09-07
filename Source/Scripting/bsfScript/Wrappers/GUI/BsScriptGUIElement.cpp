//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptMeta.h"
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
ScriptGUIElementBase::ScriptGUIElementBase(MonoObject* instance)
	: ScriptObjectBase(instance), mIsDestroyed(false), mElement(nullptr), mParent(nullptr)
{
	mGCHandle = MonoUtil::NewWeakGcHandle(instance);
}

void ScriptGUIElementBase::Initialize(GUIElement* element)
{
	mElement = element;
}

MonoObject* ScriptGUIElementBase::GetManagedInstance() const
{
	return MonoUtil::GetObjectFromGcHandle(mGCHandle);
}

void ScriptGUIElementBase::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
{
	Destroy();

	ScriptObjectBase::OnManagedInstanceDeletedInternal(assemblyRefresh);
}

void ScriptGUIElementBase::ClearManagedInstanceInternal()
{
	// Need to call destroy here because we need to release any GC handles before the domain is unloaded

	Destroy();
}

ScriptGUIInteractableBase::ScriptGUIInteractableBase(MonoObject* instance)
	: ScriptGUIElementBase(instance)
{ }

void ScriptGUIInteractableBase::RegisterEvents(GUIElement* element)
{
	GUIInteractable* guiElem = static_cast<GUIInteractable*>(element);
	guiElem->OnFocusChanged.Connect(std::bind(&ScriptGUIInteractableBase::OnFocusChanged, this, _1));
	
	ScriptGUIElementBase::RegisterEvents(element);
}

void ScriptGUIInteractableBase::OnFocusChanged(ScriptGUIElementBase* thisPtr, bool focus)
{
	MonoObject* instance = thisPtr->GetManagedInstance();

	if(focus)
		MonoUtil::InvokeThunk(ScriptGUIInteractableBase::onFocusGainedThunk, instance);
	else
		MonoUtil::InvokeThunk(ScriptGUIInteractableBase::onFocusLostThunk, instance);
}

void ScriptGUIInteractableBase::Destroy()
{
	if(!mIsDestroyed)
	{
		if(mParent != nullptr)
			mParent->RemoveChild(this);

		mElement->Destroy();
		mElement = nullptr;

		mIsDestroyed = true;
	}
}

ScriptGUIElement::ScriptGUIElement(MonoObject* instance)
	: TScriptGUIElementBase(instance, nullptr)
{
}

void ScriptGUIElement::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::InternalDestroy);
	metaData.ScriptClass->AddInternalCall("Internal_SetVisible", (void*)&ScriptGUIElement::InternalSetVisible);
	metaData.ScriptClass->AddInternalCall("Internal_GetVisible", (void*)&ScriptGUIElement::InternalGetVisible);
	metaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::InternalSetActive);
	metaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::InternalGetActive);
	metaData.ScriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::InternalSetDisabled);
	metaData.ScriptClass->AddInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::InternalGetDisabled);
	metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptGUIElement::InternalGetBounds);
	metaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptGUIElement::InternalSetBounds);
	metaData.ScriptClass->AddInternalCall("Internal_GetVisibleBounds", (void*)&ScriptGUIElement::InternalGetVisibleBounds);
	metaData.ScriptClass->AddInternalCall("Internal_GetScreenBounds", (void*)&ScriptGUIElement::InternalGetScreenBounds);
	metaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::InternalSetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::InternalSetWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::InternalSetHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::InternalSetFlexibleWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::InternalSetFlexibleHeight);
	metaData.ScriptClass->AddInternalCall("Internal_ResetDimensions", (void*)&ScriptGUIElement::InternalResetDimensions);
	metaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::InternalGetParent);
}

void ScriptGUIElement::InternalDestroy(ScriptGUIElementBase* nativeInstance)
{
	nativeInstance->Destroy();
}

void ScriptGUIElement::InternalSetVisible(ScriptGUIElementBase* nativeInstance, bool visible)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetVisible(visible);
}

void ScriptGUIElement::InternalSetActive(ScriptGUIElementBase* nativeInstance, bool enabled)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetActive(enabled);
}

void ScriptGUIElement::InternalSetDisabled(ScriptGUIElementBase* nativeInstance, bool disabled)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetDisabled(disabled);
}

bool ScriptGUIElement::InternalGetVisible(ScriptGUIElementBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElement* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsVisible();
}

bool ScriptGUIElement::InternalGetActive(ScriptGUIElementBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElement* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsActive();
}

bool ScriptGUIElement::InternalGetDisabled(ScriptGUIElementBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElement* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsDisabled();
}

MonoObject* ScriptGUIElement::InternalGetParent(ScriptGUIElementBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return nullptr;

	if(nativeInstance->GetParent() != nullptr)
		return nativeInstance->GetParent()->GetManagedInstance();

	return nullptr;
}

void ScriptGUIElement::InternalGetBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetBoundsRelativeTo();
}

void ScriptGUIElement::InternalSetBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetPosition(bounds->X, bounds->Y);
	nativeInstance->GetGuiElement()->SetWidth(bounds->Width);
	nativeInstance->GetGuiElement()->SetHeight(bounds->Height);
}

void ScriptGUIElement::InternalGetVisibleBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetBoundsRelativeTo();
}

void ScriptGUIElement::InternalGetScreenBounds(ScriptGUIElementBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetScreenBounds();
}

void ScriptGUIElement::InternalSetPosition(ScriptGUIElementBase* nativeInstance, i32 x, i32 y)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetPosition(x, y);
}

void ScriptGUIElement::InternalSetWidth(ScriptGUIElementBase* nativeInstance, u32 width)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetWidth(width);
}

void ScriptGUIElement::InternalSetFlexibleWidth(ScriptGUIElementBase* nativeInstance, u32 minWidth, u32 maxWidth)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetFlexibleWidth(minWidth, maxWidth);
}

void ScriptGUIElement::InternalSetHeight(ScriptGUIElementBase* nativeInstance, u32 height)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetHeight(height);
}

void ScriptGUIElement::InternalSetFlexibleHeight(ScriptGUIElementBase* nativeInstance, u32 minHeight, u32 maxHeight)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetFlexibleHeight(minHeight, maxHeight);
}

void ScriptGUIElement::InternalResetDimensions(ScriptGUIElementBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->ResetDimensions();
}

ScriptGUIInteractableBase::OnFocusChangedThunkDef ScriptGUIInteractableBase::onFocusGainedThunk;
ScriptGUIInteractableBase::OnFocusChangedThunkDef ScriptGUIInteractableBase::onFocusLostThunk;

ScriptGUIInteractable::ScriptGUIInteractable(MonoObject* instance)
	: TScriptGUIInteractable(instance, nullptr)
{
}

void ScriptGUIInteractable::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIInteractable::InternalSetFocus);
	metaData.ScriptClass->AddInternalCall("Internal_GetBlocking", (void*)&ScriptGUIInteractable::InternalGetBlocking);
	metaData.ScriptClass->AddInternalCall("Internal_SetBlocking", (void*)&ScriptGUIInteractable::InternalSetBlocking);
	metaData.ScriptClass->AddInternalCall("Internal_GetAcceptsKeyFocus", (void*)&ScriptGUIInteractable::InternalGetAcceptsKeyFocus);
	metaData.ScriptClass->AddInternalCall("Internal_SetAcceptsKeyFocus", (void*)&ScriptGUIInteractable::InternalSetAcceptsKeyFocus);
	metaData.ScriptClass->AddInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIInteractable::InternalSetContextMenu);
	metaData.ScriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptGUIInteractable::InternalGetStyle);
	metaData.ScriptClass->AddInternalCall("Internal_SetStyle", (void*)&ScriptGUIInteractable::InternalSetStyle);
	metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIInteractable::InternalSetTint);

	onFocusGainedThunk = (OnFocusChangedThunkDef)metaData.ScriptClass->GetMethod("Internal_OnFocusGained", 0)->GetThunk();
	onFocusLostThunk = (OnFocusChangedThunkDef)metaData.ScriptClass->GetMethod("Internal_OnFocusLost", 0)->GetThunk();
}

void ScriptGUIInteractable::InternalSetFocus(ScriptGUIInteractableBase* nativeInstance, bool focus)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();
	guiElem->SetFocus(focus, true);
}

bool ScriptGUIInteractable::InternalGetBlocking(ScriptGUIInteractableBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();
	return !guiElem->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
}

void ScriptGUIInteractable::InternalSetBlocking(ScriptGUIInteractableBase* nativeInstance, bool blocking)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();

	GUIElementOptions options = guiElem->GetOptionFlags();
	if(blocking)
		options.Unset(GUIElementOption::ClickThrough);
	else
		options.Set(GUIElementOption::ClickThrough);

	guiElem->SetOptionFlags(options);
}

bool ScriptGUIInteractable::InternalGetAcceptsKeyFocus(ScriptGUIInteractableBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();
	return guiElem->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
}

void ScriptGUIInteractable::InternalSetAcceptsKeyFocus(ScriptGUIInteractableBase* nativeInstance, bool accepts)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();

	GUIElementOptions options = guiElem->GetOptionFlags();
	if(accepts)
		options.Set(GUIElementOption::AcceptsKeyFocus);
	else
		options.Unset(GUIElementOption::AcceptsKeyFocus);

	guiElem->SetOptionFlags(options);
}

void ScriptGUIInteractable::InternalSetTint(ScriptGUIInteractableBase* nativeInstance, Color* tint)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElement = nativeInstance->GetGuiInteractable();
	guiElement->SetTint(*tint);
}

void ScriptGUIInteractable::InternalSetContextMenu(ScriptGUIInteractableBase* nativeInstance, ScriptContextMenu* contextMenu)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();

	SPtr<GUIContextMenu> nativeContextMenu;
	if(contextMenu != nullptr)
		nativeContextMenu = contextMenu->GetInternal();

	guiElem->SetContextMenu(nativeContextMenu);
}

MonoString* ScriptGUIInteractable::InternalGetStyle(ScriptGUIInteractableBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return MonoUtil::StringToMono(StringUtil::kBlank);

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();
	return MonoUtil::StringToMono(guiElem->GetStyleSheetClass());
}

void ScriptGUIInteractable::InternalSetStyle(ScriptGUIInteractableBase* nativeInstance, MonoString* style)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIInteractable* guiElem = nativeInstance->GetGuiInteractable();
	guiElem->SetStyleSheetClass(MonoUtil::MonoToString(style));
}
