//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIContextMenu.h"
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "GUI/B3DGUIManager.h"

using namespace b3d;

GUIContextMenu::~GUIContextMenu()
{
	Close();
}

void GUIContextMenu::Open(const GUIPhysicalPoint& position, GUIWidget& widget)
{
	DropDownBoxCreateInformation desc;
	desc.Camera = widget.GetCamera();
	desc.StyleSheetCascade = widget.GetStyleSheetCascadeAsShared();
	desc.Placement = TDropDownAreaPlacement<GUIPhysicalUnit>::AroundPosition(position);
	desc.DropDownData = GetDropDownData();

	TGameObjectHandle<GUIDropDownMenu> dropDownBox = GUIDropDownBoxManager::Instance().OpenDropDownBox(
		desc, GUIDropDownType::ContextMenu, [this]() { OnMenuClosed(); });

	mContextMenuOpen = true;
}

void GUIContextMenu::Close()
{
	if(mContextMenuOpen)
	{
		GUIDropDownBoxManager::Instance().CloseDropDownBox();
		mContextMenuOpen = false;
	}
}

void GUIContextMenu::OnMenuClosed()
{
	mContextMenuOpen = false;
}
