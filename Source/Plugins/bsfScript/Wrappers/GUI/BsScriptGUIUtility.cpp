//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIUtility.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIElementBase.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "GUI/BsGUILayoutUtility.h"
#include "GUI/BsGUIHelper.h"

#include "BsScriptFont.generated.h"

namespace bs
{
	ScriptGUILayoutUtility::ScriptGUILayoutUtility()
		:ScriptObject(nullptr)
	{ }

	void ScriptGUILayoutUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CalculateOptimalSize", (void*)&ScriptGUILayoutUtility::internal_CalculateOptimalSize);
		metaData.scriptClass->addInternalCall("Internal_CalculateBounds", (void*)&ScriptGUILayoutUtility::internal_CalculateBounds);
		metaData.scriptClass->addInternalCall("Internal_CalculateTextBounds", (void*)&ScriptGUILayoutUtility::internal_CalculateTextBounds);
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
		String nativeText = MonoUtil::monoToString(text);
		HFont nativeFont = fontPtr->getHandle();

		*output = GUIHelper::calcTextSize(nativeText, nativeFont, fontSize);
	}
}