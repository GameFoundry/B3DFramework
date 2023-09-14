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
			void Init(bool spacer);

			/**
			 * Appends a new character to the word.
			 *
			 * @param[in]	charIdx		Sequential index of the character in the original string.
			 * @param[in]	desc		Character description from the font.
			 * @return					How many pixels did the added character expand the word by.
			 */
			u32 AddChar(u32 charIdx, const CharDesc& desc);

			/** Adds a space to the word. Word must have previously have been declared as a "spacer". */
			void AddSpace(u32 spaceWidth);

			/**	Returns the width of the word in pixels. */
			u32 GetWidth() const { return mWidth; }

			/**	Returns height of the word in pixels. */
			u32 GetHeight() const { return mHeight; }

			/**
			 * Calculates new width of the word if we were to add the provided character, without actually adding it.
			 *
			 * @param[in]	desc	Character description from the font.
			 * @return				Width of the word in pixels with the character appended to it.
			 */
			u32 CalcWidthWithChar(const CharDesc& desc);

			/**
			 * Returns true if word is a spacer. Spacers contain just a space of a certain length with no actual characters.
			 */
			bool IsSpacer() const { return mSpacer; }

			/**	Returns the number of characters in the word. */
			u32 GetNumChars() const { return mLastChar == nullptr ? 0 : (mCharsEnd - mCharsStart + 1); }

			/**	Returns the index of the starting character in the word. */
			u32 GetCharsStart() const { return mCharsStart; }

			/**	Returns the index of the last character in the word. */
			u32 GetCharsEnd() const { return mCharsEnd; }

			/**
			 * Calculates width of the character by which it would expand the width of the word if it was added to it.
			 *
			 * @param[in]	prevDesc	Descriptor of the character preceding the one we need the width for. Can be null.
			 * @param[in]	desc		Character description from the font.
			 * @return 					How many pixels would the added character expand the word by.
			 */
			static u32 CalcCharWidth(const CharDesc* prevDesc, const CharDesc& desc);

		private:
			u32 mCharsStart, mCharsEnd;
			u32 mWidth;
			u32 mHeight;

			const CharDesc* mLastChar;

			bool mSpacer;
			u32 mSpaceWidth;
		};

		/**
		 * Contains information about a single texture page that contains rendered character data.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		struct PageInfo
		{
			u32 NumQuads;
			HTexture Texture;
		};

	public:
		/**
		 * Represents a single line as a set of words.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		class B3D_CORE_EXPORT TextLine
		{
		public:
			/**	Returns width of the line in pixels. */
			u32 GetWidth() const { return mWidth; }

			/**	Returns height of the line in pixels. */
			u32 GetHeight() const { return mHeight; }

			/**	Returns an offset used to separate two lines. */
			u32 GetYOffset() const { return mTextData->GetLineHeight(); }

			/**
			 * Calculates new width of the line if we were to add the provided character, without actually adding it.
			 *
			 * @param[in]	desc	Character description from the font.
			 * @return				Width of the line in pixels with the character appended to it.
			 */
			u32 CalcWidthWithChar(const CharDesc& desc);

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
			u32 FillBuffer(u32 page, Vector2* vertices, Vector2* uvs, u32* indexes, u32 offset, u32 size) const;

			/**	Checks are we at a word boundary (meaning the next added character will start a new word). */
			bool IsAtWordBoundary() const;

			/**	Returns the total number of characters on this line. */
			u32 GetNumChars() const;

			/**
			 * Query if this line was created explicitly due to a newline character. As opposed to a line that was created
			 * because a word couldn't fit on the previous line.
			 */
			bool HasNewlineChar() const { return mHasNewline; }

		private:
			friend class TextDataBase;

			/**
			 * Appends a new character to the line.
			 *
			 * @param[in]	charIdx		Sequential index of the character in the original string.
			 * @param[in]	charDesc	Character description from the font.
			 */
			void Add(u32 charIdx, const CharDesc& charDesc);

			/**	Appends a space to the line. */
			void AddSpace(u32 spaceWidth);

			/**
			 * Adds a new word to the line.
			 *
			 * @param[in]	wordIdx		Sequential index of the word in the original string. Spaces are counted as words as
			 *							well.
			 * @param[in]	word		Description of the word.
			 */
			void AddWord(u32 wordIdx, const TextWord& word);

			/** Initializes the line. Must be called after construction. */
			void Init(TextDataBase* textData);

			/**
			 * Finalizes the line. Do not add new characters/words after a line has been finalized.
			 *
			 * @param[in]	hasNewlineChar	Set to true if line was create due to an explicit newline char. As opposed to a
			 *								line that was created because a word couldn't fit on the previous line.
			 */
			void Finalize(bool hasNewlineChar);

			/**	Returns true if the line contains no words. */
			bool IsEmpty() const { return mIsEmpty; }

			/**	Removes last word from the line and returns its sequential index. */
			u32 RemoveLastWord();

			/**	Calculates the line width and height in pixels. */
			void CalculateBounds();

		private:
			TextDataBase* mTextData;
			u32 mWordsStart, mWordsEnd;

			u32 mWidth;
			u32 mHeight;

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
		B3D_CORE_EXPORT TextDataBase(const U32String& text, const HFont& font, u32 fontSize, u32 width = 0, u32 height = 0, bool wordWrap = false, bool wordBreak = true);
		B3D_CORE_EXPORT virtual ~TextDataBase() = default;

		/**	Returns the number of lines that were generated. */
		B3D_CORE_EXPORT u32 GetNumLines() const { return mNumLines; }

		/**	Returns the number of font pages references by the used characters. */
		B3D_CORE_EXPORT u32 GetNumPages() const { return mNumPageInfos; }

		/**	Returns the height of a line in pixels. */
		B3D_CORE_EXPORT u32 GetLineHeight() const;

		/**	Gets information describing a single line at the specified index. */
		B3D_CORE_EXPORT const TextLine& GetLine(u32 idx) const { return mLines[idx]; }

		/**	Returns font texture for the provided page index.  */
		B3D_CORE_EXPORT const HTexture& GetTextureForPage(u32 page) const;

		/**	Returns the number of quads used by all the characters in the provided page. */
		B3D_CORE_EXPORT u32 GetNumQuadsForPage(u32 page) const { return mPageInfos[page].NumQuads; }

		/**	Returns the width of the actual text in pixels. */
		B3D_CORE_EXPORT u32 GetWidth() const;

		/**	Returns the height of the actual text in pixels. */
		B3D_CORE_EXPORT u32 GetHeight() const;

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
		B3D_CORE_EXPORT void GeneratePersistentData(const U32String& text, u8* buffer, u32& size, bool freeTemporary = true);

	private:
		friend class TextLine;

		/**	Returns Y offset that determines the line on which the characters are placed. In pixels. */
		i32 GetBaselineOffset() const;

		/**	Returns the width of a single space in pixels. */
		u32 GetSpaceWidth() const;

		/** Gets a description of a single character referenced by its sequential index based on the original string. */
		const CharDesc& GetChar(u32 idx) const { return *mChars[idx]; }

		/** Gets a description of a single word referenced by its sequential index based on the original string. */
		const TextWord& GetWord(u32 idx) const { return mWords[idx]; }

	protected:
		const CharDesc** mChars;
		u32 mNumChars;

		TextWord* mWords;
		u32 mNumWords;

		TextLine* mLines;
		u32 mNumLines;

		PageInfo* mPageInfos;
		u32 mNumPageInfos;

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
			u32 AllocWord(bool spacer);

			/** Allocates a new line and adds it to the buffer. Returns index of the line in the line buffer. */
			u32 AllocLine(TextDataBase* textData);

			/**
			 * Increments the count of characters for the referenced page, and optionally creates page info if it doesn't
			 * already exist.
			 */
			void AddCharToPage(u32 page, const FontBitmap& fontData);

			/**	Resets all allocation counters, but doesn't actually release memory. */
			void DeallocAll();

			TextWord* WordBuffer;
			u32 WordBufferSize;
			u32 NextFreeWord;

			TextLine* LineBuffer;
			u32 LineBufferSize;
			u32 NextFreeLine;

			PageInfo* PageBuffer;
			u32 PageBufferSize;
			u32 NextFreePageInfo;
		};

		static B3D_THREADLOCAL BufferData* MemBuffer;

		/**	Allocates an initial set of buffers that will be reused while parsing text data. */
		static void InitAlloc();
	};

	/** @} */

	/** @addtogroup Text
	 *  @{
	 */

	/** @copydoc TextDataBase */
	template <class AllocatorTag = DefaultAllocatorTag>
	class TextData : public TextDataBase
	{
	public:
		/** @copydoc TextDataBase::TextDataBase */
		TextData(const U32String& text, const HFont& font, u32 fontSize, u32 width = 0, u32 height = 0, bool wordWrap = false, bool wordBreak = true)
			: TextDataBase(text, font, fontSize, width, height, wordWrap, wordBreak), mData(nullptr)
		{
			u32 totalBufferSize = 0;
			GeneratePersistentData(text, nullptr, totalBufferSize);

			mData = (u8*)B3DAllocate<AllocatorTag>(totalBufferSize);
			GeneratePersistentData(text, (u8*)mData, totalBufferSize);
		}

		~TextData()
		{
			if(mData != nullptr)
				B3DFree<AllocatorTag>(mData);
		}

	private:
		u8* mData;
	};

	/** @} */
} // namespace bs
