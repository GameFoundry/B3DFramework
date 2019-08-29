//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Text/BsFontDesc.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * This object takes as input a string, a font and optionally some constraints (like word wrap) and outputs a set of
	 * character data you may use for rendering or metrics.
	 */
	class TextDataBase
	{
	protected:
		/**
		 * Represents a single word as a set of characters, or optionally just a blank space of a certain length.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		class TextWord
		{
		public:
			/**
			 * Initializes the word and signals if it just a space (or multiple spaces), or an actual word with letters.
			 */
			void init(bool spacer);

			/**
			 * Appends a new character to the word.
			 *
			 * @param[in]	charIdx		Sequential index of the character in the original string.
			 * @param[in]	desc		Character description from the font.
			 * @return					How many pixels did the added character expand the word by.
			 */
			UINT32 addChar(UINT32 charIdx, const CharDesc& desc);

			/** Adds a space to the word. Word must have previously have been declared as a "spacer". */
			void addSpace(UINT32 spaceWidth);

			/**	Returns the width of the word in pixels. */
			UINT32 getWidth() const { return mWidth; }

			/**	Returns height of the word in pixels. */
			UINT32 getHeight() const { return mHeight; }

			/**
			 * Calculates new width of the word if we were to add the provided character, without actually adding it.
			 *
			 * @param[in]	desc	Character description from the font.
			 * @return				Width of the word in pixels with the character appended to it.
			 */
			UINT32 calcWidthWithChar(const CharDesc& desc);

			/**
			 * Returns true if word is a spacer. Spacers contain just a space of a certain length with no actual characters.
			 */
			bool isSpacer() const { return mSpacer; }

			/**	Returns the number of characters in the word. */
			UINT32 getNumChars() const { return mLastChar == nullptr ? 0 : (mCharsEnd - mCharsStart + 1); }

			/**	Returns the index of the starting character in the word. */
			UINT32 getCharsStart() const { return mCharsStart; }

			/**	Returns the index of the last character in the word. */
			UINT32 getCharsEnd() const { return mCharsEnd; }

			/**
			 * Calculates width of the character by which it would expand the width of the word if it was added to it.
			 *
			 * @param[in]	prevDesc	Descriptor of the character preceding the one we need the width for. Can be null.
			 * @param[in]	desc		Character description from the font.
			 * @return 					How many pixels would the added character expand the word by.
			 */
			static UINT32 calcCharWidth(const CharDesc* prevDesc, const CharDesc& desc);

		private:
			UINT32 mCharsStart, mCharsEnd;
			UINT32 mWidth;
			UINT32 mHeight;

			const CharDesc* mLastChar;

			bool mSpacer;
			UINT32 mSpaceWidth;
		};

		/**
		 * Contains information about a single texture page that contains rendered character data.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		struct PageInfo
		{
			UINT32 numQuads;
			HTexture texture;
		};

	public:
		/**
		 * Represents a single line as a set of words.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		class BS_CORE_EXPORT TextLine
		{
		public:
			/**	Returns width of the line in pixels. */
			UINT32 getWidth() const { return mWidth; }

			/**	Returns height of the line in pixels. */
			UINT32 getHeight() const { return mHeight; }

			/**	Returns an offset used to separate two lines. */
			UINT32 getYOffset() const { return mTextData->getLineHeight(); }

			/**
			 * Calculates new width of the line if we were to add the provided character, without actually adding it.
			 *
			 * @param[in]	desc	Character description from the font.
			 * @return				Width of the line in pixels with the character appended to it.
			 */
			UINT32 calcWidthWithChar(const CharDesc& desc);

			/**
			 * Fills the vertex/uv/index buffers for the specified page, with all the character data needed for rendering.
			 *
			 * @param[in]	page		The page.
			 * @param[out]	vertices	Pre-allocated array where character vertices will be written.
			 * @param[out]	uvs			Pre-allocated array where character uv coordinates will be written.
			 * @param[out]	indexes 	Pre-allocated array where character indices will be written.
			 * @param[in]	offset		Offsets the location at which the method writes to the buffers. Counted as number
			 *							of quads.
			 * @param[in]	size		Total number of quads that can fit into the specified buffers.
			 * @return					Number of quads that were written.
			 */
			UINT32 fillBuffer(UINT32 page, Vector2* vertices, Vector2* uvs, UINT32* indexes, UINT32 offset, UINT32 size) const;

			/**	Checks are we at a word boundary (meaning the next added character will start a new word). */
			bool isAtWordBoundary() const;

			/**	Returns the total number of characters on this line. */
			UINT32 getNumChars() const;

			/**
			 * Query if this line was created explicitly due to a newline character. As opposed to a line that was created
			 * because a word couldn't fit on the previous line.
			 */
			bool hasNewlineChar() const { return mHasNewline; }
		private:
			friend class TextDataBase;

			/**
			 * Appends a new character to the line.
			 *
			 * @param[in]	charIdx		Sequential index of the character in the original string.
			 * @param[in]	charDesc	Character description from the font.
			 */
			void add(UINT32 charIdx, const CharDesc& charDesc);

			/**	Appends a space to the line. */
			void addSpace(UINT32 spaceWidth);

			/**
			 * Adds a new word to the line.
			 *
			 * @param[in]	wordIdx		Sequential index of the word in the original string. Spaces are counted as words as
			 *							well.
			 * @param[in]	word		Description of the word.
			 */
			void addWord(UINT32 wordIdx, const TextWord& word);

			/** Initializes the line. Must be called after construction. */
			void init(TextDataBase* textData);

			/**
			 * Finalizes the line. Do not add new characters/words after a line has been finalized.
			 *
			 * @param[in]	hasNewlineChar	Set to true if line was create due to an explicit newline char. As opposed to a
			 *								line that was created because a word couldn't fit on the previous line.
			 */
			void finalize(bool hasNewlineChar);

			/**	Returns true if the line contains no words. */
			bool isEmpty() const { return mIsEmpty; }

			/**	Removes last word from the line and returns its sequential index. */
			UINT32 removeLastWord();

			/**	Calculates the line width and height in pixels. */
			void calculateBounds();

		private:
			TextDataBase* mTextData;
			UINT32 mWordsStart, mWordsEnd;

			UINT32 mWidth;
			UINT32 mHeight;

			bool mIsEmpty;
			bool mHasNewline;
		};

	public:
		/**
		 * Initializes a new text data using the specified string and font. Text will attempt to fit into the provided area.
		 * If enabled it will wrap words to new line when they don't fit. Individual words will be broken into multiple
		 * pieces if they don't fit on a single line when word break is enabled, otherwise they will be clipped. If the
		 * specified area is zero size then the text will not be clipped or word wrapped in any way.
		 *
		 * After this object is constructed you may call various getter methods to get needed information.
		 */
		BS_CORE_EXPORT TextDataBase(const U32String& text, const HFont& font, UINT32 fontSize,
			UINT32 width = 0, UINT32 height = 0, bool wordWrap = false, bool wordBreak = true);
		BS_CORE_EXPORT virtual ~TextDataBase() = default;

		/**	Returns the number of lines that were generated. */
		BS_CORE_EXPORT UINT32 getNumLines() const { return mNumLines; }

		/**	Returns the number of font pages references by the used characters. */
		BS_CORE_EXPORT UINT32 getNumPages() const { return mNumPageInfos; }

		/**	Returns the height of a line in pixels. */
		BS_CORE_EXPORT UINT32 getLineHeight() const;

		/**	Gets information describing a single line at the specified index. */
		BS_CORE_EXPORT const TextLine& getLine(UINT32 idx) const { return mLines[idx]; }

		/**	Returns font texture for the provided page index.  */
		BS_CORE_EXPORT const HTexture& getTextureForPage(UINT32 page) const;

		/**	Returns the number of quads used by all the characters in the provided page. */
		BS_CORE_EXPORT UINT32 getNumQuadsForPage(UINT32 page) const { return mPageInfos[page].numQuads; }

		/**	Returns the width of the actual text in pixels. */
		BS_CORE_EXPORT UINT32 getWidth() const;

		/**	Returns the height of the actual text in pixels. */
		BS_CORE_EXPORT UINT32 getHeight() const;

	protected:
		/**
		 * Copies internally stored data in temporary buffers to a persistent buffer.
		 *
		 * @param[in]	text			Text originally used for creating the internal temporary buffer data.
		 * @param[in]	buffer			Memory location to copy the data to. If null then no data will be copied and the
		 *								parameter @p size will contain the required buffer size.
		 * @param[in]	size			Size of the provided memory buffer, or if the buffer is null, this will contain the
		 *								required buffer size after method exists.
		 * @param[in]	freeTemporary	If true the internal temporary data will be freed after copying.
		 *
		 * @note	Must be called after text data has been constructed and is in the temporary buffers.
		 */
		BS_CORE_EXPORT void generatePersistentData(const U32String& text, UINT8* buffer, UINT32& size,
			bool freeTemporary = true);
	private:
		friend class TextLine;

		/**	Returns Y offset that determines the line on which the characters are placed. In pixels. */
		INT32 getBaselineOffset() const;

		/**	Returns the width of a single space in pixels. */
		UINT32 getSpaceWidth() const;

		/** Gets a description of a single character referenced by its sequential index based on the original string. */
		const CharDesc& getChar(UINT32 idx) const { return *mChars[idx]; }

		/** Gets a description of a single word referenced by its sequential index based on the original string. */
		const TextWord& getWord(UINT32 idx) const { return mWords[idx]; }

	protected:
		const CharDesc** mChars;
		UINT32 mNumChars;

		TextWord* mWords;
		UINT32 mNumWords;

		TextLine* mLines;
		UINT32 mNumLines;

		PageInfo* mPageInfos;
		UINT32 mNumPageInfos;

		HFont mFont;
		SPtr<const FontBitmap> mFontData;

		// Static buffers used to reduce runtime memory allocation
	protected:
		/** Stores per-thread memory buffers used to reduce memory allocation. */
		// Note: I could replace this with the global frame allocator to avoid the extra logic
		struct BufferData
		{
			BufferData();
			~BufferData();

			/**
			 * Allocates a new word and adds it to the buffer. Returns index of the word in the word buffer.
			 *
			 * @param[in]	spacer	Specify true if the word is only to contain spaces. (Spaces are considered a special
			 *						type of word).
			 */
			UINT32 allocWord(bool spacer);

			/** Allocates a new line and adds it to the buffer. Returns index of the line in the line buffer. */
			UINT32 allocLine(TextDataBase* textData);

			/**
			 * Increments the count of characters for the referenced page, and optionally creates page info if it doesn't
			 * already exist.
			 */
			void addCharToPage(UINT32 page, const FontBitmap& fontData);

			/**	Resets all allocation counters, but doesn't actually release memory. */
			void deallocAll();

			TextWord* WordBuffer;
			UINT32 WordBufferSize;
			UINT32 NextFreeWord;

			TextLine* LineBuffer;
			UINT32 LineBufferSize;
			UINT32 NextFreeLine;

			PageInfo* PageBuffer;
			UINT32 PageBufferSize;
			UINT32 NextFreePageInfo;
		};

		static BS_THREADLOCAL BufferData* MemBuffer;

		/**	Allocates an initial set of buffers that will be reused while parsing text data. */
		static void initAlloc();
	};

	/** @} */

	/** @addtogroup Text
	 *  @{
	 */

	/** @copydoc TextDataBase */
	template<class Alloc = GenAlloc>
	class TextData : public TextDataBase
	{
	public:
		/** @copydoc TextDataBase::TextDataBase */
		TextData(const U32String& text, const HFont& font, UINT32 fontSize,
			UINT32 width = 0, UINT32 height = 0, bool wordWrap = false, bool wordBreak = true)
			:TextDataBase(text, font, fontSize, width, height, wordWrap, wordBreak), mData(nullptr)
		{
			UINT32 totalBufferSize = 0;
			generatePersistentData(text, nullptr, totalBufferSize);

			mData = (UINT8*)bs_alloc<Alloc>(totalBufferSize);
			generatePersistentData(text, (UINT8*)mData, totalBufferSize);
		}

		~TextData()
		{
			if (mData != nullptr)
				bs_free<Alloc>(mData);
		}

	private:
		UINT8* mData;
	};

	/** @} */
}
