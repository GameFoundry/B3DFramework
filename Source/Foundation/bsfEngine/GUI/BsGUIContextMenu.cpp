//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIContextMenu.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "GUI/BsGUIManager.h"

namespace bs
{
	GUIContextMenu::~GUIContextMenu()
	{
		close();
	}

	void GUIContextMenu::Open(const Vector2I& position, GUIWidget& widget)
	{
		DROP_DOWN_BOX_DESC desc;
		desc.camera = widget.GetCamera();
		desc.skin = widget.GetSkinResource();
		desc.placement = DropDownAreaPlacement::aroundPosition(position);
		desc.dropDownData = getDropDownData();

		GameObjectHandle<GUIDropDownMenu> dropDownBox = GUIDropDownBoxManager::instance().openDropDownBox(
			desc, GUIDropDownType::ContextMenu, std::Bind(&GUIContextMenu::onMenuClosed, this));

		mContextMenuOpen = true;
	}

	void GUIContextMenu::Close()
	{
		if(mContextMenuOpen)
		{
			GUIDropDownBoxManager::instance().CloseDropDownBox();
			mContextMenuOpen = false;
		}
	}

	void GUIContextMenu::OnMenuClosed()
	{
		mContextMenuOpen = false;
	}
}
