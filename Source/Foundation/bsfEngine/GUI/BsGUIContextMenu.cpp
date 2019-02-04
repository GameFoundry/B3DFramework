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

	void GUIContextMenu::open(const Vector2I& position, GUIWidget& widget)
	{
		DROP_DOWN_BOX_DESC desc;
		desc.camera = widget.getCamera();
		desc.skin = widget.getSkinResource();
		desc.placement = DropDownAreaPlacement::aroundPosition(position);
		desc.dropDownData = getDropDownData();

		GameObjectHandle<GUIDropDownMenu> dropDownBox = GUIDropDownBoxManager::instance().openDropDownBox(
			desc, GUIDropDownType::ContextMenu, std::bind(&GUIContextMenu::onMenuClosed, this));

		mContextMenuOpen = true;
	}

	void GUIContextMenu::close()
	{
		if(mContextMenuOpen)
		{
			GUIDropDownBoxManager::instance().closeDropDownBox();
			mContextMenuOpen = false;
		}
	}

	void GUIContextMenu::onMenuClosed()
	{
		mContextMenuOpen = false;
	}
}