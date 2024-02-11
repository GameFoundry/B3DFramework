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
#include "Wrappers/GUI/BsScriptGUILayout.h"

using namespace bs;
ScriptGUIScrollArea::ScriptGUIScrollArea(MonoObject* instance, GUIScrollArea* scrollArea)
	: TScriptGUIInteractable(instance, scrollArea), mLayout(nullptr)
{
}

void ScriptGUIScrollArea::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollArea::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_GetContentBounds", (void*)&ScriptGUIScrollArea::InternalGetContentBounds);
	metaData.ScriptClass->AddInternalCall("Internal_GetHorzScroll", (void*)&ScriptGUIScrollArea::InternalGetHorzScroll);
	metaData.ScriptClass->AddInternalCall("Internal_SetHorzScroll", (void*)&ScriptGUIScrollArea::InternalSetHorzScroll);
	metaData.ScriptClass->AddInternalCall("Internal_GetVertScroll", (void*)&ScriptGUIScrollArea::InternalGetVertScroll);
	metaData.ScriptClass->AddInternalCall("Internal_SetVertScroll", (void*)&ScriptGUIScrollArea::InternalSetVertScroll);
	metaData.ScriptClass->AddInternalCall("Internal_GetScrollBarWidth", (void*)&ScriptGUIScrollArea::InternalGetScrollBarWidth);
}

void ScriptGUIScrollArea::Initialize(ScriptGUIScrollAreaLayout* layout)
{
	mLayout = layout;
	mLayout->mParentScrollArea = this;
}

void ScriptGUIScrollArea::NotifyLayoutDestroyed()
{
	mLayout = nullptr;
}

void ScriptGUIScrollArea::Destroy()
{
	if(!mIsDestroyed)
	{
		if(mParent != nullptr)
			mParent->RemoveChild(this);

		if(mLayout != nullptr)
		{
			mLayout->Destroy();
			mLayout = nullptr;
		}

		mElement->Destroy();
		mElement = nullptr;

		mIsDestroyed = true;
	}
}

void ScriptGUIScrollArea::InternalCreateInstance(MonoObject* instance, ScrollBarType vertBarType, ScrollBarType horzBarType, MonoString* scrollBarStyle, MonoString* scrollAreaStyle, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIScrollArea* guiScrollArea = GUIScrollArea::Create(vertBarType, horzBarType, options, MonoUtil::MonoToString(scrollBarStyle), MonoUtil::MonoToString(scrollAreaStyle));

	new(B3DAllocate<ScriptGUIScrollArea>()) ScriptGUIScrollArea(instance, guiScrollArea);
}

void ScriptGUIScrollArea::InternalGetContentBounds(ScriptGUIScrollArea* nativeInstance, Rect2I* bounds)
{
	GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->GetGuiElement());
	*bounds = guiScrollArea->GetContentBounds();
}

float ScriptGUIScrollArea::InternalGetHorzScroll(ScriptGUIScrollArea* nativeInstance)
{
	GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->GetGuiElement());
	return guiScrollArea->GetHorizontalScroll();
}

void ScriptGUIScrollArea::InternalSetHorzScroll(ScriptGUIScrollArea* nativeInstance, float value)
{
	GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->GetGuiElement());
	guiScrollArea->ScrollToHorizontal(value);
}

float ScriptGUIScrollArea::InternalGetVertScroll(ScriptGUIScrollArea* nativeInstance)
{
	GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->GetGuiElement());
	return guiScrollArea->GetVerticalScroll();
}

void ScriptGUIScrollArea::InternalSetVertScroll(ScriptGUIScrollArea* nativeInstance, float value)
{
	GUIScrollArea* guiScrollArea = static_cast<GUIScrollArea*>(nativeInstance->GetGuiElement());
	guiScrollArea->ScrollToVertical(value);
}

int ScriptGUIScrollArea::InternalGetScrollBarWidth(ScriptGUIScrollArea* nativeInstance)
{
	return GUIScrollArea::kScrollBarWidth;
}
