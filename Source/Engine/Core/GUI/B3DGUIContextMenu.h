//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIMenu.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Manages display and logic for a context menu. Context menus can be opened anywhere within the GUI and can contain a
	 * hierarchy of menu items.
	 */
	class B3D_EXPORT GUIContextMenu : public GUIMenu, public IScriptExportable
	{
	public:
		GUIContextMenu() = default;
		~GUIContextMenu();

		/** Opens a context menu at the specified position relative to the provided widget. */
		void Open(const GUIPhysicalPoint& position, GUIWidget& widget);

	private:
		/**	Closes the context menu if open. */
		void Close();

		/**
		 * Called when the context menu is closed externally (for example when user selects an item or clicks outside it).
		 */
		void OnMenuClosed();

	private:
		bool mContextMenuOpen = false;
	};

	/** @} */
} // namespace b3d
