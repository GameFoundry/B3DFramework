//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptContextMenu.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIContextMenu.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "GUI/BsGUILayout.h"

#include "Generated/BsScriptHString.generated.h"

using namespace std::placeholders;

using namespace bs;
ScriptContextMenu::OnEntryTriggeredThunkDef ScriptContextMenu::onEntryTriggered;

ScriptContextMenu::ScriptContextMenu(const SPtr<GUIContextMenu>& nativeObject)
	: TScriptNonReflectableWrapper(nativeObject)
{ }

void ScriptContextMenu::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptContextMenu::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Open", (void*)&ScriptContextMenu::InternalOpen);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddItem", (void*)&ScriptContextMenu::InternalAddItem);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddSeparator", (void*)&ScriptContextMenu::InternalAddSeparator);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLocalizedName", (void*)&ScriptContextMenu::InternalSetLocalizedName);

	onEntryTriggered = (OnEntryTriggeredThunkDef)sInteropMetaData.ScriptClass->GetMethod("InternalDoOnEntryTriggered", 1)->GetThunk();
}

MonoObject* ScriptContextMenu::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

void ScriptContextMenu::InternalCreateInstance(MonoObject* scriptObject)
{
	auto nativeObject = B3DMakeShared<GUIContextMenu>();
	ScriptObjectWrapper::Create<ScriptContextMenu>(nativeObject, scriptObject);
}

void ScriptContextMenu::InternalOpen(ScriptContextMenu* self, Vector2I* position, ScriptGUILayoutWrapperBase* layoutPtr)
{
	if(!self->IsNativeObjectValid())
		return;

	GUIElement* layout = layoutPtr->GetNativeObject();

	GUIWidget* widget = layout->GetParentWidget();
	if(widget == nullptr)
		return;

	GUIPhysicalArea bounds = layout->CalculateAbsoluteBounds();
	Vector2I windowPosition = *position + bounds.GetPosition().To<i32>();

	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->Open(windowPosition, *widget);
}

void ScriptContextMenu::InternalAddItem(ScriptContextMenu* self, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut)
{
	if(!self->IsNativeObjectValid())
		return;

	String nativePath = MonoUtil::MonoToString(path);

	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->AddMenuItem(nativePath, std::bind(&ScriptContextMenu::OnContextMenuItemTriggered, self, callbackIdx), 0, *shortcut);
}

void ScriptContextMenu::InternalAddSeparator(ScriptContextMenu* self, MonoString* path)
{
	if(!self->IsNativeObjectValid())
		return;

	String nativePath = MonoUtil::MonoToString(path);

	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->AddSeparator(nativePath, 0);
}

void ScriptContextMenu::InternalSetLocalizedName(ScriptContextMenu* self, MonoString* label, ScriptLocString* name)
{
	if(!self->IsNativeObjectValid())
		return;

	if(label == nullptr || name == nullptr)
		return;

	String nativeLabel = MonoUtil::MonoToString(label);
	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->SetLocalizedName(nativeLabel, *name->GetNativeObjectAsShared());
}

void ScriptContextMenu::OnContextMenuItemTriggered(u32 idx)
{
	MonoObject* scriptObject = GetScriptObject();
	MonoUtil::InvokeThunk(onEntryTriggered, scriptObject, idx);
}
