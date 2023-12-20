//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsTextSprite.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Represents a single line of text used by the input tools. */
	class B3D_EXPORT GUIInputLineDesc
	{
	public:
		/**
		 * Constructs a new input line description.
		 *
		 * @param[in]	startChar		Index of the first character on the line.
		 * @param[in]	endChar			Index of the last character on the line.
		 * @param[in]	lineHeight		Height of the line in pixels.
		 * @param[in]	lineYStart		Vertical offset from the top of the text to the start of this line (0 for first
		 *								line usually).
		 * @param[in]	includesNewline	True if the lines end character is a newline character.
		 */
		GUIInputLineDesc(u32 startChar, u32 endChar, u32 lineHeight, i32 lineYStart, bool includesNewline);

		/**
		 * Returns index of the last character on the line. If lines contains a newline character it will be returned unless
		 * you set @p includeNewLine to false, in which case the next end-most character is returned. (If newline is the
		 * only character on the line, its index will still be returned).
		 */
		u32 GetEndChar(bool includeNewline = true) const;

		/**	Returns index of the first character on the line. */
		u32 GetStartChar() const { return mStartChar; }

		/**	Returns line height in pixels. */
		u32 GetLineHeight() const { return mLineHeight; }

		/**	Returns vertical offset from the top of the text to the start of this line (0 for first line usually). */
		i32 GetLineYStart() const { return mLineYStart; }

		/**
		 * Checks is the specified character index a newline. Character index is a global character index, not relative to
		 * the start character index of this line. If the index is out of range of this line character indices, it will
		 * always return false.
		 */
		bool IsNewline(u32 charIdx) const;

		/**	Returns true if the last character on this line is a newline. */
		bool HasNewlineChar() const { return mIncludesNewline; }

	private:
		u32 mStartChar;
		u32 mEndChar;
		u32 mLineHeight;
		i32 mLineYStart;
		bool mIncludesNewline;
	};

	/** Base class for input helper tools, like caret and text selection. */
	class B3D_EXPORT GUIInputTool
	{
	public:
		GUIInputTool() = default;
		~GUIInputTool() = default;

		/**
		 * Updates the input tool with new text descriptor and parent GUI element. These values will be used for all
		 * further calculations.
		 */
		void UpdateText(const GUIInteractable* element, const TextSpriteInformation& textDesc);

	protected:
		/**	Returns text offset relative to parent widget. */
		Vector2I GetTextOffset() const;

		/**	Returns number of lines in the current text string. */
		u32 GetNumLines() const { return (u32)mLineDescs.size(); }

		/**	Returns descriptor for a line with the specified index. */
		const GUIInputLineDesc& GetLineDesc(u32 lineIdx) const { return mLineDescs.at(lineIdx); }

		/**
		 * Returns index of a line containing the specified character.
		 *
		 * @param[in]	charIdx					Index of the character to look for.
		 * @param[in]	newlineCountsOnNextLine	If true, newline characters will return the next line and not the line
		 *										they're actually on.
		 */
		u32 GetLineForChar(u32 charIdx, bool newlineCountsOnNextLine = false) const;

		/**
		 * Returns a rectangle containing position and size of the character with the provided index, relative to parent
		 * widget.
		 */
		Rect2I GetCharRect(u32 charIdx) const;

		/**
		 * Returns a rectangle containing position and size of the character with the provided index, relative to parent
		 * element.
		 */
		Rect2I GetLocalCharRect(u32 charIdx) const;

		/** Returns character index nearest to the specified position. Position should be relative to parent widget. */
		i32 GetCharIdxAtPos(const Vector2I& pos) const;

		/**	Returns true if the currently set text desctiptor is valid (has any characters). */
		bool IsDescValid() const;

		/**
		 * Gets a character index after the input index. Input index represents the empty areas between the characters.
		 * Newline counts as a character. (for example 0 is before the first character, 1 is after the first character but
		 * before the second, etc.)
		 *
		 * @note
		 * This can return an out of range character index, in case the input index is specified after the last character.
		 */
		u32 GetCharIdxAtInputIdx(u32 inputIdx) const;

		/**	Checks is the specified character index a newline. */
		bool IsNewlineChar(u32 charIdx) const;

		/**
		 * Checks is the character after the specified input index a newline.
		 *
		 * @see		getCharIdxAtInputIdx
		 */
		bool IsNewline(u32 inputIdx) const;

	protected:
		const GUIInteractable* mElement = nullptr;

		Vector2* mQuads = nullptr;
		u32 mNumQuads = 0;

		TextSpriteInformation mTextDesc;
		u32 mNumChars = 0;

		Vector<GUIInputLineDesc> mLineDescs;
	};

	/** @} */
} // namespace bs
