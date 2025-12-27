//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIDropDownMenu.h"
#include "GUI/B3DShortcutKey.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	class GUIMenuItem;

	/** Used for comparing GUI menu items in order to determine the order in which they are presented. */
	struct GUIMenuItemComparer
	{
		bool operator()(const GUIMenuItem* const& a, const GUIMenuItem* const& b) const;
	};

	/** Holds information about a single element in a GUI menu. */
	class B3D_EXPORT GUIMenuItem
	{
	public:
		/**
		 * Constructs a new non-separator menu item.
		 *
		 * @param	parent				Parent item, if any.
		 * @param	name				Name of the item to be displayed.
		 * @param	callback			Callback to be triggered when menu items is selected.
		 * @param	priority			Priority that determines the order of this element compared to its siblings.
		 * @param	sequentialIndex		Sequential index of the menu item that specifies in what order was it added to the menu
		 * 								compared to other items.
		 * @param	key					Keyboard shortcut that can be used for triggering the menu item.
		 */
		GUIMenuItem(GUIMenuItem* parent, const String& name, std::function<void()> callback, i32 priority, u32 sequentialIndex, const ShortcutKey& key);

		/**
		 * Constructs a new separator menu item.
		 *
		 * @param	parent				Parent item, if any.
		 * @param	priority			Priority that determines the order of this element compared to its siblings.
		 * @param	sequentialIndex		Sequential index of the menu item that specifies in what order was it added to the menu
		 * 								compared to other items.
		 */
		GUIMenuItem(GUIMenuItem* parent, i32 priority, u32 sequentialIndex);
		~GUIMenuItem();

		/**	Registers a new child with the item. */
		void AddChild(GUIMenuItem* child) { mChildren.insert(child); }

		/**	Returns number of child menu items. */
		u32 GetNumChildren() const { return (u32)mChildren.size(); }

		/**	Returns the parent menu item, or null if none. */
		GUIMenuItem* GetParent() const { return mParent; }

		/**	Returns name of the menu item. Empty if separator. */
		const String& GetName() const { return mName; }

		/**	Returns callback that will trigger when menu item is selected. Null for separators. */
		std::function<void()> GetCallback() const { return mCallback; }

		/**	Returns a keyboard shortcut that may be used for triggering the menu item callback. */
		const ShortcutKey& GetShortcut() const { return mShortcut; }

		/**	Checks is the menu item a separator or a normal named menu item. */
		bool IsSeparator() const { return mIsSeparator; }

		/** Attempts to find a child menu item with the specified name. Only direct descendants are searched. */
		const GUIMenuItem* FindChild(const String& name) const;

		/**	Removes the first child with the specified name. */
		void RemoveChild(const String& name);

		/**	Removes the specified child. */
		void RemoveChild(const GUIMenuItem* item);

	private:
		friend class GUIMenu;
		friend struct GUIMenuItemComparer;

		/** @copydoc GUIMenuItem::FindChild(const String& name) const */
		GUIMenuItem* FindChild(const String& name);

		GUIMenuItem* mParent;
		bool mIsSeparator;
		String mName;
		std::function<void()> mCallback;
		i32 mPriority;
		ShortcutKey mShortcut;
		u32 mSeqIdx;
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
	class B3D_EXPORT GUIMenu
	{
	public:
		GUIMenu();
		virtual ~GUIMenu();

		/**
		 * Adds a new menu item with the specified callback.
		 *
		 * @param	path		Path that determines where to add the element. See class information on how to specify
		 *						paths. All sub-elements of a path will be added automatically.
		 * @param	callback	Callback that triggers when the path element is selected.
		 * @param	priority	Priority determines the position of the menu item relative to its siblings. Higher
		 *						priority means it will be placed earlier in the menu.
		 * @param	key			Keyboard shortcut that can be used for triggering the menu item.
		 * @return				A menu item object that you may use for removing the menu item later. Its lifetime is
		 *						managed internally.
		 */
		GUIMenuItem* AddMenuItem(const String& path, std::function<void()> callback, i32 priority, const ShortcutKey& key = ShortcutKey::kNone);

		/**
		 * Adds a new separator menu item with the specified callback.
		 *
		 * @param	path		Path that determines where to add the element. See class information on how to specify
		 *						paths. All sub-elements of a path will be added automatically.
		 * @param	priority	Priority determines the position of the menu item relative to its siblings. Higher
		 *						priority means it will be placed earlier in the menu.
		 * @return				A menu item object that you may use for removing the menu item later. Its lifetime is
		 *						managed internally.
		 */
		GUIMenuItem* AddSeparator(const String& path, i32 priority);

		/**	Returns a menu item at the specified path, or null if one is not found. */
		GUIMenuItem* GetMenuItem(const String& path);

		/**	Removes the specified menu item from the path. If the menu item has any sub-menus they will also be removed. */
		void RemoveMenuItem(const GUIMenuItem* item);

		/**
		 * Normally menu items use values from their paths as their names. However path labels don't provide a way of
		 * localizing the menu item. This method allows you to set specific names (different from path labels) to each menu
		 * item. All the values are localized so they will also be updated according to the string table.
		 *
		 * @param	menuItemLabel	The menu item label. (for example if you have a menu like "View/Toolbars/Find, this
		 *							parameter would be either "View", "Toolbars" or "Find" depending which entry you
		 *							want to localize)
		 * @param	localizedName	Localized string with the name.
		 */
		void SetLocalizedName(const String& menuItemLabel, const HString& localizedName);

		/**	Returns data used for initializing a drop down list, for all elements. */
		GUIDropDownData GetDropDownData() const;

	protected:
		/**	Adds a menu item at the specified path, as a normal button or as a separator. */
		GUIMenuItem* AddMenuItemInternal(const String& path, std::function<void()> callback, bool isSeparator, i32 priority, const ShortcutKey& key);

		/**	Return drop down data for the specified menu. */
		GUIDropDownData GetDropDownDataInternal(const GUIMenuItem& menu) const;

		GUIMenuItem mRootElement;
		UnorderedMap<String, HString> mLocalizedEntryNames;
		u32 mNextIdx;
	};

	/** @} */
} // namespace b3d
