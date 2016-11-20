//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsShortcutKey.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**
	 * Allows you to register global keyboard shortcuts that trigger callbacks when a certain key, or a key combination is
	 * pressed.
	 */
	class BS_EXPORT ShortcutManager : public Module<ShortcutManager>
	{
	public:
		ShortcutManager();
		~ShortcutManager();

		/**	Registers a new shortcut key and a callback to be called when the shortcut key is triggered. */
		void addShortcut(const ShortcutKey& key, std::function<void()> callback);

		/** Removes an existing shortcut key (it's callback will no longer be triggered when this combination is pressed). */
		void removeShortcut(const ShortcutKey& key);

	private:
		/**	Triggered whenever a user presses a button. */
		void onButtonDown(const ButtonEvent& event);

		UnorderedMap<ShortcutKey, std::function<void()>, ShortcutKey::Hash, ShortcutKey::Equals> mShortcuts;
		HEvent mOnButtonDownConn;
	};

	/** @} */
}