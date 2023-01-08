//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptContextMenu.h"
#include "BsScriptMeta.h"
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

ScriptContextMenu::ScriptContextMenu(MonoObject* instance)
	: ScriptObject(instance)
{
	mGCHandle = MonoUtil::NewWeakGcHandle(instance);
	mContextMenu = B3DMakeShared<GUIContextMenu>();
}

void ScriptContextMenu::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptContextMenu::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_Open", (void*)&ScriptContextMenu::InternalOpen);
	metaData.ScriptClass->AddInternalCall("Internal_AddItem", (void*)&ScriptContextMenu::InternalAddItem);
	metaData.ScriptClass->AddInternalCall("Internal_AddSeparator", (void*)&ScriptContextMenu::InternalAddSeparator);
	metaData.ScriptClass->AddInternalCall("Internal_SetLocalizedName", (void*)&ScriptContextMenu::InternalSetLocalizedName);

	onEntryTriggered = (OnEntryTriggeredThunkDef)metaData.ScriptClass->GetMethod("InternalDoOnEntryTriggered", 1)->GetThunk();
}

void ScriptContextMenu::InternalCreateInstance(MonoObject* instance)
{
	new(B3DAllocate<ScriptContextMenu>()) ScriptContextMenu(instance);
}

void ScriptContextMenu::InternalOpen(ScriptContextMenu* instance, Vector2I* position, ScriptGUILayout* layoutPtr)
{
	GUIElementBase* layout = layoutPtr->GetGuiElement();

	GUIWidget* widget = layout->GetParentWidgetInternal();
	if(widget == nullptr)
		return;

	Rect2I bounds = layout->GetGlobalBounds();
	Vector2I windowPosition = *position + Vector2I(bounds.X, bounds.Y);

	SPtr<GUIContextMenu> contextMenu = instance->GetInternal();
	contextMenu->Open(windowPosition, *widget);
}

void ScriptContextMenu::InternalAddItem(ScriptContextMenu* instance, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut)
{
	String nativePath = MonoUtil::MonoToString(path);

	SPtr<GUIContextMenu> contextMenu = instance->GetInternal();
	contextMenu->AddMenuItem(nativePath, std::bind(&ScriptContextMenu::OnContextMenuItemTriggered, instance, callbackIdx), 0, *shortcut);
}

void ScriptContextMenu::InternalAddSeparator(ScriptContextMenu* instance, MonoString* path)
{
	String nativePath = MonoUtil::MonoToString(path);

	SPtr<GUIContextMenu> contextMenu = instance->GetInternal();
	contextMenu->AddSeparator(nativePath, 0);
}

void ScriptContextMenu::InternalSetLocalizedName(ScriptContextMenu* instance, MonoString* label, ScriptLocString* name)
{
	if(label == nullptr || name == nullptr)
		return;

	String nativeLabel = MonoUtil::MonoToString(label);
	SPtr<GUIContextMenu> contextMenu = instance->GetInternal();
	contextMenu->SetLocalizedName(nativeLabel, *name->GetInternal());
}

void ScriptContextMenu::OnContextMenuItemTriggered(u32 idx)
{
	MonoObject* instance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
	MonoUtil::InvokeThunk(onEntryTriggered, instance, idx);
}
