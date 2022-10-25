//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "GUI/BsShortcutKey.h"

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
		void AddShortcut(const ShortcutKey& key, std::function<void()> callback);

		/** Removes an existing shortcut key (it's callback will no longer be triggered when this combination is pressed). */
		void RemoveShortcut(const ShortcutKey& key);

	private:
		/**	Triggered whenever a user presses a button. */
		void OnButtonDown(const ButtonEvent& event);

		UnorderedMap<ShortcutKey, std::function<void()>, ShortcutKey::Hash, ShortcutKey::Equals> mShortcuts;
		HEvent mOnButtonDownConn;
	};

	/** @} */
} // namespace bs
