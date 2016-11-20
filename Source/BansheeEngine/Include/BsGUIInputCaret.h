//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIInputTool.h"
#include "BsTextSprite.h"

namespace bs
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
	class BS_EXPORT GUIInputCaret : public GUIInputTool
	{
	public:
		GUIInputCaret();
		~GUIInputCaret();

		/**	Returns sprite used for rendering the caret. */
		ImageSprite* getSprite() const { return mCaretSprite; }

		/** Returns offset relative to parent widget that determines placement of the caret sprite. */
		Vector2I getSpriteOffset() const;

		/**
		 * Returns clip rectangle relative to parent GUI element that determines how is caret sprite clipped.
		 *
		 * @param[in]	parentClipRect	Clip rectangle of the parent GUI element. Caret clip rectangle will additionally be
		 *								clipped by this area. Relative to parent element.
		 */
		Rect2I getSpriteClipRect(const Rect2I& parentClipRect) const;

		/**	Rebuilts internal caret sprite using current properties. */
		void updateSprite();

		/**	Moves caret to the start of text. */
		void moveCaretToStart();

		/**	Moves caret to the end of text. */
		void moveCaretToEnd();

		/**	Moves caret one character to the left, if not at start already. */
		void moveCaretLeft();

		/**	Moves caret one character to the right, if not at end already. */
		void moveCaretRight();

		/**	Moves caret one line up if possible. */
		void moveCaretUp();

		/**	Moves caret one line down if possible. */
		void moveCaretDown();

		/** Moves caret to the character nearest to the specified position. Position is relative to parent widget. */
		void moveCaretToPos(const Vector2I& pos);

		/**
		 * Moves the caret to a specific character index.
		 *
		 * @param[in]	charIdx		Index of the character to move the caret to.
		 * @param[in]	caretPos	Whether to place the caret before or after the character.
		 */
		void moveCaretToChar(UINT32 charIdx, CaretPos caretPos);

		/**	Returns character index after the current caret position. */
		UINT32 getCharIdxAtCaretPos() const;

		/**
		 * Returns current caret position, relative to parent widget. Requires you to provide offset to text the caret is
		 * used for (also relative to parent widget).
		 */
		Vector2I getCaretPosition(const Vector2I& offset) const;

		/**	Returns height of the caret, in pixels. */
		UINT32 getCaretHeight() const;

		/**	Returns true if the character after the caret is newline. */
		bool isCaretAtNewline() const;

		/**	Returns maximum valid caret index. */
		UINT32 getMaxCaretPos() const;

		/**	Returns current caret index (not equal to character index). */
		UINT32 getCaretPos() const { return mCaretPos; }
	private:
		UINT32 mCaretPos;
		ImageSprite* mCaretSprite;
	};

	/** @} */
}