//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIUtility.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIElement.h"
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

void ScriptGUILayoutUtility::InternalCalculateOptimalSize(ScriptGUIElementWrapper* guiElement, Vector2I* output)
{
	*output = GUIUtility::CalcOptimalSize(guiElement->GetNativeObject());
}

void ScriptGUILayoutUtility::InternalCalculateBounds(ScriptGUIElementWrapper* guiElement, ScriptGUILayoutWrapperBase* relativeTo, Rect2I* output)
{
	if(!guiElement->IsNativeObjectValid())
	{
		*output = Rect2I();
		return;
	}

	GUILayout* relativeToPanel = nullptr;
	if(relativeTo != nullptr && relativeTo->IsNativeObjectValid())
		relativeToPanel = relativeTo->GetNativeObject();

	*output = guiElement->GetNativeObject()->GetBoundsRelativeTo(relativeToPanel);
}

void ScriptGUILayoutUtility::InternalCalculateTextBounds(MonoString* text, ScriptFont* fontPtr, float fontSize, Vector2I* output)
{
	String nativeText = MonoUtil::MonoToString(text);
	HFont nativeFont = fontPtr->GetNativeObjectAsHandle();

	*output = GUIHelper::CalculateTextBounds(nativeText, nativeFont, fontSize);
}
