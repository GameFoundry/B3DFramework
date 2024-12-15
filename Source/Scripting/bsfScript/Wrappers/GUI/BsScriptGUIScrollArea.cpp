//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIScrollArea.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

using namespace bs;
ScriptGUIScrollArea::ScriptGUIScrollArea(GUIScrollArea* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptGUIScrollArea::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIScrollArea::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayout", (void*)&ScriptGUIScrollArea::InternalGetLayout);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContentBounds", (void*)&ScriptGUIScrollArea::InternalGetContentBounds);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHorzScroll", (void*)&ScriptGUIScrollArea::InternalGetHorzScroll);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHorzScroll", (void*)&ScriptGUIScrollArea::InternalSetHorzScroll);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVertScroll", (void*)&ScriptGUIScrollArea::InternalGetVertScroll);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetVertScroll", (void*)&ScriptGUIScrollArea::InternalSetVertScroll);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScrollBarWidth", (void*)&ScriptGUIScrollArea::InternalGetScrollBarWidth);
}

MonoObject* ScriptGUIScrollArea::CreateScriptObject(bool construct)
{
	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	if(construct)
		return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

	return sInteropMetaData.ScriptClass->CreateInstance(false);
}

void ScriptGUIScrollArea::InternalCreateInstance(MonoObject* instance, ScrollBarType vertBarType, ScrollBarType horzBarType, MonoString* scrollBarStyle, MonoString* scrollAreaStyle, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIScrollArea* guiScrollArea = GUIScrollArea::Create(vertBarType, horzBarType, options, MonoUtil::MonoToString(scrollBarStyle), MonoUtil::MonoToString(scrollAreaStyle));
	ScriptObjectWrapper::Create<ScriptGUIScrollArea>(guiScrollArea, instance);
}

MonoObject* ScriptGUIScrollArea::InternalGetLayout(ScriptGUIScrollArea* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;
	
	return ScriptGUILayoutY::GetOrCreateScriptObject(&self->GetNativeObject()->GetLayout());
}

void ScriptGUIScrollArea::InternalGetContentBounds(ScriptGUIScrollArea* self, Rect2I* bounds)
{
	if(!self->IsNativeObjectValid())
		*bounds = Rect2I::kEmpty;

	*bounds = self->GetNativeObject()->GetContentBounds();
}

float ScriptGUIScrollArea::InternalGetHorzScroll(ScriptGUIScrollArea* self)
{
	if(!self->IsNativeObjectValid())
		return 0.0f;

	return self->GetNativeObject()->GetHorizontalScroll();
}

void ScriptGUIScrollArea::InternalSetHorzScroll(ScriptGUIScrollArea* self, float value)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->ScrollToHorizontal(value);
}

float ScriptGUIScrollArea::InternalGetVertScroll(ScriptGUIScrollArea* self)
{
	if(!self->IsNativeObjectValid())
		return 0.0f;

	return self->GetNativeObject()->GetVerticalScroll();
}

void ScriptGUIScrollArea::InternalSetVertScroll(ScriptGUIScrollArea* self, float value)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->ScrollToVertical(value);
}

int ScriptGUIScrollArea::InternalGetScrollBarWidth(ScriptGUIScrollArea* self)
{
	return GUIScrollArea::kScrollBarWidth;
}
