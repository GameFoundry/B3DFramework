//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIScrollArea.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

namespace bs
{
	ScriptGUIScrollArea::ScriptGUIScrollArea(MonoObject* instance, GUIScrollArea* scrollArea)
		:TScriptGUIElement(instance, scrollArea), mLayout(nullptr)
	{

	}

	void ScriptGUIScrollArea::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollArea::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetContentBounds", (void*)&ScriptGUIScrollArea::internal_getContentBounds);
		metaData.scriptClass->addInternalCall("Internal_GetHorzScroll", (void*)&ScriptGUIScrollArea::internal_getHorzScroll);
		metaData.scriptClass->addInternalCall("Internal_SetHorzScroll", (void*)&ScriptGUIScrollArea::internal_setHorzScroll);
		metaData.scriptClass->addInternalCall("Internal_GetVertScroll", (void*)&ScriptGUIScrollArea::internal_getVertScroll);
		metaData.scriptClass->addInternalCall("Internal_SetVertScroll", (void*)&ScriptGUIScrollArea::internal_setVertScroll);
		metaData.scriptClass->addInternalCall("Internal_GetScrollBarWidth", (void*)&ScriptGUIScrollArea::internal_getScrollBarWidth);
	}

	void ScriptGUIScrollArea::initialize(ScriptGUIScrollAreaLayout* layout)
	{
		mLayout = layout;
		mLayout->mParentScrollArea = this;
	}

	void ScriptGUIScrollArea::notifyLayoutDestroyed()
	{
		mLayout = nullptr;
	}

	void ScriptGUIScrollArea::destroy()
	{
		if (!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->removeChild(this);

			if (mLayout != nullptr)
			{
				mLayout->destroy();
				mLayout = nullptr;
			}

			if (mElement->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement::destroy((GUIElement*)mElement);
				mElement = nullptr;

				mIsDestroyed = true;
			}
		}
	}

	void ScriptGUIScrollArea::internal_createInstance(MonoObject* instance, ScrollBarType vertBarType, ScrollBarType horzBarType, 
		MonoString* scrollBarStyle, MonoString* scrollAreaStyle, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIScrollArea* guiScrollArea = GUIScrollArea::create(vertBarType, horzBarType, options, 
			MonoUtil::monoToString(scrollBarStyle), MonoUtil::monoToString(scrollAreaStyle));

		new (bs_alloc<ScriptGUIScrollArea>()) ScriptGUIScrollArea(instance, guiScrollArea);
	}

	void ScriptGUIScrollArea::internal_getContentBounds(ScriptGUIScrollArea* nativeInstance, Rect2I* bounds)
	{
		GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->getGUIElement());
		*bounds = guiScrollArea->getContentBounds();
	}

	float ScriptGUIScrollArea::internal_getHorzScroll(ScriptGUIScrollArea* nativeInstance)
	{
		GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->getGUIElement());
		return guiScrollArea->getHorizontalScroll();
	}

	void ScriptGUIScrollArea::internal_setHorzScroll(ScriptGUIScrollArea* nativeInstance, float value)
	{
		GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->getGUIElement());
		guiScrollArea->scrollToHorizontal(value);
	}

	float ScriptGUIScrollArea::internal_getVertScroll(ScriptGUIScrollArea* nativeInstance)
	{
		GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->getGUIElement());
		return guiScrollArea->getVerticalScroll();
	}

	void ScriptGUIScrollArea::internal_setVertScroll(ScriptGUIScrollArea* nativeInstance, float value)
	{
		GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->getGUIElement());
		guiScrollArea->scrollToVertical(value);
	}

	int ScriptGUIScrollArea::internal_getScrollBarWidth(ScriptGUIScrollArea* nativeInstance)
	{
		return GUIScrollArea::ScrollBarWidth;
	}
}