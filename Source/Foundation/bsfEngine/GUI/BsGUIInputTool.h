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
	class BS_EXPORT GUIInputLineDesc
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
		GUIInputLineDesc(UINT32 startChar, UINT32 endChar, UINT32 lineHeight, INT32 lineYStart, bool includesNewline);

		/**
		 * Returns index of the last character on the line. If lines contains a newline character it will be returned unless
		 * you set @p includeNewLine to false, in which case the next end-most character is returned. (If newline is the 
		 * only character on the line, its index will still be returned).
		 */
		UINT32 getEndChar(bool includeNewline = true) const;

		/**	Returns index of the first character on the line. */
		UINT32 getStartChar() const { return mStartChar; }

		/**	Returns line height in pixels. */
		UINT32 getLineHeight() const { return mLineHeight; }

		/**	Returns vertical offset from the top of the text to the start of this line (0 for first line usually). */
		INT32 getLineYStart() const { return mLineYStart; }

		/**
		 * Checks is the specified character index a newline. Character index is a global character index, not relative to
		 * the start character index of this line. If the index is out of range of this line character indices, it will
		 * always return false.
		 */
		bool isNewline(UINT32 charIdx) const;

		/**	Returns true if the last character on this line is a newline. */
		bool hasNewlineChar() const { return mIncludesNewline; }

	private:
		UINT32 mStartChar;
		UINT32 mEndChar;
		UINT32 mLineHeight;
		INT32 mLineYStart;
		bool mIncludesNewline;
	};

	/** Base class for input helper tools, like caret and text selection. */
	class BS_EXPORT GUIInputTool
	{
	public:
		GUIInputTool();
		~GUIInputTool();

		/**
		 * Updates the input tool with new text descriptor and parent GUI element. These values will be used for all 
		 * further calculations.
		 */
		void updateText(const GUIElement* element, const TEXT_SPRITE_DESC& textDesc);
	protected:
		/**	Returns text offset relative to parent widget. */
		Vector2I getTextOffset() const;

		/**	Returns number of lines in the current text string. */
		UINT32 getNumLines() const { return (UINT32)mLineDescs.size(); }

		/**	Returns descriptor for a line with the specified index. */
		const GUIInputLineDesc& getLineDesc(UINT32 lineIdx) const { return mLineDescs.at(lineIdx); }

		/**
		 * Returns index of a line containing the specified character.
		 *
		 * @param[in]	charIdx					Index of the character to look for.
		 * @param[in]	newlineCountsOnNextLine	If true, newline characters will return the next line and not the line
		 *										they're actually on.
		 */
		UINT32 getLineForChar(UINT32 charIdx, bool newlineCountsOnNextLine = false) const;

		/**
		 * Returns a rectangle containing position and size of the character with the provided index, relative to parent
		 * widget.
		 */
		Rect2I getCharRect(UINT32 charIdx) const;

		/**
		 * Returns a rectangle containing position and size of the character with the provided index, relative to parent 
		 * element.
		 */
		Rect2I getLocalCharRect(UINT32 charIdx) const;

		/** Returns character index nearest to the specified position. Position should be relative to parent widget. */
		INT32 getCharIdxAtPos(const Vector2I& pos) const;

		/**	Returns true if the currently set text desctiptor is valid (has any characters). */
		bool isDescValid() const;

		/**
		 * Gets a character index after the input index. Input index represents the empty areas between the characters. 
		 * Newline counts as a character. (for example 0 is before the first character, 1 is after the first character but
		 * before the second, etc.)
		 * 			
		 * @note	
		 * This can return an out of range character index, in case the input index is specified after the last character.
		 */
		UINT32 getCharIdxAtInputIdx(UINT32 inputIdx) const;

		/**	Checks is the specified character index a newline. */
		bool isNewlineChar(UINT32 charIdx) const;

		/**
		 * Checks is the character after the specified input index a newline.
		 *
		 * @see		getCharIdxAtInputIdx
		 */
		bool isNewline(UINT32 inputIdx) const;

	protected:
		const GUIElement* mElement;

		Vector2* mQuads;
		UINT32 mNumQuads;

		TEXT_SPRITE_DESC mTextDesc;
		UINT32 mNumChars = 0;

		Vector<GUIInputLineDesc> mLineDescs;
	};

	/** @} */
}