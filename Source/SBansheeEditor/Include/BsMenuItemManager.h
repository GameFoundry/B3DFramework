//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**	Tracks main menu items that are registered in managed code using the MenuItem attribute. */
	class BS_SCR_BED_EXPORT MenuItemManager : public Module<MenuItemManager>
	{
	public:
		MenuItemManager(ScriptAssemblyManager& scriptObjectManager);
		~MenuItemManager();

	private:
		/**	Removes all managed menu items from the main menu. */
		void clearMenuItems();

		/**
		 * Reloads all assembly types and attempts to find uses of MenuItem. Old menu items are cleared and new are added.
		 */
		void reloadAssemblyData();

		/**
		 * Parse the provided method and detect whether it has a MenuItem attribute. If it has extract needed data from it.
		 *
		 * @param[in]	method		Managed method to parse.
		 * @param[in]	path		Output path defined in the MenuItem attribute.
		 * @param[in]	shortcut	Shortcut key defined in the MenuItem attribute.
		 * @param[in]	priority	Menu item priority defined in the MenuItem attribute.
		 * @param[in]	separator	Should the separator be inserted before the menu item.
		 * @return					True if the method has a MenuItem attribute. If false is returned output parameters
		 *							from this method are undefined.
		 */
		bool parseMenuItemMethod(MonoMethod* method, WString& path, ShortcutKey& shortcut, INT32& priority, bool& separator) const;

		/**
		 * Triggered when one of the managed menu items is clicked. 
		 *
		 * @param[in]	method	Managed method that the MenuItem is referencing.
		 */
		static void menuItemCallback(MonoMethod* method);

		ScriptAssemblyManager& mScriptObjectManager;
		HEvent mDomainLoadedConn;

		MonoClass* mMenuItemAttribute;
		MonoField* mPathField;
		MonoField* mShortcutField;
		MonoField* mPriorityField;
		MonoField* mSeparatorField;

		Vector<GUIMenuItem*> mMenuItems;
	};

	/** @} */
}