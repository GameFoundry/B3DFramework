//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Wrappers/GUI/BsScriptGUIScrollArea.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIScrollArea.h"
#include "BsMonoUtil.h"

using namespace bs;
ScriptGUILayout::ScriptGUILayout(GUILayout* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{}

void ScriptGUILayout::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstanceX", (void*)&ScriptGUILayout::InternalCreateInstanceX);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstanceY", (void*)&ScriptGUILayout::InternalCreateInstanceY);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstancePanel", (void*)&ScriptGUILayout::InternalCreateInstancePanel);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstanceYFromScrollArea", (void*)&ScriptGUILayout::InternalCreateInstanceYFromScrollArea);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::InternalAddElement);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::InternalInsertElement);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::InternalGetChildCount);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::InternalGetChild);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUILayout::InternalClear);
}

MonoObject* ScriptGUILayout::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

void ScriptGUILayout::InternalCreateInstanceX(MonoObject* instance, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUILayout* layout = GUILayoutX::Create(options);
	ScriptObjectWrapper::Create<ScriptGUILayout>(layout, instance);
}

void ScriptGUILayout::InternalCreateInstanceY(MonoObject* instance, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUILayout* layout = GUILayoutY::Create(options);
	ScriptObjectWrapper::Create<ScriptGUILayout>(layout, instance);
}

void ScriptGUILayout::InternalCreateInstancePanel(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIPanel* panel = GUIPanel::Create(depth, depthRangeMin, depthRangeMax, options);
	ScriptObjectWrapper::Create<ScriptGUIPanel>(panel, instance);
}

void ScriptGUILayout::InternalCreateInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea)
{
	ScriptGUIScrollArea* scriptScrollArea = ScriptGUIScrollArea::ToNative(parentScrollArea);
	GUIScrollArea* scrollArea = (GUIScrollArea*)scriptScrollArea->GetGuiElement();

	GUILayout* nativeLayout = &scrollArea->GetLayout();

	ScriptGUIScrollAreaLayout* const layoutWrapper = ScriptObjectWrapper::Create<ScriptGUIScrollAreaLayout>(nativeLayout, instance);

	// This method is expected to be called during GUIScrollArea construction, so we finish its initialization
	scriptScrollArea->Initialize(layoutWrapper);
}

void ScriptGUILayout::InternalAddElement(ScriptGUILayoutWrapperBase* self, ScriptGUIElementWrapper* element)
{
	if(!self->IsNativeObjectValid() || !element->IsNativeObjectValid())
		return;

	self->GetNativeObject()->AddElement(element->GetNativeObject());
}

void ScriptGUILayout::InternalInsertElement(ScriptGUILayoutWrapperBase* self, u32 index, ScriptGUIElementWrapper* element)
{
	if(!self->IsNativeObjectValid() || !element->IsNativeObjectValid())
		return;

	self->GetNativeObject()->InsertElement(index, element->GetNativeObject());
}

u32 ScriptGUILayout::InternalGetChildCount(ScriptGUILayoutWrapperBase* self)
{
	if(!self->IsNativeObjectValid())
		return 0;

	return self->GetNativeObject()->GetChildCount();
}

MonoObject* ScriptGUILayout::InternalGetChild(ScriptGUILayoutWrapperBase* self, u32 index)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	GUILayout* const nativeObject = self->GetNativeObject();
	if(index >= nativeObject->GetChildCount())
		return nullptr;

	GUIElement* const child = nativeObject->GetChild(index);
	if(!B3D_ENSURE(child != nullptr))
		return nullptr;

	// Note: This should be calling GetOrCreateScriptObject, but for the time being we don't support ad-hoc script object creation for
	// GUI elements. Instead script can only access script objects it has itself created. We can easily change this in the future, by
	// adding RTTI IDs to all GUI elements, and then registering them in a lookup similar to other reflectable types.
	ScriptGUIElementWrapper* const childScriptObjectWrapper = static_cast<ScriptGUIElementWrapper*>(child->GetScriptObjectWrapper());
	if(childScriptObjectWrapper == nullptr)
		return nullptr;

	return childScriptObjectWrapper->GetScriptObject();
}

void ScriptGUILayout::InternalClear(ScriptGUILayoutWrapperBase* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->Clear();
}

ScriptGUIPanel::ScriptGUIPanel(GUIPanel* nativeObject)
	: ScriptGUIPanel(nativeObject)
{}

void ScriptGUIPanel::SetupScriptBindings()
{}

MonoObject* ScriptGUIPanel::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

ScriptGUIScrollAreaLayout::ScriptGUIScrollAreaLayout(MonoObject* instance, GUILayout* layout)
	: ScriptGUILayout(instance, layout, false), mParentScrollArea(nullptr)
{
}

void ScriptGUIScrollAreaLayout::Destroy()
{
	if(!mIsDestroyed)
	{
		if(mParentScrollArea != nullptr)
			mParentScrollArea->NotifyLayoutDestroyed();

		while(mChildren.size() > 0)
		{
			ChildInfo childInfo = mChildren[0];
			childInfo.Element->Destroy();
		}

		mLayout = nullptr;
		mIsDestroyed = true;
	}
}
