//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIUnits.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInputTool.h"
#include "2D/B3DTextSprite.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** When paired with a character index determines should the caret be placed before or after it. */
	enum CaretPos
	{
		CARET_BEFORE,
		CARET_AFTER
	};

	/** Helper class for dealing with caret for text input boxes and similar controls. */
	class B3D_EXPORT GUIInputCaret : public GUIInputTool
	{
	public:
		GUIInputCaret();
		~GUIInputCaret();

		/**	Returns sprite used for rendering the caret. */
		ImageSprite* GetSprite() const { return mCaretSprite; }

		/** Returns the bounds of the caret, relative to the current parent GUI element. */
		GUIPhysicalArea GetBounds() const;

		/**	Rebuilts internal caret sprite using current properties. */
		void UpdateSprite();

		/**	Moves caret to the start of text. */
		void MoveCaretToStart();

		/**	Moves caret to the end of text. */
		void MoveCaretToEnd();

		/**	Moves caret one character to the left, if not at start already. */
		void MoveCaretLeft();

		/**	Moves caret one character to the right, if not at end already. */
		void MoveCaretRight();

		/**	Moves caret one line up if possible. */
		void MoveCaretUp();

		/**	Moves caret one line down if possible. */
		void MoveCaretDown();

		/** Moves caret to the character nearest to the specified position. Position is relative to parent GUI element. */
		void MoveCaretToPos(const GUIPhysicalPoint& pos);

		/**
		 * Moves the caret to a specific character index.
		 *
		 * @param	characterIndex	Index of the character to move the caret to.
		 * @param	caretPos		Whether to place the caret before or after the character.
		 */
		void MoveCaretToChar(u32 characterIndex, CaretPos caretPos);

		/**	Returns character index after the current caret position. */
		u32 GetCharIdxAtCaretPos() const;

		/** Returns current caret position, relative to parent GUI element. */
		GUIPhysicalPoint GetCaretPosition() const;

		/**	Returns height of the caret, in pixels. */
		GUIPhysicalUnit GetCaretHeight() const;

		/**	Returns true if the character after the caret is newline. */
		bool IsCaretAtNewline() const;

		/**	Returns maximum valid caret index. */
		u32 GetMaxCaretPos() const;

		/**	Returns current caret index (not equal to character index). */
		u32 GetCaretPos() const { return mCaretPos; }

	private:
		u32 mCaretPos = 0;
		ImageSprite* mCaretSprite;
	};

	/** @} */
} // namespace b3d
