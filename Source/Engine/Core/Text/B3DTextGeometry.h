//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	struct CharacterInformation;

	/** @addtogroup Text-Internal
	 *  @{
	 */

	/**
	 * This object takes as input a string, a font and optionally some constraints (like word wrap) and outputs a set of
	 * character data you may use for rendering or calculating dimensions.
	 */
	class TextGeometry
	{
		/**
		 * Contains information about a single texture page that contains rendered character data.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		struct PageInfo
		{
			u32 QuadCount = 0;
			HTexture Texture;
		};

	protected:
		/**
		 * Represents a single word as a set of characters, or optionally just a blank space of a certain length.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		class Word
		{
		public:
			/** Initializes the word and signals if it just a space (or multiple spaces), or an actual word with letters. */
			void Initialize(bool isSpacer);

			/**
			 * Appends a new character to the word.
			 *
			 * @param	characterIndex			Sequential index of the character in the original string.
			 * @param	characterInformation	Character description from the font.
			 * @return							How many pixels did the added character expand the word by.
			 */
			float AddCharacter(u32 characterIndex, const CharacterInformation& characterInformation);

			/** Adds a space to the word. Word must have previously have been declared as a "spacer". */
			void AddSpace(float spaceWidth);

			/**	Returns the width of the word in pixels. */
			float GetWidth() const { return mWidth; }

			/**	Returns height of the word in pixels. */
			float GetHeight() const { return mHeight; }

			/**
			 * Calculates new width of the word if we were to add the provided character, without actually adding it.
			 *
			 * @param	characterInformation	Character description from the font.
			 * @return							Width of the word in pixels with the character appended to it.
			 */
			float CalculateWidthWithCharacter(const CharacterInformation& characterInformation) const;

			/**
			 * Returns true if word is a spacer. Spacers contain just a space of a certain length with no actual characters.
			 */
			bool IsSpacer() const { return mIsSpacer; }

			/**	Returns the number of characters in the word. */
			u32 GetCharacterCount() const { return mLastCharacter == nullptr ? 0 : (mCharacterEndIndex - mCharacterStartIndex + 1); }

			/**	Returns the index of the starting character in the word. */
			u32 GetStartCharacterIndex() const { return mCharacterStartIndex; }

			/**	Returns the index of the last character in the word. */
			u32 GetEndCharacterIndex() const { return mCharacterEndIndex; }

			/**
			 * Calculates width of the character by which it would expand the width of the word if it was added to it.
			 *
			 * @param	previousCharacter	Descriptor of the character preceding the one we need the width for. Can be null.
			 * @param	currentCharacter	Character description from the font.
			 * @return 						How many pixels would the added character expand the word by.
			 */
			static float CalculateCharacterWidth(const CharacterInformation* previousCharacter, const CharacterInformation& currentCharacter);

		private:
			u32 mCharacterStartIndex = 0;
			u32 mCharacterEndIndex = 0;
			float mWidth = 0.0f;
			float mHeight = 0.0f;

			const CharacterInformation* mLastCharacter = nullptr;

			bool mIsSpacer = false;
			float mSpaceWidth = 0.0f;
		};

	public:
		/**
		 * Represents a single line as a set of words.
		 *
		 * @note	Due to the way allocation is handled, this class is not allowed to have a destructor.
		 */
		class B3D_EXPORT Line
		{
		public:
			/**	Returns width of the line in pixels. */
			float GetWidth() const { return mWidth; }

			/**	Returns height of the line in pixels. */
			float GetHeight() const { return mHeight; }

			/**	Returns an offset used to separate two lines. */
			float GetYOffset() const { return mTextData->GetLineHeight(); }

			/**
			 * Calculates new width of the line if we were to add the provided character, without actually adding it.
			 *
			 * @param	characterInformation	Character description from the font.
			 * @return							Width of the line in pixels with the character appended to it.
			 */
			float CalculateWidthWithChararacter(const CharacterInformation& characterInformation) const;

			/**
			 * Fills the vertex/uv/index buffers for the specified page, with all the character data needed for rendering.
			 *
			 * @param	page		The page.
			 * @param	outVertices	Pre-allocated array where character vertices will be written.
			 * @param	outUVs		Pre-allocated array where character uv coordinates will be written.
			 * @param	outIndices 	Pre-allocated array where character indices will be written.
			 * @param	offset		Offsets the location at which the method writes to the buffers. Counted as number
			 *						of quads.
			 * @param	size		Total number of quads that can fit into the specified buffers.
			 * @return				Number of quads that were written.
			 */
			u32 FillBuffer(u32 page, Vector2* outVertices, Vector2* outUVs, u32* outIndices, u32 offset, u32 size) const;

			/**	Checks are we at a word boundary (meaning the next added character will start a new word). */
			bool IsAtWordBoundary() const;

			/**	Returns the total number of characters on this line. */
			u32 GetCharacterCount() const;

			/**
			 * Query if this line was created explicitly due to a newline character. As opposed to a line that was created
			 * because a word couldn't fit on the previous line.
			 */
			bool HasNewlineChar() const { return mHasNewline; }

		private:
			friend class TextGeometry;

			/**
			 * Appends a new character to the line.
			 *
			 * @param	characterIndex			Sequential index of the character in the original string.
			 * @param	characterInformation	Character description from the font.
			 */
			void Add(u32 characterIndex, const CharacterInformation& characterInformation);

			/**	Appends a space to the line. */
			void AddSpace(float spaceWidth);

			/**
			 * Adds a new word to the line.
			 *
			 * @param	wordIndex		Sequential index of the word in the original string. Spaces are counted as words as well.
			 * @param	word		Description of the word.
			 */
			void AddWord(u32 wordIndex, const Word& word);

			/** Initializes the line. Must be called after construction. */
			void Initialize(TextGeometry* textData);

			/**
			 * Finalizes the line. Do not add new characters/words after a line has been finalized.
			 *
			 * @param	hasNewlineChar	Set to true if line was create due to an explicit newline char. As opposed to a
			 *							line that was created because a word couldn't fit on the previous line.
			 */
			void Finalize(bool hasNewlineChar);

			/**	Returns true if the line contains no words. */
			bool IsEmpty() const { return mIsEmpty; }

			/**	Removes last word from the line and returns its sequential index. */
			u32 RemoveLastWord();

			/**	Calculates the line width and height in pixels. */
			void CalculateBounds();

		private:
			TextGeometry* mTextData = nullptr;
			u32 mWordStartIndex = 0;
			u32 mWordEndIndex = 0;

			float mWidth = 0.0f;
			float mHeight = 0.0f;

			bool mIsEmpty = true;
			bool mHasNewline = false;
		};

	public:
		/**
		 * Initializes a new text geometry object using the specified string and font. Text will attempt to fit into the provided area.
		 * If enabled it will wrap words to new line when they don't fit. Individual words will be broken into multiple
		 * pieces if they don't fit on a single line when word break is enabled, otherwise they will be clipped. If the
		 * specified area is zero size then the text will not be clipped or word wrapped in any way.
		 *
		 * After this object is constructed you may call various getter methods to get needed information.
		 */
		B3D_EXPORT TextGeometry(const U32String& text, const HFont& font, float fontSize, u32 width = 0, u32 height = 0, bool wordWrap = false, bool wordBreak = true);
		B3D_EXPORT virtual ~TextGeometry() = default;

		/**	Returns the number of lines that were generated. */
		B3D_EXPORT u32 GetLineCount() const { return mLineCount; }

		/**	Returns the number of font pages references by the used characters. */
		B3D_EXPORT u32 GetPageCount() const { return mPageCount; }

		/**	Returns the height of a line in pixels. */
		B3D_EXPORT float GetLineHeight() const;

		/**	Gets information describing a single line at the specified index. */
		B3D_EXPORT const Line& GetLine(u32 idx) const { return mLines[idx]; }

		/**	Returns font texture for the provided page index.  */
		B3D_EXPORT const HTexture& GetTextureForPage(u32 page) const;

		/**	Returns the number of quads used by all the characters in the provided page. */
		B3D_EXPORT u32 GetQuadCount(u32 page) const { return mPageInfos[page].QuadCount; }

		/**	Returns the width of the actual text in pixels. */
		B3D_EXPORT float GetWidth() const;

		/**	Returns the height of the actual text in pixels. */
		B3D_EXPORT float GetHeight() const;

	protected:
		/**
		 * Copies internally stored data in temporary buffers to a persistent buffer.
		 *
		 * @param	text			Text originally used for creating the internal temporary buffer data.
		 * @param	buffer			Memory location to copy the data to. If null then no data will be copied and the
		 *							parameter @p size will contain the required buffer size.
		 * @param	size			Size of the provided memory buffer, or if the buffer is null, this will contain the
		 *							required buffer size after method exists.
		 * @param	freeTemporary	If true the internal temporary data will be freed after copying.
		 *
		 * @note	Must be called after text data has been constructed and is in the temporary buffers.
		 */
		B3D_EXPORT void GeneratePersistentData(const U32String& text, u8* buffer, u32& size, bool freeTemporary = true);

	private:
		friend class Line;

		/**	Returns Y offset that determines the line on which the characters are placed. In pixels. */
		float GetBaselineOffset() const;

		/**	Returns the width of a single space in pixels. */
		float GetSpaceWidth() const;

		/** Gets a description of a single character referenced by its sequential index based on the original string. */
		const CharacterInformation& GetCharacter(u32 index) const { return *mCharacters[index]; }

		/** Gets a description of a single word referenced by its sequential index based on the original string. */
		const Word& GetWord(u32 index) const { return mWords[index]; }

	protected:
		const CharacterInformation** mCharacters;
		u32 mCharacterCount;

		Word* mWords;
		u32 mWordCount;

		Line* mLines;
		u32 mLineCount;

		PageInfo* mPageInfos;
		u32 mPageCount;

		HFont mFont;
		SPtr<const FontBitmapInformation> mFontBitmapInformation;

		// Static buffers used to reduce runtime memory allocation
	protected:
		/** Stores per-thread memory buffers used to reduce memory allocation. */
		// Note: I could replace this with the global frame allocator to avoid the extra logic
		struct TemporaryBuffer
		{
			TemporaryBuffer();
			~TemporaryBuffer();

			/**
			 * Allocates a new word and adds it to the buffer. Returns index of the word in the word buffer.
			 *
			 * @param[in]	spacer	Specify true if the word is only to contain spaces. (Spaces are considered a special
			 *						type of word).
			 */
			u32 AllocWord(bool spacer);

			/** Allocates a new line and adds it to the buffer. Returns index of the line in the line buffer. */
			u32 AllocLine(TextGeometry* textData);

			/**
			 * Increments the count of characters for the referenced page, and optionally creates page info if it doesn't
			 * already exist.
			 */
			void AddCharToPage(u32 page, const FontBitmapInformation& fontData);

			/**	Resets all allocation counters, but doesn't actually release memory. */
			void DeallocAll();

			Word* WordBuffer;
			u32 WordBufferSize;
			u32 NextFreeWord;

			Line* LineBuffer;
			u32 LineBufferSize;
			u32 NextFreeLine;

			PageInfo* PageBuffer;
			u32 PageBufferSize;
			u32 NextFreePageInfo;
		};

		static B3D_THREADLOCAL TemporaryBuffer* PerThreadTemporaryBuffer;

		/**	Allocates an initial set of buffers that will be reused while parsing text data. */
		static void EnsurePerThreadTemporaryBufferIsInitialized();
	};

	/** @} */

	/** @addtogroup Text
	 *  @{
	 */

	/** @copydoc TextGeometry */
	template <class AllocatorTag = DefaultAllocatorTag>
	class TTextGeometry : public TextGeometry
	{
	public:
		TTextGeometry(const U32String& text, const HFont& font, float fontSize, u32 width = 0, u32 height = 0, bool wordWrap = false, bool wordBreak = true)
			: TextGeometry(text, font, fontSize, width, height, wordWrap, wordBreak), mData(nullptr)
		{
			u32 totalBufferSize = 0;
			GeneratePersistentData(text, nullptr, totalBufferSize);

			mData = (u8*)B3DAllocate<AllocatorTag>(totalBufferSize);
			GeneratePersistentData(text, (u8*)mData, totalBufferSize);
		}

		~TTextGeometry()
		{
			if(mData != nullptr)
				B3DFree<AllocatorTag>(mData);
		}

	private:
		u8* mData;
	};

	/** @} */
} // namespace b3d
