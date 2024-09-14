//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUILayout.h"
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
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::InternalAddElement);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::InternalInsertElement);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::InternalGetChildCount);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::InternalGetChild);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUILayout::InternalClear);
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
	: TScriptGUIElementWrapper(nativeObject)
{}

void ScriptGUIPanel::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIPanel::InternalCreate);
}

MonoObject* ScriptGUIPanel::CreateScriptObject(bool construct)
{
	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	if(construct)
		return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

	return sInteropMetaData.ScriptClass->CreateInstance(false);
}

void ScriptGUIPanel::InternalCreate(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIPanel* panel = GUIPanel::Create(depth, depthRangeMin, depthRangeMax, options);
	ScriptObjectWrapper::Create<ScriptGUIPanel>(panel, instance);
}

ScriptGUILayoutX::ScriptGUILayoutX(GUILayoutX* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{}

void ScriptGUILayoutX::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUILayoutX::InternalCreate);
}

MonoObject* ScriptGUILayoutX::CreateScriptObject(bool construct)
{
	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	if(construct)
		return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

	return sInteropMetaData.ScriptClass->CreateInstance(false);
}

void ScriptGUILayoutX::InternalCreate(MonoObject* instance, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUILayoutX* layout = GUILayoutX::Create(options);
	ScriptObjectWrapper::Create<ScriptGUILayoutX>(layout, instance);
}

ScriptGUILayoutY::ScriptGUILayoutY(GUILayoutY* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{ }

void ScriptGUILayoutY::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUILayoutY::InternalCreate);
}

MonoObject* ScriptGUILayoutY::CreateScriptObject(bool construct)
{
	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	if(construct)
		return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

	return sInteropMetaData.ScriptClass->CreateInstance(false);
}

void ScriptGUILayoutY::InternalCreate(MonoObject* instance, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUILayoutY* layout = GUILayoutY::Create(options);
	ScriptObjectWrapper::Create<ScriptGUILayoutY>(layout, instance);
}
