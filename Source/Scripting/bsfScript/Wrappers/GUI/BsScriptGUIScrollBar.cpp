//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIScrollBar.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "GUI/BsGUIScrollBar.h"
#include "GUI/BsGUIHorizontalScrollBar.h"
#include "GUI/BsGUIVerticalScrollBar.h"

using namespace std::placeholders;

using namespace bs;
ScriptGUIScrollBar::ScriptGUIScrollBar(MonoObject* instance)
	: TScriptGUIInteractable(instance, nullptr)
{
}

void ScriptGUIScrollBar::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptGUIScrollBar::InternalGetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIScrollBar::InternalSetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_GetHandleSize", (void*)&ScriptGUIScrollBar::InternalGetHandleSize);
	metaData.ScriptClass->AddInternalCall("Internal_SetHandleSize", (void*)&ScriptGUIScrollBar::InternalSetHandleSize);
}

float ScriptGUIScrollBar::InternalGetPosition(ScriptGUIElementBase* nativeInstance)
{
	GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->GetGuiElement();
	return scrollBar->GetScrollHandlePosition();
}

void ScriptGUIScrollBar::InternalSetPosition(ScriptGUIElementBase* nativeInstance, float percent)
{
	GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->GetGuiElement();
	scrollBar->SetScrollPosInternal(percent);
}

float ScriptGUIScrollBar::InternalGetHandleSize(ScriptGUIElementBase* nativeInstance)
{
	GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->GetGuiElement();
	return scrollBar->GetScrollHandleSize();
}

void ScriptGUIScrollBar::InternalSetHandleSize(ScriptGUIElementBase* nativeInstance, float percent)
{
	GUIScrollBar* scrollBar = (GUIScrollBar*)nativeInstance->GetGuiElement();
	scrollBar->SetScrollHandleSize(percent);
}

ScriptGUIScrollBarH::OnScrolledThunkDef ScriptGUIScrollBarH::onScrolledThunk;

ScriptGUIScrollBarH::ScriptGUIScrollBarH(MonoObject* instance, GUIHorizontalScrollBar* scrollBar)
	: TScriptGUIInteractable(instance, scrollBar)
{
}

void ScriptGUIScrollBarH::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollBarH::InternalCreateInstance);

	onScrolledThunk = (OnScrolledThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnScroll", 1)->GetThunk();
}

void ScriptGUIScrollBarH::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIHorizontalScrollBar* guiScrollBar = GUIHorizontalScrollBar::Create(MonoUtil::MonoToString(style), options);
	ScriptGUIScrollBarH* scriptScrollBar = new(B3DAllocate<ScriptGUIScrollBarH>()) ScriptGUIScrollBarH(instance, guiScrollBar);

	guiScrollBar->OnScrollOrResize.Connect(std::bind(&ScriptGUIScrollBarH::OnScroll, scriptScrollBar, _1, _2));
}

void ScriptGUIScrollBarH::OnScroll(float position, float size)
{
	MonoUtil::InvokeThunk(onScrolledThunk, GetManagedInstance(), position);
}

ScriptGUIScrollBarV::OnScrolledThunkDef ScriptGUIScrollBarV::onScrolledThunk;

ScriptGUIScrollBarV::ScriptGUIScrollBarV(MonoObject* instance, GUIVerticalScrollBar* scrollBar)
	: TScriptGUIInteractable(instance, scrollBar)
{
}

void ScriptGUIScrollBarV::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollBarV::InternalCreateInstance);

	onScrolledThunk = (OnScrolledThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnScroll", 1)->GetThunk();
}

void ScriptGUIScrollBarV::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIVerticalScrollBar* guiScrollBar = GUIVerticalScrollBar::Create(MonoUtil::MonoToString(style), options);
	ScriptGUIScrollBarV* scriptScrollBar = new(B3DAllocate<ScriptGUIScrollBarV>()) ScriptGUIScrollBarV(instance, guiScrollBar);

	guiScrollBar->OnScrollOrResize.Connect(std::bind(&ScriptGUIScrollBarV::OnScroll, scriptScrollBar, _1, _2));
}

void ScriptGUIScrollBarV::OnScroll(float position, float size)
{
	MonoUtil::InvokeThunk(onScrolledThunk, GetManagedInstance(), position);
}

ScriptGUIResizeableScrollBarH::OnScrollOrResizeThunkDef ScriptGUIResizeableScrollBarH::onScrollOrResizeThunk;

ScriptGUIResizeableScrollBarH::ScriptGUIResizeableScrollBarH(MonoObject* instance, GUIResizableHorizontalScrollBar* scrollBar)
	: TScriptGUIInteractable(instance, scrollBar)
{
}

void ScriptGUIResizeableScrollBarH::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIResizeableScrollBarH::InternalCreateInstance);

	onScrollOrResizeThunk = (OnScrollOrResizeThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnScrollOrResize", 2)->GetThunk();
}

void ScriptGUIResizeableScrollBarH::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIResizableHorizontalScrollBar* guiScrollBar = GUIResizableHorizontalScrollBar::Create(MonoUtil::MonoToString(style), options);
	ScriptGUIResizeableScrollBarH* scriptScrollBar = new(B3DAllocate<ScriptGUIResizeableScrollBarH>()) ScriptGUIResizeableScrollBarH(instance, guiScrollBar);

	guiScrollBar->OnScrollOrResize.Connect(std::bind(&ScriptGUIResizeableScrollBarH::OnScroll, scriptScrollBar, _1, _2));
}

void ScriptGUIResizeableScrollBarH::OnScroll(float position, float size)
{
	MonoUtil::InvokeThunk(onScrollOrResizeThunk, GetManagedInstance(), position, size);
}

ScriptGUIResizeableScrollBarV::OnScrollOrResizeThunkDef ScriptGUIResizeableScrollBarV::onScrollOrResizeThunk;

ScriptGUIResizeableScrollBarV::ScriptGUIResizeableScrollBarV(MonoObject* instance, GUIResizableVerticalScrollBar* scrollBar)
	: TScriptGUIInteractable(instance, scrollBar)
{
}

void ScriptGUIResizeableScrollBarV::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIResizeableScrollBarV::InternalCreateInstance);

	onScrollOrResizeThunk = (OnScrollOrResizeThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnScrollOrResize", 2)->GetThunk();
}

void ScriptGUIResizeableScrollBarV::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIResizableVerticalScrollBar* guiScrollBar = GUIResizableVerticalScrollBar::Create(MonoUtil::MonoToString(style), options);
	ScriptGUIResizeableScrollBarV* scriptScrollBar = new(B3DAllocate<ScriptGUIResizeableScrollBarV>()) ScriptGUIResizeableScrollBarV(instance, guiScrollBar);

	guiScrollBar->OnScrollOrResize.Connect(std::bind(&ScriptGUIResizeableScrollBarV::OnScroll, scriptScrollBar, _1, _2));
}

void ScriptGUIResizeableScrollBarV::OnScroll(float position, float size)
{
	MonoUtil::InvokeThunk(onScrollOrResizeThunk, GetManagedInstance(), position, size);
}
