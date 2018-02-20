//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "GUI/BsShortcutKey.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/**
	 * A menu bar GUI element that contains a horizontal list of elements that can each be expanded into a hierarchical 
	 * sub-menus, as well as a list of tool bar buttons.
	 * 			
	 * Contents of the menu and tool bar are customizable.
	 *
	 * The menu bar also displays the minimize, maximize and close buttons for the window.
	 */
	class BS_ED_EXPORT GUIMenuBar
	{
		/**	Contains data about the top level menu elements. */
		struct GUIMenuBarData
		{
			WString name;
			GUIMenu* menu;
			GUIButton* button;
			GUIFixedSpace* space;
			INT32 priority;
		};

		/**	Contains data about a single tool bar element. */
		struct GUIToolBarData
		{
			String name;
			INT32 priority;
			GUIButton* button;
			GUITexture* separator;
			GUIFixedSpace* space;
		};

	public:
		/**	Returns the style type for the menu bar background. */
		static const String& getBackgroundStyleType();

		/**	Returns the style type for the menu bar line draw under the menu items. */
		static const String& getLineStyleType();

		/**	Returns the style type for the menu bar logo. */
		static const String& getLogoStyleType();

		/**	Returns the style type for the menu bar menu item buttons. */
		static const String& getMenuItemButtonStyleType();

		/**	Returns the style type for tool bar buttons. */
		static const String& getToolBarButtonStyleType();

		/**	Returns the style type for the tool bar button separator. */
		static const String& getToolBarSeparatorStyleType();

		/**
		 * Constructs a new menu bar.
		 *
		 * @param[in]	parent			Parent GUI widget the menu bar will be docked in.
		 * @param[in]	parentWindow	Window to trigger the min/max/close events on.
		 */
		GUIMenuBar(GUIWidget* parent, RenderWindow* parentWindow);
		virtual ~GUIMenuBar();

		/**	Sets the area of the menu bar, in pixels relative to the parent GUI widget. */
		void setArea(INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**
		 * Adds a new menu item to the menu bar. 
		 *
		 * @param[in]	path		Path to the menu item. Each element of the path must be separated using "/". 
		 *							First element of the path will create the top level menu, and any further element
		 *							will create a new sub-menu. Last element will be the interactable element.
		 * @param[in]	callback	Callback to trigger when user click on the interactable element (last element in the 
		 *							provided path). Can be null.
		 * @param[in]	priority	Determines where is the element positioned compared to other elements in the same 
		 *							sub-menu. Higher priority elements get placed higher up in the sub-menu. This only
		 *							applies to the last element. If you need to customize its parent element priority call
		 *							this method with with their specific paths.
		 * @param[in]	shortcut	Keyboard shortcut key to display next to the interactable element, and register with the
		 *							global shortcut manager.
		 */
		GUIMenuItem* addMenuItem(const WString& path, std::function<void()> callback, INT32 priority = 0, 
			const ShortcutKey& shortcut = ShortcutKey::NONE);

		/**
		 * Adds a menu item separator element at the specified path. The separator is added as a child of the path.
		 *
		 * @param[in]	path		Parent path of the sub-menu to add the separator.
		 * @param[in]	priority	Determines where is the separator positioned compared to other elements in the same 
		 *							sub-menu. Higher priority elements get placed higher up in the sub-menu.
		 */
		GUIMenuItem* addMenuItemSeparator(const WString& path, INT32 priority = 0);

		/**	Returns an existing menu item at the specified path, or null if one cannot be found. */
		GUIMenuItem* getMenuItem(const WString& path);

		/**
		 * Removes a menu item from the specified path. If this path points to a sub-menu entire sub-menu will be removed.
		 */
		void removeMenuItem(const WString& path);

		/**	Removes the specified menu item. */
		void removeMenuItem(GUIMenuItem* item);

		/**
		 * Adds a new button to the tool bar.
		 *
		 * @param[in]	name		Unique name of the button that can be used for identifiying it.
		 * @param[in]	content 	Content to display on the button.
		 * @param[in]	callback	Callback to trigger when the button is pressed.
		 * @param[in]	priority	Determines where is the button positioned compared to other elements on the tool bar.
		 * 							Higher priority elements get placed before lower priority ones.
		 */
		void addToolBarButton(const String& name, const GUIContent& content, std::function<void()> callback, INT32 priority = 0);

		/**
		 * Toggles an existing toolbar button into an on or off state which changes the visuals of the button.
		 *
		 * @param[in]	name	Name of the existing button to toggle.
		 * @param[in]	on		True to toggle on, false to toggle off (default).
		 */
		void toggleToolbarButton(const String& name, bool on);

		/**
		 * Adds a new separator element to the tool bar.
		 *
		 * @param[in]	name		Unique name of the separator that can be used for identifiying it.
		 * @param[in]	priority	Determines where is the separator positioned compared to other elements on the tool bar.
		 * 							Higher priority elements get placed before lower priority ones.
		 */
		void addToolBarSeparator(const String& name, INT32 priority = 0);

		/**
		 * Removes an element from the tool bar.
		 *
		 * @param[in]	name	Unique name of the element to remove.
		 */
		void removeToolBarButton(const String& name);
	private:
		/**	Finds a top level sub-menu with the specified name. */
		const GUIMenuBarData* getSubMenu(const WString& name) const;

		/**	Adds a new top level sub-menu button. */
		GUIMenuBarData* addNewButton(const WString& name, INT32 priority);

		/**
		 * Attempts to remove the first element from the specified path. First element returned in specified in @p pathRoot,
		 * and original @p path is modified so it no longer includes the first element.
		 *
		 * @return	False if first element doesn't exist, true otherwise.
		 */
		bool stripPath(WString& path, WString& pathRoot) const;

		/**
		 * Registers a shortcut with the global shortcut manager. Pressing the shortcut will trigger the provided callback.
		 */
		void registerShortcut(const WString& path, const ShortcutKey& shortcut, std::function<void()> callback);

		/**	Unregisters a shortcut assigned to the provided path from the global shortcut manager. */
		void unregisterShortcut(const WString& path);

		/**	Opens a top level sub-menu with the provided name. */
		void openSubMenu(const WString& name);

		/**	Closes any currently active sub-menu. */
		void closeSubMenu();

		/**
		 * Triggered when a sub-menu is open and a user hovers over another top level sub-menu button.
		 *
		 * @param[in]	name	Name of the sub-menu the user is hovering over.
		 */
		void onSubMenuHover(const WString& name);

		/**	Triggered when a sub-menu is closed. */
		void onSubMenuClosed();

		/**	Triggered when the minimize button is clicked. Minimizes the attached window. */
		void onMinimizeClicked();

		/**	Triggered when the maximize button is clicked. Maximizes the attached window. */
		void onMaximizeClicked();

		/**	Triggered when the close button is clicked. Closes the attached window. */
		void onCloseClicked();

		/**
		 * Refreshes the OS client area that allow the window to be dragged by dragging the empty areas on the menu bar.
		 * Should be called when top level button configuration changes or menu bar area changes.
		 */
		void refreshNonClientAreas();

		static const UINT32 NUM_ELEMENTS_AFTER_CONTENT;
		static const UINT32 ELEMENT_SPACING;

		RenderWindow* mParentWindow;
		GUIPanel* mMainPanel;
		GUIPanel* mBgPanel;
		GUILayout* mMenuItemLayout;
		GUILayout* mToolBarLayout;
		GUITexture* mBgTexture;
		GUITexture* mLogoTexture;
		GUITexture* mSplitterLine;

		GUIButton* mMinBtn;
		GUIButton* mMaxBtn;
		GUIButton* mCloseBtn;

		Vector<GUIMenuBarData> mChildMenus;
		UnorderedMap<WString, ShortcutKey> mEntryShortcuts;
		Vector<GUIToolBarData> mToolbarElements;

		GUIButton* mSubMenuButton;
		bool mSubMenuOpen;
	};

	/** @} */
}