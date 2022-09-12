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

namespace bs
{
	ScriptGUIElementBaseTBase::ScriptGUIElementBaseTBase(MonoObject* instance)
		:ScriptObjectBase(instance), mIsDestroyed(false), mElement(nullptr), mParent(nullptr)
	{
		mGCHandle = MonoUtil::newWeakGCHandle(instance);
	}

	void ScriptGUIElementBaseTBase::Initialize(GUIElementBase* element)
	{
		mElement = element;

		if (mElement != nullptr && mElement->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(element);
			guiElem->onFocusChanged.Connect(std::bind(&ScriptGUIElementBaseTBase::onFocusChanged, this, _1));
		}
	}

	void ScriptGUIElementBaseTBase::OnFocusChanged(ScriptGUIElementBaseTBase* thisPtr, bool focus)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(thisPtr->mGCHandle);

		if (focus)
			MonoUtil::invokeThunk(ScriptGUIElement::onFocusGainedThunk, instance);
		else
			MonoUtil::invokeThunk(ScriptGUIElement::onFocusLostThunk, instance);
	}

	MonoObject* ScriptGUIElementBaseTBase::getManagedInstance() const
	{
		return MonoUtil::GetObjectFromGCHandle(mGCHandle);
	}

	void ScriptGUIElementBaseTBase::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		destroy();

		ScriptObjectBase::_onManagedInstanceDeleted(assemblyRefresh);
	}

	void ScriptGUIElementBaseTBase::_clearManagedInstance()
	{
		// Need to call destroy here because we need to release any GC handles before the domain is unloaded

		destroy();
	}

	ScriptGUIElementTBase::ScriptGUIElementTBase(MonoObject* instance)
		:ScriptGUIElementBaseTBase(instance)
	{

	}

	void ScriptGUIElementTBase::Destroy()
	{
		if(!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->RemoveChild(this);

			if (mElement->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement::destroy((GUIElement*)mElement);
				mElement = nullptr;

				mIsDestroyed = true;
			}
		}
	}

	ScriptGUIElement::OnFocusChangedThunkDef ScriptGUIElement::onFocusGainedThunk;
	ScriptGUIElement::OnFocusChangedThunkDef ScriptGUIElement::onFocusLostThunk;

	ScriptGUIElement::ScriptGUIElement(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	void ScriptGUIElement::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::internal_destroy);
		metaData.scriptClass->AddInternalCall("Internal_SetVisible", (void*)&ScriptGUIElement::internal_setVisible);
		metaData.scriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::internal_setActive);
		metaData.scriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::internal_setDisabled);
		metaData.scriptClass->AddInternalCall("Internal_GetVisible", (void*)&ScriptGUIElement::internal_getVisible);
		metaData.scriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::internal_getActive);
		metaData.scriptClass->AddInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::internal_getDisabled);
		metaData.scriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::internal_setFocus);
		metaData.scriptClass->AddInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::internal_setFocus);
		metaData.scriptClass->AddInternalCall("Internal_GetBlocking", (void*)&ScriptGUIElement::internal_getBlocking);
		metaData.scriptClass->AddInternalCall("Internal_SetBlocking", (void*)&ScriptGUIElement::internal_setBlocking);
		metaData.scriptClass->AddInternalCall("Internal_GetAcceptsKeyFocus", (void*)&ScriptGUIElement::internal_getAcceptsKeyFocus);
		metaData.scriptClass->AddInternalCall("Internal_SetAcceptsKeyFocus", (void*)&ScriptGUIElement::internal_setAcceptsKeyFocus);
		metaData.scriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptGUIElement::internal_getBounds);
		metaData.scriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptGUIElement::internal_setBounds);
		metaData.scriptClass->AddInternalCall("Internal_GetVisibleBounds", (void*)&ScriptGUIElement::internal_getVisibleBounds);
		metaData.scriptClass->AddInternalCall("Internal_GetScreenBounds", (void*)&ScriptGUIElement::internal_getScreenBounds);
		metaData.scriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::internal_SetPosition);
		metaData.scriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::internal_SetWidth);
		metaData.scriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::internal_SetFlexibleWidth);
		metaData.scriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::internal_SetHeight);
		metaData.scriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::internal_SetFlexibleHeight);
		metaData.scriptClass->AddInternalCall("Internal_ResetDimensions", (void*)&ScriptGUIElement::internal_ResetDimensions);
		metaData.scriptClass->AddInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIElement::internal_SetContextMenu);
		metaData.scriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptGUIElement::internal_GetStyle);
		metaData.scriptClass->AddInternalCall("Internal_SetStyle", (void*)&ScriptGUIElement::internal_SetStyle);
		metaData.scriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::internal_getParent);

		onFocusGainedThunk = (OnFocusChangedThunkDef)metaData.scriptClass->GetMethod("Internal_OnFocusGained", 0)->getThunk();
		onFocusLostThunk = (OnFocusChangedThunkDef)metaData.scriptClass->GetMethod("Internal_OnFocusLost", 0)->getThunk();
	}

	void ScriptGUIElement::internal_destroy(ScriptGUIElementBaseTBase* nativeInstance)
	{
		nativeInstance->Destroy();
	}

	void ScriptGUIElement::internal_setVisible(ScriptGUIElementBaseTBase* nativeInstance, bool visible)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setVisible(visible);
	}

	void ScriptGUIElement::internal_setActive(ScriptGUIElementBaseTBase* nativeInstance, bool enabled)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setActive(enabled);
	}

	void ScriptGUIElement::internal_setDisabled(ScriptGUIElementBaseTBase* nativeInstance, bool disabled)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setDisabled(disabled);
	}

	void ScriptGUIElement::internal_setFocus(ScriptGUIElementBaseTBase* nativeInstance, bool focus)
	{
		if (nativeInstance->IsDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			guiElem->SetFocus(focus, true);
		}		
	}

	bool ScriptGUIElement::internal_getVisible(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		return guiElemBase->_isVisible();
	}

	bool ScriptGUIElement::internal_getActive(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		return guiElemBase->_isActive();
	}

	bool ScriptGUIElement::internal_getDisabled(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		return guiElemBase->_isDisabled();
	}

	bool ScriptGUIElement::internal_getBlocking(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			return !guiElem->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
		}

		return false;
	}

	void ScriptGUIElement::internal_setBlocking(ScriptGUIElementBaseTBase* nativeInstance, bool blocking)
	{
		if (nativeInstance->IsDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
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

	bool ScriptGUIElement::internal_getAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			return guiElem->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
		}

		return false;
	}

	void ScriptGUIElement::internal_setAcceptsKeyFocus(ScriptGUIElementBaseTBase* nativeInstance, bool accepts)
	{
		if (nativeInstance->IsDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
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

	MonoObject* ScriptGUIElement::internal_getParent(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return nullptr;

		if (nativeInstance->GetParent() != nullptr)
			return nativeInstance->GetParent()->getManagedInstance();

		return nullptr;
	}

	void ScriptGUIElement::internal_getBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->IsDestroyed())
		{
			*bounds = Rect2I();
			return;
		}

		*bounds = nativeInstance->GetGUIElement()->getBounds();
	}

	void ScriptGUIElement::internal_setBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setPosition(bounds->x, bounds->y);
		nativeInstance->GetGUIElement()->setWidth(bounds->width);
		nativeInstance->GetGUIElement()->setHeight(bounds->height);
	}

	void ScriptGUIElement::internal_getVisibleBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->IsDestroyed())
		{
			*bounds = Rect2I();
			return;
		}

		*bounds = nativeInstance->GetGUIElement()->getVisibleBounds();
	}

	void ScriptGUIElement::internal_getScreenBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->IsDestroyed())
		{
			*bounds = Rect2I();
			return;
		}

		*bounds = nativeInstance->GetGUIElement()->getScreenBounds();
	}

	void ScriptGUIElement::internal_SetPosition(ScriptGUIElementBaseTBase* nativeInstance, INT32 x, INT32 y)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setPosition(x, y);
	}

	void ScriptGUIElement::internal_SetWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 width)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setWidth(width);
	}

	void ScriptGUIElement::internal_SetFlexibleWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minWidth, UINT32 maxWidth)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setFlexibleWidth(minWidth, maxWidth);
	}

	void ScriptGUIElement::internal_SetHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 height)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setHeight(height);
	}

	void ScriptGUIElement::internal_SetFlexibleHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minHeight, UINT32 maxHeight)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->setFlexibleHeight(minHeight, maxHeight);
	}

	void ScriptGUIElement::internal_ResetDimensions(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->IsDestroyed())
			return;

		nativeInstance->GetGUIElement()->resetDimensions();
	}

	void ScriptGUIElement::internal_SetContextMenu(ScriptGUIElementBaseTBase* nativeInstance, ScriptContextMenu* contextMenu)
	{
		if (nativeInstance->IsDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);

			SPtr<GUIContextMenu> nativeContextMenu;
			if (contextMenu != nullptr)
				nativeContextMenu = contextMenu->GetInternal();

			guiElem->SetContextMenu(nativeContextMenu);
		}
	}

	MonoString* ScriptGUIElement::internal_GetStyle(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (!nativeInstance->IsDestroyed())
		{
			GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
			if (guiElemBase->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
				return MonoUtil::StringToMono(guiElem->GetStyleName());
			}
		}

		return MonoUtil::StringToMono(StringUtil::BLANK);
	}

	void ScriptGUIElement::internal_SetStyle(ScriptGUIElementBaseTBase* nativeInstance, MonoString* style)
	{
		if (!nativeInstance->IsDestroyed())
		{
			GUIElementBase* guiElemBase = nativeInstance->GetGUIElement();
			if (guiElemBase->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
				guiElem->SetStyle(MonoUtil::monoToString(style));
			}
		}
	}
}
