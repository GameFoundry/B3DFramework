//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Input/BsInputFwd.h"

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
			size_t operator()(const ShortcutKey& x) const;
		};

		struct BS_EXPORT Equals
		{
			bool operator()(const ShortcutKey& a, const ShortcutKey& b) const;
		};

		ShortcutKey();
		ShortcutKey(ButtonModifier modifier, ButtonCode code);

		/**	Checks is the shortcut button and modifier combination valid. */
		bool isValid() const { return button != BC_UNASSIGNED; }

		/**	Returns a readable name of the shortcut key (for example "Shift + F"). */
		String getName() const;

		ButtonModifier modifier;
		ButtonCode button;

		static const ShortcutKey NONE;
	};

	/** @} */
}