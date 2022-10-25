//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIContextMenu.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "GUI/BsGUIManager.h"

using namespace bs;

GUIContextMenu::~GUIContextMenu()
{
	Close();
}

void GUIContextMenu::Open(const Vector2I& position, GUIWidget& widget)
{
	DROP_DOWN_BOX_DESC desc;
	desc.Camera = widget.GetCamera();
	desc.Skin = widget.GetSkinResource();
	desc.Placement = DropDownAreaPlacement::AroundPosition(position);
	desc.DropDownData = GetDropDownData();

	GameObjectHandle<GUIDropDownMenu> dropDownBox = GUIDropDownBoxManager::Instance().OpenDropDownBox(
		desc, GUIDropDownType::ContextMenu, std::bind(&GUIContextMenu::OnMenuClosed, this));

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
