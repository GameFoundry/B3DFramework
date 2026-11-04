//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIUnits.h"
#include "B3DPrerequisites.h"
#include "2D/B3DTextSprite.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Location in a text, between two characters. */
	struct GUITextPosition
	{
		GUITextPosition() = default;
		GUITextPosition(u32 characterIndex, bool isEndOfWrappedLine)
			: CharacterIndex(characterIndex), IsEndOfWrappedLine(isEndOfWrappedLine)
		{ }

		u32 CharacterIndex = 0; /**< Index of the character after the position. Equal to the character count at the end of the text. */

		/**
		 * The end of a word-wrapped line and the start of the next line share a character index. This places such a position
		 * at the end of the wrapped line, rather than at the start of the next one. It has no effect on any other position.
		 */
		bool IsEndOfWrappedLine = false;
	};

	/**
	 * Maps between locations in a text and pixels, for text editing. All coordinates are in physical pixels relative to the
	 * top left of the text bounds. The text must be anchored to the top left, and use only the newline character as a line
	 * break.
	 */
	class B3D_EXPORT GUITextLayout
	{
	public:
		/** Rebuilds the layout, unless @p information would result in the same layout as the existing one. */
		void Update(const TextSpriteInformation& information);

		/** Releases the layout. Queries behave as for an empty text until the next Update(). */
		void Clear();

		/** Returns the number of lines in the text. */
		u32 GetLineCount() const { return (u32)mLines.size(); }

		/** Returns the number of characters in the text, including line breaks. */
		u32 GetCharacterCount() const { return mCharacterCount; }

		/** Returns the index of the line that @p position is displayed on. */
		u32 GetLineIndex(const GUITextPosition& position) const;

		/**
		 * Returns the position nearest to @p point. Points above the text resolve to the start of the text, and points below
		 * it to the end of the text.
		 */
		GUITextPosition FindPosition(const GUIPhysicalPoint& point) const;

		/** Returns the position on the line with index @p lineIndex that is nearest to the horizontal coordinate @p x. */
		GUITextPosition FindPositionOnLine(u32 lineIndex, float x) const;

		/** Returns the position one step to the left of @p position. Moves to the previous line from the start of a line. */
		GUITextPosition GetPositionLeftOf(const GUITextPosition& position) const;

		/** Returns the position one step to the right of @p position. Moves to the next line from the end of a line. */
		GUITextPosition GetPositionRightOf(const GUITextPosition& position) const;

		/** Returns the area in which to display a one pixel wide caret located at @p position. */
		GUIPhysicalArea GetCaretBounds(const GUITextPosition& position) const;

		/**
		 * Calculates the areas that cover a range of characters, with one area per line. Lines without any visible characters
		 * in the range do not produce an area.
		 *
		 * @param	startCharacterIndex		Index of the first character in the range.
		 * @param	endCharacterIndex		Index of the character after the last one in the range.
		 * @param	outAreas				Receives the areas. Existing contents are replaced.
		 */
		void GetSelectionAreas(u32 startCharacterIndex, u32 endCharacterIndex, Vector<Area2I>& outAreas) const;

	private:
		/** Describes a single line of text. */
		struct Line
		{
			Line(u32 startCharacterIndex, u32 characterCount, u32 firstEdgeIndex, i32 top, bool endsWithNewline)
				: StartCharacterIndex(startCharacterIndex), CharacterCount(characterCount), FirstEdgeIndex(firstEdgeIndex), Top(top), EndsWithNewline(endsWithNewline)
			{ }

			u32 StartCharacterIndex; /**< Index of the first character on the line. */
			u32 CharacterCount; /**< Number of characters on the line, excluding the line break. */
			u32 FirstEdgeIndex; /**< Index into mCharacterEdges of the left edge of the first character on the line. */
			i32 Top; /**< Vertical position of the top of the line. */
			bool EndsWithNewline; /**< True if the line is ended by a line break, false if by word wrap or the end of the text. */
		};

		/** Returns the height of the line with index @p lineIndex. Lines touch, so that every row in between belongs to one. */
		i32 GetLineHeight(u32 lineIndex) const;

		/** Returns true if the line with index @p lineIndex ends where a word wrap moved the text onto the next line. */
		bool IsLineWrapped(u32 lineIndex) const;

		TextSpriteInformation mInformation;

		Vector<Line> mLines;
		Vector<float> mCharacterEdges; /**< CharacterCount + 1 horizontal positions per line, in text order. */
		u32 mCharacterCount = 0;
		i32 mLastLineHeight = 0;
	};

	/** @} */
} // namespace b3d
