//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptContextMenu.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "B3DMonoMethod.h"
#include "B3DMonoUtil.h"
#include "B3DScriptTVector2.generated.h"
#include "GUI/B3DGUIContextMenu.h"
#include "GUI/B3DGUILayout.h"

#include "Generated/B3DScriptHString.generated.h"
#include "Generated/B3DScriptGUILayout.generated.h"

using namespace b3d;
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

void ScriptContextMenu::InternalOpen(ScriptContextMenu* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position, ScriptGUILayoutWrapperBase* layoutPtr)
{
	if(!self->IsNativeObjectValid())
		return;

	GUIElement* layout = layoutPtr->GetNativeObject();

	GUIWidget* widget = layout->GetParentWidget();
	if(widget == nullptr)
		return;

	GUIPhysicalArea bounds = layout->CalculateAbsoluteBounds();
	GUIPhysicalPoint windowPosition = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(*position) + bounds.GetPosition();

	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->Open(windowPosition, *widget);
}

void ScriptContextMenu::InternalAddItem(ScriptContextMenu* self, MonoString* path, u32 callbackIdx, ShortcutKey* shortcut)
{
	if(!self->IsNativeObjectValid())
		return;

	String nativePath = MonoUtil::MonoToString(path);

	SPtr<GUIContextMenu> contextMenu = self->GetNativeObjectAsShared();
	contextMenu->AddMenuItem(nativePath, [self, callbackIdx]() { self->OnContextMenuItemTriggered(callbackIdx); }, 0, *shortcut);
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
