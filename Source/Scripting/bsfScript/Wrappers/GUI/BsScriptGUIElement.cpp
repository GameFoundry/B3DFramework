//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIElement.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "Wrappers/BsScriptContextMenu.h"
#include "GUI/BsGUIElement.h"

using namespace std::placeholders;

using namespace bs;
ScriptGUIElementBaseTBase::ScriptGUIElementBaseTBase(MonoObject* instance)
	: ScriptObjectBase(instance), mIsDestroyed(false), mElement(nullptr), mParent(nullptr)
{
	mGCHandle = MonoUtil::NewWeakGcHandle(instance);
}

void ScriptGUIElementBaseTBase::Initialize(GUIElementBase* element)
{
	mElement = element;

	if(mElement != nullptr && mElement->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(element);
		guiElem->OnFocusChanged.Connect(std::bind(&ScriptGUIElementBaseTBase::OnFocusChanged, this, _1));
	}
}

void ScriptGUIElementBaseTBase::OnFocusChanged(ScriptGUIElementBaseTBase* thisPtr, bool focus)
{
	MonoObject* instance = MonoUtil::GetObjectFromGcHandle(thisPtr->mGCHandle);

	if(focus)
		MonoUtil::InvokeThunk(ScriptGUIElement::onFocusGainedThunk, instance);
	else
		MonoUtil::InvokeThunk(ScriptGUIElement::onFocusLostThunk, instance);
}

MonoObject* ScriptGUIElementBaseTBase::GetManagedInstance() const
{
	return MonoUtil::GetObjectFromGcHandle(mGCHandle);
}

void ScriptGUIElementBaseTBase::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
{
	Destroy();

	ScriptObjectBase::OnManagedInstanceDeletedInternal(assemblyRefresh);
}

void ScriptGUIElementBaseTBase::ClearManagedInstanceInternal()
{
	// Need to call destroy here because we need to release any GC handles before the domain is unloaded

	Destroy();
}

ScriptGUIElementTBase::ScriptGUIElementTBase(MonoObject* instance)
	: ScriptGUIElementBaseTBase(instance)
{
}

void ScriptGUIElementTBase::Destroy()
{
	if(!mIsDestroyed)
	{
		if(mParent != nullptr)
			mParent->RemoveChild(this);

		if(mElement->GetTypeInternal() == GUIElementBase::Type::Element)
		{
			GUIElement::Destroy((GUIElement*)mElement);
			mElement = nullptr;

			mIsDestroyed = true;
		}
	}
}

ScriptGUIElement::OnFocusChangedThunkDef ScriptGUIElement::onFocusGainedThunk;
ScriptGUIElement::OnFocusChangedThunkDef ScriptGUIElement::onFocusLostThunk;

ScriptGUIElement::ScriptGUIElement(MonoObject* instance)
	: ScriptObject(instance)
{
}

void ScriptGUIElement::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::InternalDestroy);
	metaData.ScriptClass->AddInternalCall("Internal_SetVisible", (void*)&ScriptGUIElement::InternalSetVisible);
	metaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::InternalSetActive);
	metaData.ScriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::InternalSetDisabled);
	metaData.ScriptClass->AddInternalCall("Internal_GetVisible", (void*)&ScriptGUIElement::InternalGetVisible);
	metaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::InternalGetActive);
	metaData.ScriptClass->AddInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::InternalGetDisabled);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::InternalSetFocus);
	metaData.ScriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::InternalSetFocus);
	metaData.ScriptClass->AddInternalCall("Internal_GetBlocking", (void*)&ScriptGUIElement::InternalGetBlocking);
	metaData.ScriptClass->AddInternalCall("Internal_SetBlocking", (void*)&ScriptGUIElement::InternalSetBlocking);
	metaData.ScriptClass->AddInternalCall("Internal_GetAcceptsKeyFocus", (void*)&ScriptGUIElement::InternalGetAcceptsKeyFocus);
	metaData.ScriptClass->AddInternalCall("Internal_SetAcceptsKeyFocus", (void*)&ScriptGUIElement::InternalSetAcceptsKeyFocus);
	metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptGUIElement::InternalGetBounds);
	metaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptGUIElement::InternalSetBounds);
	metaData.ScriptClass->AddInternalCall("Internal_GetVisibleBounds", (void*)&ScriptGUIElement::InternalGetVisibleBounds);
	metaData.ScriptClass->AddInternalCall("Internal_GetScreenBounds", (void*)&ScriptGUIElement::InternalGetScreenBounds);
	metaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::InternalSetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::InternalSetWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::InternalSetFlexibleWidth);
	metaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::InternalSetHeight);
	metaData.ScriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::InternalSetFlexibleHeight);
	metaData.ScriptClass->AddInternalCall("Internal_ResetDimensions", (void*)&ScriptGUIElement::InternalResetDimensions);
	metaData.ScriptClass->AddInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIElement::InternalSetContextMenu);
	metaData.ScriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptGUIElement::InternalGetStyle);
	metaData.ScriptClass->AddInternalCall("Internal_SetStyle", (void*)&ScriptGUIElement::InternalSetStyle);
	metaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::InternalGetParent);

	onFocusGainedThunk = (OnFocusChangedThunkDef)metaData.ScriptClass->GetMethod("Internal_OnFocusGained", 0)->GetThunk();
	onFocusLostThunk = (OnFocusChangedThunkDef)metaData.ScriptClass->GetMethod("Internal_OnFocusLost", 0)->GetThunk();
}

void ScriptGUIElement::InternalDestroy(ScriptGUIElementBaseTBase* nativeInstance)
{
	nativeInstance->Destroy();
}

void ScriptGUIElement::InternalSetVisible(ScriptGUIElementBaseTBase* nativeInstance, bool visible)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetVisible(visible);
}

void ScriptGUIElement::InternalSetActive(ScriptGUIElementBaseTBase* nativeInstance, bool enabled)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetActive(enabled);
}

void ScriptGUIElement::InternalSetDisabled(ScriptGUIElementBaseTBase* nativeInstance, bool disabled)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetDisabled(disabled);
}

void ScriptGUIElement::InternalSetFocus(ScriptGUIElementBaseTBase* nativeInstance, bool focus)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
		guiElem->SetFocus(focus, true);
	}
}

bool ScriptGUIElement::InternalGetVisible(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsVisible();
}

bool ScriptGUIElement::InternalGetActive(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsActive();
}

bool ScriptGUIElement::InternalGetDisabled(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	return guiElemBase->IsDisabled();
}

bool ScriptGUIElement::InternalGetBlocking(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
		return !guiElem->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

void ScriptGUIElement::InternalSetBlocking(ScriptGUIElementBaseTBase* nativeInstance, bool blocking)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);

		GUIElementOptions options = guiElem->GetOptionFlags();
		if(blocking)
			options.Unset(GUIElementOption::ClickThrough);
		else
			options.Set(GUIElementOption::ClickThrough);

		guiElem->SetOptionFlags(options);
	}
}

bool ScriptGUIElement::InternalGetAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return false;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
		return guiElem->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
	}

	return false;
}

void ScriptGUIElement::InternalSetAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance, bool accepts)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);

		GUIElementOptions options = guiElem->GetOptionFlags();
		if(accepts)
			options.Set(GUIElementOption::AcceptsKeyFocus);
		else
			options.Unset(GUIElementOption::AcceptsKeyFocus);

		guiElem->SetOptionFlags(options);
	}
}

MonoObject* ScriptGUIElement::InternalGetParent(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return nullptr;

	if(nativeInstance->GetParent() != nullptr)
		return nativeInstance->GetParent()->GetManagedInstance();

	return nullptr;
}

void ScriptGUIElement::InternalGetBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetBounds();
}

void ScriptGUIElement::InternalSetBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetPosition(bounds->X, bounds->Y);
	nativeInstance->GetGuiElement()->SetWidth(bounds->Width);
	nativeInstance->GetGuiElement()->SetHeight(bounds->Height);
}

void ScriptGUIElement::InternalGetVisibleBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetVisibleBounds();
}

void ScriptGUIElement::InternalGetScreenBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
{
	if(nativeInstance->IsDestroyed())
	{
		*bounds = Rect2I();
		return;
	}

	*bounds = nativeInstance->GetGuiElement()->GetScreenBounds();
}

void ScriptGUIElement::InternalSetPosition(ScriptGUIElementBaseTBase* nativeInstance, i32 x, i32 y)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetPosition(x, y);
}

void ScriptGUIElement::InternalSetWidth(ScriptGUIElementBaseTBase* nativeInstance, u32 width)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetWidth(width);
}

void ScriptGUIElement::InternalSetFlexibleWidth(ScriptGUIElementBaseTBase* nativeInstance, u32 minWidth, u32 maxWidth)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetFlexibleWidth(minWidth, maxWidth);
}

void ScriptGUIElement::InternalSetHeight(ScriptGUIElementBaseTBase* nativeInstance, u32 height)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetHeight(height);
}

void ScriptGUIElement::InternalSetFlexibleHeight(ScriptGUIElementBaseTBase* nativeInstance, u32 minHeight, u32 maxHeight)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->SetFlexibleHeight(minHeight, maxHeight);
}

void ScriptGUIElement::InternalResetDimensions(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(nativeInstance->IsDestroyed())
		return;

	nativeInstance->GetGuiElement()->ResetDimensions();
}

void ScriptGUIElement::InternalSetContextMenu(ScriptGUIElementBaseTBase* nativeInstance, ScriptContextMenu* contextMenu)
{
	if(nativeInstance->IsDestroyed())
		return;

	GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
	if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
	{
		GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);

		SPtr<GUIContextMenu> nativeContextMenu;
		if(contextMenu != nullptr)
			nativeContextMenu = contextMenu->GetInternal();

		guiElem->SetContextMenu(nativeContextMenu);
	}
}

MonoString* ScriptGUIElement::InternalGetStyle(ScriptGUIElementBaseTBase* nativeInstance)
{
	if(!nativeInstance->IsDestroyed())
	{
		GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
		if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			return MonoUtil::StringToMono(guiElem->GetStyleName());
		}
	}

	return MonoUtil::StringToMono(StringUtil::kBlank);
}

void ScriptGUIElement::InternalSetStyle(ScriptGUIElementBaseTBase* nativeInstance, MonoString* style)
{
	if(!nativeInstance->IsDestroyed())
	{
		GUIElementBase* guiElemBase = nativeInstance->GetGuiElement();
		if(guiElemBase->GetTypeInternal() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			guiElem->SetStyle(MonoUtil::MonoToString(style));
		}
	}
}
