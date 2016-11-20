//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsInputFwd.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** A key combination that is used for triggering keyboard shortcuts. Contains a button code and an optional modifier. */
	struct BS_EXPORT ShortcutKey
	{
		struct BS_EXPORT Hash
		{
			inline size_t operator()(const ShortcutKey& x) const;
		};

		struct BS_EXPORT Equals
		{
			inline bool operator()(const ShortcutKey& a, const ShortcutKey& b) const;
		};

		ShortcutKey();
		ShortcutKey(ButtonModifier modifier, ButtonCode code);

		/**	Checks is the shortcut button and modifier combination valid. */
		bool isValid() const { return button != BC_UNASSIGNED; }

		/**	Returns a readable name of the shortcut key (for example "Shift + F"). */
		WString getName() const;

		ButtonModifier modifier;
		ButtonCode button;

		static const ShortcutKey NONE;
	};

	/** @} */
}