//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIUtility.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptGUIElement.h"
#include "BsGUIPanel.h"
#include "BsGUIElementBase.h"
#include "BsScriptGUILayout.h"
#include "BsGUILayoutUtility.h"
#include "BsGUIHelper.h"
#include "BsScriptFont.h"

namespace bs
{
	ScriptGUILayoutUtility::ScriptGUILayoutUtility()
		:ScriptObject(nullptr)
	{ }

	void ScriptGUILayoutUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CalculateOptimalSize", &ScriptGUILayoutUtility::internal_CalculateOptimalSize);
		metaData.scriptClass->addInternalCall("Internal_CalculateBounds", &ScriptGUILayoutUtility::internal_CalculateBounds);
		metaData.scriptClass->addInternalCall("Internal_CalculateTextBounds", &ScriptGUILayoutUtility::internal_CalculateTextBounds);
	}

	void ScriptGUILayoutUtility::internal_CalculateOptimalSize(ScriptGUIElementBaseTBase* guiElement, Vector2I* output)
	{
		*output = GUILayoutUtility::calcOptimalSize(guiElement->getGUIElement());;
	}

	void ScriptGUILayoutUtility::internal_CalculateBounds(ScriptGUIElementBaseTBase* guiElement, ScriptGUILayout* relativeTo, Rect2I* output)
	{
		if (guiElement->isDestroyed())
		{
			*output = Rect2I();
			return;
		}

		GUIPanel* relativeToPanel = nullptr;
		if (relativeTo != nullptr && !relativeTo->isDestroyed())
			relativeToPanel = static_cast<GUIPanel*>(relativeTo->getGUIElement());

		*output = guiElement->getGUIElement()->getBounds(relativeToPanel);
	}

	void ScriptGUILayoutUtility::internal_CalculateTextBounds(MonoString* text, ScriptFont* fontPtr, int fontSize, Vector2I* output)
	{
		WString nativeText = MonoUtil::monoToWString(text);
		HFont nativeFont = fontPtr->getHandle();

		*output = GUIHelper::calcTextSize(nativeText, nativeFont, fontSize);
	}
}