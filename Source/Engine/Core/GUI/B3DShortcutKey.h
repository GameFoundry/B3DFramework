//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** A key combination that is used for triggering keyboard shortcuts. Contains a button code and an optional modifier. */
	struct B3D_EXPORT ShortcutKey
	{
		struct B3D_EXPORT Hash
		{
			size_t operator()(const ShortcutKey& x) const;
		};

		struct B3D_EXPORT Equals
		{
			bool operator()(const ShortcutKey& a, const ShortcutKey& b) const;
		};

		ShortcutKey() = default;
		ShortcutKey(ButtonModifier modifier, ButtonCode code);

		/**	Checks is the shortcut button and modifier combination valid. */
		bool IsValid() const { return Button != ButtonCode::Unassigned; }

		/**	Returns a readable name of the shortcut key (for example "Shift + F"). */
		String GetName() const;

		ButtonModifier Modifier = ButtonModifier::None;
		ButtonCode Button = ButtonCode::Unassigned;

		static const ShortcutKey kNone;
	};

	/** @} */
} // namespace b3d
