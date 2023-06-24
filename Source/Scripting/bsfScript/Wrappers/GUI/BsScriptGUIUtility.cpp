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
#include "GUI/BsGUIUtility.h"
#include "GUI/BsGUIHelper.h"

#include "Generated/BsScriptFont.generated.h"

using namespace bs;
ScriptGUILayoutUtility::ScriptGUILayoutUtility()
	: ScriptObject(nullptr)
{}

void ScriptGUILayoutUtility::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CalculateOptimalSize", (void*)&ScriptGUILayoutUtility::InternalCalculateOptimalSize);
	metaData.ScriptClass->AddInternalCall("Internal_CalculateBounds", (void*)&ScriptGUILayoutUtility::InternalCalculateBounds);
	metaData.ScriptClass->AddInternalCall("Internal_CalculateTextBounds", (void*)&ScriptGUILayoutUtility::InternalCalculateTextBounds);
}

void ScriptGUILayoutUtility::InternalCalculateOptimalSize(ScriptGUIElementBaseTBase* guiElement, Vector2I* output)
{
	*output = GUIUtility::CalcOptimalSize(guiElement->GetGuiElement());
	;
}

void ScriptGUILayoutUtility::InternalCalculateBounds(ScriptGUIElementBaseTBase* guiElement, ScriptGUILayout* relativeTo, Rect2I* output)
{
	if(guiElement->IsDestroyed())
	{
		*output = Rect2I();
		return;
	}

	GUIPanel* relativeToPanel = nullptr;
	if(relativeTo != nullptr && !relativeTo->IsDestroyed())
		relativeToPanel = static_cast<GUIPanel*>(relativeTo->GetGuiElement());

	*output = guiElement->GetGuiElement()->GetBounds(relativeToPanel);
}

void ScriptGUILayoutUtility::InternalCalculateTextBounds(MonoString* text, ScriptFont* fontPtr, int fontSize, Vector2I* output)
{
	String nativeText = MonoUtil::MonoToString(text);
	HFont nativeFont = fontPtr->GetHandle();

	*output = GUIHelper::CalcTextSize(nativeText, nativeFont, fontSize);
}
