//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIScrollArea.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUIScrollArea.h"
#include "BsGUIOptions.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

namespace bs
{
	ScriptGUIScrollArea::ScriptGUIScrollArea(MonoObject* instance, GUIScrollArea* scrollArea)
		:TScriptGUIElement(instance, scrollArea), mLayout(nullptr)
	{

	}

	void ScriptGUIScrollArea::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIScrollArea::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetContentBounds", &ScriptGUIScrollArea::internal_getContentBounds);
		metaData.scriptClass->addInternalCall("Internal_GetHorzScroll", &ScriptGUIScrollArea::internal_getHorzScroll);
		metaData.scriptClass->addInternalCall("Internal_SetHorzScroll", &ScriptGUIScrollArea::internal_setHorzScroll);
		metaData.scriptClass->addInternalCall("Internal_GetVertScroll", &ScriptGUIScrollArea::internal_getVertScroll);
		metaData.scriptClass->addInternalCall("Internal_SetVertScroll", &ScriptGUIScrollArea::internal_setVertScroll);
		metaData.scriptClass->addInternalCall("Internal_GetScrollBarWidth", &ScriptGUIScrollArea::internal_getScrollBarWidth);
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
			toString(MonoUtil::monoToWString(scrollBarStyle)), toString(MonoUtil::monoToWString(scrollAreaStyle)));

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