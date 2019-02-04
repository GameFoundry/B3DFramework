//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIMenu.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Manages display and logic for a context menu. Context menus can be opened anywhere within the GUI and can contain a
	 * hierarchy of menu items. 
	 */
	class BS_EXPORT GUIContextMenu : public GUIMenu
	{
	public:
		GUIContextMenu() = default;
		~GUIContextMenu();

		/** Opens a context menu at the specified position relative to the provided widget. */
		void open(const Vector2I& position, GUIWidget& widget);

	private:
		/**	Closes the context menu if open. */
		void close();

		/**	
		 * Called when the context menu is closed externally (for example when user selects an item or clicks outside it). 
		 */
		void onMenuClosed();

	private:
		bool mContextMenuOpen = false;
	};

	/** @} */
}