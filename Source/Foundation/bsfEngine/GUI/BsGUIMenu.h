//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIDropDownMenu.h"
#include "GUI/BsShortcutKey.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	class GUIMenuItem;

	/** Used for comparing GUI menu items in order to determine the order in which they are presented. */
	struct GUIMenuItemComparer
	{
		bool operator() (const GUIMenuItem* const& a, const GUIMenuItem* const& b) const;
	};

	/** Holds information about a single element in a GUI menu. */
	class BS_EXPORT GUIMenuItem
	{
	public:
		/**
		 * Constructs a new non-separator menu item.
		 *
		 * @param[in]	parent		Parent item, if any.
		 * @param[in]	name		Name of the item to be displayed.
		 * @param[in]	callback	Callback to be triggered when menu items is selected.
		 * @param[in]	priority	Priority that determines the order of this element compared to its siblings.
		 * @param[in]	seqIdx		Sequential index of the menu item that specifies in what order was it added to the menu
		 * 							compared to other items.
		 * @param[in]	key			Keyboard shortcut that can be used for triggering the menu item.
		 */
		GUIMenuItem(GUIMenuItem* parent, const String& name, std::function<void()> callback,
			INT32 priority, UINT32 seqIdx, const ShortcutKey& key);

		/**
		 * Constructs a new separator menu item.
		 *
		 * @param[in]	parent		Parent item, if any.
		 * @param[in]	priority	Priority that determines the order of this element compared to its siblings.
		 * @param[in]	seqIdx		Sequential index of the menu item that specifies in what order was it added to the menu
		 * 							compared to other items.
		 */
		GUIMenuItem(GUIMenuItem* parent, INT32 priority, UINT32 seqIdx);
		~GUIMenuItem();

		/**	Registers a new child with the item. */
		void addChild(GUIMenuItem* child) { mChildren.insert(child); }

		/**	Returns number of child menu items. */
		UINT32 getNumChildren() const { return (UINT32)mChildren.size(); }

		/**	Returns the parent menu item, or null if none. */
		GUIMenuItem* getParent() const { return mParent; }

		/**	Returns name of the menu item. Empty if separator. */
		const String& getName() const { return mName; }

		/**	Returns callback that will trigger when menu item is selected. Null for separators. */
		std::function<void()> getCallback() const { return mCallback; }

		/**	Returns a keyboard shortcut that may be used for triggering the menu item callback. */
		const ShortcutKey& getShortcut() const { return mShortcut; }

		/**	Checks is the menu item a separator or a normal named menu item. */
		bool isSeparator() const { return mIsSeparator; }

		/** Attempts to find a child menu item with the specified name. Only direct descendants are searched. */
		const GUIMenuItem* findChild(const String& name) const;

		/**	Removes the first child with the specified name. */
		void removeChild(const String& name);

		/**	Removes the specified child. */
		void removeChild(const GUIMenuItem* item);

	private:
		friend class GUIMenu;
		friend struct GUIMenuItemComparer;

		/** @copydoc GUIMenuItem::findChild(const String& name) const */
		GUIMenuItem* findChild(const String& name);

		GUIMenuItem* mParent;
		bool mIsSeparator;
		String mName;
		std::function<void()> mCallback;
		INT32 mPriority;
		ShortcutKey mShortcut;
		UINT32 mSeqIdx;
		Set<GUIMenuItem*, GUIMenuItemComparer> mChildren;
	};

	/**
	 * Class that allows creation of menus with drop down functionality.
	 * Menu consists out of a number of top level elements, each of which opens
	 * a drop down menu which may internally hold a deeper hierarchy of menus.
	 *
	 * @note	
	 * When specifying menu items you must provide a path. Path must be formated in a certain way. All path elements must
	 * be separated by /, for example "View/Toolbars/Find". "View" would be the top level path element, "Toolbars" a child
	 * in its menu that opens up its own submenu, and "Find" a child in the "Toolbars" sub-menu with an optional callback.
	 * @note
	 * This is an abstract class and you should provide specialized implementations for specific menu types.
	 */
	class BS_EXPORT GUIMenu
	{
	public:
		GUIMenu();
		virtual ~GUIMenu();

		/**
		 * Adds a new menu item with the specified callback.
		 *			
		 * @param[in]	path		Path that determines where to add the element. See class information on how to specify
		 *							paths. All sub-elements of a path will be added automatically.
		 * @param[in]	callback	Callback that triggers when the path element is selected.
		 * @param[in]	priority	Priority determines the position of the menu item relative to its siblings. Higher
		 *							priority means it will be placed earlier in the menu.
		 * @param[in]	key			Keyboard shortcut that can be used for triggering the menu item.
		 * @return					A menu item object that you may use for removing the menu item later. Its lifetime is
		 *							managed internally.
		 */
		GUIMenuItem* addMenuItem(const String& path, std::function<void()> callback, INT32 priority, const ShortcutKey& key = ShortcutKey::NONE);

		/**
		 * Adds a new separator menu item with the specified callback.
		 *			
		 * @param[in]	path		Path that determines where to add the element. See class information on how to specify
		 *							paths. All sub-elements of a path will be added automatically.
		 * @param[in]	priority	Priority determines the position of the menu item relative to its siblings. Higher
		 *							priority means it will be placed earlier in the menu.
		 * @return					A menu item object that you may use for removing the menu item later. Its lifetime is
		 *							managed internally.
		 */
		GUIMenuItem* addSeparator(const String& path, INT32 priority);

		/**	Returns a menu item at the specified path, or null if one is not found. */
		GUIMenuItem* getMenuItem(const String& path);

		/**	Removes the specified menu item from the path. If the menu item has any sub-menus they will also be removed. */
		void removeMenuItem(const GUIMenuItem* item);

		/**
		 * Normally menu items use values from their paths as their names. However path labels don't provide a way of
		 * localizing the menu item. This method allows you to set specific names (different from path labels) to each menu
		 * item. All the values are localized so they will also be updated according to the string table.
		 *
		 * @param[in]	menuItemLabel	The menu item label. (for example if you have a menu like "View/Toolbars/Find, this
		 *								parameter would be either "View", "Toolbars" or "Find" depending which entry you
		 *								want to localize)
		 * @param[in]	localizedName	Localized string with the name.
		 */
		void setLocalizedName(const String& menuItemLabel, const HString& localizedName);

		/**	Returns data used for initializing a drop down list, for all elements. */
		GUIDropDownData getDropDownData() const;
	protected:
		/**	Adds a menu item at the specified path, as a normal button or as a separator. */
		GUIMenuItem* addMenuItemInternal(const String& path, std::function<void()> callback, bool isSeparator,
			INT32 priority, const ShortcutKey& key);

		/**	Return drop down data for the specified menu. */
		GUIDropDownData getDropDownDataInternal(const GUIMenuItem& menu) const;

		GUIMenuItem mRootElement;
		UnorderedMap<String, HString> mLocalizedEntryNames;
		UINT32 mNextIdx;
	};

	/** @} */
}
