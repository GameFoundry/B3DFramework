//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptContextMenu.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIContextMenu.h"
#include "BsScriptHString.h"
#include "BsScriptGUILayout.h"
#include "BsGUILayout.h"

using namespace std::placeholders;

namespace bs
{
	ScriptContextMenu::OnEntryTriggeredThunkDef ScriptContextMenu::onEntryTriggered;

	ScriptContextMenu::ScriptContextMenu(MonoObject* instance)
		: ScriptObject(instance)
	{
		mContextMenu = bs_shared_ptr_new<GUIContextMenu>();
	}

	void ScriptContextMenu::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptContextMenu::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Open", &ScriptContextMenu::internal_Open);
		metaData.scriptClass->addInternalCall("Internal_AddItem", &ScriptContextMenu::internal_AddItem);
		metaData.scriptClass->addInternalCall("Internal_AddSeparator", &ScriptContextMenu::internal_AddSeparator);
		metaData.scriptClass->addInternalCall("Internal_SetLocalizedName", &ScriptContextMenu::internal_SetLocalizedName);

		onEntryTriggered = (OnEntryTriggeredThunkDef)metaData.scriptClass->getMethod("InternalDoOnEntryTriggered", 1)->getThunk();
	}

	void ScriptContextMenu::internal_CreateInstance(MonoObject* instance)
	{
		new (bs_alloc<ScriptContextMenu>()) ScriptContextMenu(instance);
	}

	void ScriptContextMenu::internal_Open(ScriptContextMenu* instance, Vector2I* position, ScriptGUILayout* layoutPtr)
	{
		GUIElementBase* layout = layoutPtr->getGUIElement();

		GUIWidget* widget = layout->_getParentWidget();
		if (widget == nullptr)
			return;

		Rect2I bounds = layout->getGlobalBounds();
		Vector2I windowPosition = *position + Vector2I(bounds.x, bounds.y);

		SPtr<GUIContextMenu> contextMenu = instance->getInternal();
		contextMenu->open(windowPosition, *widget);
	}

	void ScriptContextMenu::internal_AddItem(ScriptContextMenu* instance, MonoString* path, UINT32 callbackIdx,
		ShortcutKey* shortcut)
	{
		WString nativePath = MonoUtil::monoToWString(path);

		SPtr<GUIContextMenu> contextMenu = instance->getInternal();
		contextMenu->addMenuItem(nativePath, std::bind(&ScriptContextMenu::onContextMenuItemTriggered,
			instance, callbackIdx), 0, *shortcut);
	}

	void ScriptContextMenu::internal_AddSeparator(ScriptContextMenu* instance, MonoString* path)
	{
		WString nativePath = MonoUtil::monoToWString(path);

		SPtr<GUIContextMenu> contextMenu = instance->getInternal();
		contextMenu->addSeparator(nativePath, 0);
	}

	void ScriptContextMenu::internal_SetLocalizedName(ScriptContextMenu* instance, MonoString* label, ScriptHString* name)
	{
		if (label == nullptr || name == nullptr)
			return;

		WString nativeLabel = MonoUtil::monoToWString(label);
		SPtr<GUIContextMenu> contextMenu = instance->getInternal();
		contextMenu->setLocalizedName(nativeLabel, name->getInternalValue());
	}

	void ScriptContextMenu::onContextMenuItemTriggered(UINT32 idx)
	{
		MonoUtil::invokeThunk(onEntryTriggered, getManagedInstance(), idx);
	}
}