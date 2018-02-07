//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	void ScriptGUIElementBaseTBase::initialize(GUIElementBase* element)
	{
		mElement = element;

		if (mElement != nullptr && mElement->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(element);
			guiElem->onFocusChanged.connect(std::bind(&ScriptGUIElementBaseTBase::onFocusChanged, this, _1));
		}
	}

	void ScriptGUIElementBaseTBase::onFocusChanged(ScriptGUIElementBaseTBase* thisPtr, bool focus)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(thisPtr->mGCHandle);

		if (focus)
			MonoUtil::invokeThunk(ScriptGUIElement::onFocusGainedThunk, instance);
		else
			MonoUtil::invokeThunk(ScriptGUIElement::onFocusLostThunk, instance);
	}

	MonoObject* ScriptGUIElementBaseTBase::getManagedInstance() const
	{
		return MonoUtil::getObjectFromGCHandle(mGCHandle);
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

	void ScriptGUIElementTBase::destroy()
	{
		if(!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->removeChild(this);

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

	void ScriptGUIElement::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::internal_destroy);
		metaData.scriptClass->addInternalCall("Internal_SetVisible", (void*)&ScriptGUIElement::internal_setVisible);
		metaData.scriptClass->addInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::internal_setActive);
		metaData.scriptClass->addInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::internal_setDisabled);
		metaData.scriptClass->addInternalCall("Internal_GetVisible", (void*)&ScriptGUIElement::internal_getVisible);
		metaData.scriptClass->addInternalCall("Internal_GetActive", (void*)&ScriptGUIElement::internal_getActive);
		metaData.scriptClass->addInternalCall("Internal_GetDisabled", (void*)&ScriptGUIElement::internal_getDisabled);
		metaData.scriptClass->addInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::internal_setFocus);
		metaData.scriptClass->addInternalCall("Internal_SetFocus", (void*)&ScriptGUIElement::internal_setFocus);
		metaData.scriptClass->addInternalCall("Internal_GetBlocking", (void*)&ScriptGUIElement::internal_getBlocking);
		metaData.scriptClass->addInternalCall("Internal_SetBlocking", (void*)&ScriptGUIElement::internal_setBlocking);
		metaData.scriptClass->addInternalCall("Internal_GetBounds", (void*)&ScriptGUIElement::internal_getBounds);
		metaData.scriptClass->addInternalCall("Internal_SetBounds", (void*)&ScriptGUIElement::internal_setBounds);
		metaData.scriptClass->addInternalCall("Internal_GetVisibleBounds", (void*)&ScriptGUIElement::internal_getVisibleBounds);
		metaData.scriptClass->addInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::internal_SetPosition);
		metaData.scriptClass->addInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::internal_SetWidth);
		metaData.scriptClass->addInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::internal_SetFlexibleWidth);
		metaData.scriptClass->addInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::internal_SetHeight);
		metaData.scriptClass->addInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::internal_SetFlexibleHeight);
		metaData.scriptClass->addInternalCall("Internal_ResetDimensions", (void*)&ScriptGUIElement::internal_ResetDimensions);
		metaData.scriptClass->addInternalCall("Internal_SetContextMenu", (void*)&ScriptGUIElement::internal_SetContextMenu);
		metaData.scriptClass->addInternalCall("Internal_GetStyle", (void*)&ScriptGUIElement::internal_GetStyle);
		metaData.scriptClass->addInternalCall("Internal_SetStyle", (void*)&ScriptGUIElement::internal_SetStyle);
		metaData.scriptClass->addInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::internal_getParent);

		onFocusGainedThunk = (OnFocusChangedThunkDef)metaData.scriptClass->getMethod("Internal_OnFocusGained", 0)->getThunk();
		onFocusLostThunk = (OnFocusChangedThunkDef)metaData.scriptClass->getMethod("Internal_OnFocusLost", 0)->getThunk();
	}

	void ScriptGUIElement::internal_destroy(ScriptGUIElementBaseTBase* nativeInstance)
	{
		nativeInstance->destroy();
	}

	void ScriptGUIElement::internal_setVisible(ScriptGUIElementBaseTBase* nativeInstance, bool visible)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setVisible(visible);
	}

	void ScriptGUIElement::internal_setActive(ScriptGUIElementBaseTBase* nativeInstance, bool enabled)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setActive(enabled);
	}

	void ScriptGUIElement::internal_setDisabled(ScriptGUIElementBaseTBase* nativeInstance, bool disabled)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setDisabled(disabled);
	}

	void ScriptGUIElement::internal_setFocus(ScriptGUIElementBaseTBase* nativeInstance, bool focus)
	{
		if (nativeInstance->isDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			guiElem->setFocus(focus);
		}		
	}

	bool ScriptGUIElement::internal_getVisible(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		return guiElemBase->_isVisible();
	}

	bool ScriptGUIElement::internal_getActive(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		return guiElemBase->_isActive();
	}

	bool ScriptGUIElement::internal_getDisabled(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		return guiElemBase->_isDisabled();
	}

	bool ScriptGUIElement::internal_getBlocking(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return false;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			return guiElem->getBlockPointerEvents();
		}

		return false;
	}

	void ScriptGUIElement::internal_setBlocking(ScriptGUIElementBaseTBase* nativeInstance, bool blocking)
	{
		if (nativeInstance->isDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
			guiElem->setBlockPointerEvents(blocking);
		}
	}

	MonoObject* ScriptGUIElement::internal_getParent(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return nullptr;

		if (nativeInstance->getParent() != nullptr)
			return nativeInstance->getParent()->getManagedInstance();

		return nullptr;
	}

	void ScriptGUIElement::internal_getBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->isDestroyed())
		{
			*bounds = Rect2I();
			return;
		}

		*bounds = nativeInstance->getGUIElement()->getBounds();
	}

	void ScriptGUIElement::internal_setBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setPosition(bounds->x, bounds->y);
		nativeInstance->getGUIElement()->setWidth(bounds->width);
		nativeInstance->getGUIElement()->setHeight(bounds->height);
	}

	void ScriptGUIElement::internal_getVisibleBounds(ScriptGUIElementBaseTBase* nativeInstance, Rect2I* bounds)
	{
		if (nativeInstance->isDestroyed())
		{
			*bounds = Rect2I();
			return;
		}

		*bounds = nativeInstance->getGUIElement()->getVisibleBounds();
	}

	void ScriptGUIElement::internal_SetPosition(ScriptGUIElementBaseTBase* nativeInstance, INT32 x, INT32 y)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setPosition(x, y);
	}

	void ScriptGUIElement::internal_SetWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 width)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setWidth(width);
	}

	void ScriptGUIElement::internal_SetFlexibleWidth(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minWidth, UINT32 maxWidth)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setFlexibleWidth(minWidth, maxWidth);
	}

	void ScriptGUIElement::internal_SetHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 height)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setHeight(height);
	}

	void ScriptGUIElement::internal_SetFlexibleHeight(ScriptGUIElementBaseTBase* nativeInstance, UINT32 minHeight, UINT32 maxHeight)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->setFlexibleHeight(minHeight, maxHeight);
	}

	void ScriptGUIElement::internal_ResetDimensions(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (nativeInstance->isDestroyed())
			return;

		nativeInstance->getGUIElement()->resetDimensions();
	}

	void ScriptGUIElement::internal_SetContextMenu(ScriptGUIElementBaseTBase* nativeInstance, ScriptContextMenu* contextMenu)
	{
		if (nativeInstance->isDestroyed())
			return;

		GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
		if (guiElemBase->_getType() == GUIElementBase::Type::Element)
		{
			GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);

			SPtr<GUIContextMenu> nativeContextMenu;
			if (contextMenu != nullptr)
				nativeContextMenu = contextMenu->getInternal();

			guiElem->setContextMenu(nativeContextMenu);
		}
	}

	MonoString* ScriptGUIElement::internal_GetStyle(ScriptGUIElementBaseTBase* nativeInstance)
	{
		if (!nativeInstance->isDestroyed())
		{
			GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
			if (guiElemBase->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
				return MonoUtil::stringToMono(guiElem->getStyleName());
			}
		}

		return MonoUtil::stringToMono(StringUtil::BLANK);
	}

	void ScriptGUIElement::internal_SetStyle(ScriptGUIElementBaseTBase* nativeInstance, MonoString* style)
	{
		if (!nativeInstance->isDestroyed())
		{
			GUIElementBase* guiElemBase = nativeInstance->getGUIElement();
			if (guiElemBase->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* guiElem = static_cast<GUIElement*>(guiElemBase);
				guiElem->setStyle(MonoUtil::monoToString(style));
			}
		}
	}
}