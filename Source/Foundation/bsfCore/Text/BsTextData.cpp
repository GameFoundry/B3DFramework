//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsTextData.h"
#include "Text/BsFont.h"
#include "Math/BsVector2.h"
#include "Debug/BsDebug.h"

namespace bs
{
	const int SPACE_CHAR = 32;
	const int TAB_CHAR = 9;

	void TextDataBase::TextWord::Init(bool spacer)
	{
		mWidth = mHeight = 0;
		mSpacer = spacer;
		mSpaceWidth = 0;
		mCharsStart = 0;
		mCharsEnd = 0;
		mLastChar = nullptr;
	}

	// Assumes charIdx is an index right after last char in the list (if any). All chars need to be sequential.
	UINT32 TextDataBase::TextWord::AddChar(UINT32 charIdx, const CharDesc& desc)
	{
		UINT32 charWidth = CalcCharWidth(mLastChar, desc);

		mWidth += charWidth;
		mHeight = std::max(mHeight, desc.height);

		if(mLastChar == nullptr) // First char
			mCharsStart = mCharsEnd = charIdx;
		else
			mCharsEnd = charIdx;

		mLastChar = &desc;

		return charWidth;
	}

	UINT32 TextDataBase::TextWord::CalcWidthWithChar(const CharDesc& desc)
	{
		return mWidth + CalcCharWidth(mLastChar, desc);
	}

	UINT32 TextDataBase::TextWord::CalcCharWidth(const CharDesc* prevDesc, const CharDesc& desc)
	{
		UINT32 charWidth = desc.xAdvance;
		if (prevDesc != nullptr)
		{
			UINT32 kerning = 0;
			for (size_t j = 0; j < prevDesc->kerningPairs.size(); j++)
			{
				if (prevDesc->kerningPairs[j].otherCharId == desc.charId)
				{
					kerning = prevDesc->kerningPairs[j].amount;
					break;
				}
			}

			charWidth += kerning;
		}

		return charWidth;
	}

	void TextDataBase::TextWord::AddSpace(UINT32 spaceWidth)
	{
		mSpaceWidth += spaceWidth;
		mWidth = mSpaceWidth;
		mHeight = 0;
	}

	void TextDataBase::TextLine::Init(TextDataBase* textData)
	{
		mWidth = 0;
		mHeight = 0;
		mIsEmpty = true;
		mTextData = textData;
		mWordsStart = mWordsEnd = 0;
	}

	void TextDataBase::TextLine::Finalize(bool hasNewlineChar)
	{
		mHasNewline = hasNewlineChar;
	}

	void TextDataBase::TextLine::Add(UINT32 charIdx, const CharDesc& charDesc)
	{
		UINT32 charWidth = 0;
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = MemBuffer->AllocWord(false);
			mIsEmpty = false;
		}
		else
		{
			if(MemBuffer->WordBuffer[mWordsEnd].IsSpacer())
				mWordsEnd = MemBuffer->AllocWord(false);
		}

		TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
		charWidth = lastWord.AddChar(charIdx, charDesc);

		mWidth += charWidth;
		mHeight = std::max(mHeight, lastWord.GetHeight());
	}

	void TextDataBase::TextLine::AddSpace(UINT32 spaceWidth)
	{
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = MemBuffer->AllocWord(true);
			mIsEmpty = false;
		}
		else
			mWordsEnd = MemBuffer->AllocWord(true); // Each space is counted as its own word, to make certain operations easier

		TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
		lastWord.AddSpace(spaceWidth);

		mWidth += spaceWidth;
	}

	// Assumes wordIdx is an index right after last word in the list (if any). All words need to be sequential.
	void TextDataBase::TextLine::AddWord(UINT32 wordIdx, const TextWord& word)
	{
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = wordIdx;
			mIsEmpty = false;
		}
		else
			mWordsEnd = wordIdx;

		mWidth += word.GetWidth();
		mHeight = std::max(mHeight, word.GetHeight());
	}

	UINT32 TextDataBase::TextLine::RemoveLastWord()
	{
		if(mIsEmpty)
		{
			assert(false);
			return 0;
		}

		UINT32 lastWord = mWordsEnd--;
		if(mWordsStart == lastWord)
		{
			mIsEmpty = true;
			mWordsStart = mWordsEnd = 0;
		}

		CalculateBounds();

		return lastWord;
	}

	UINT32 TextDataBase::TextLine::CalcWidthWithChar(const CharDesc& desc)
	{
		UINT32 charWidth = 0;

		if (!mIsEmpty)
		{
			TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
			if (lastWord.IsSpacer())
				charWidth = TextWord::CalcCharWidth(nullptr, desc);
			else
				charWidth = lastWord.CalcWidthWithChar(desc) - lastWord.GetWidth();
		}
		else
		{
			charWidth = TextWord::CalcCharWidth(nullptr, desc);
		}

		return mWidth + charWidth;
	}

	bool TextDataBase::TextLine::IsAtWordBoundary() const
	{
		return mIsEmpty || MemBuffer->WordBuffer[mWordsEnd].IsSpacer();
	}

	UINT32 TextDataBase::TextLine::FillBuffer(UINT32 page, Vector2* vertices, Vector2* uvs, UINT32* indexes, UINT32 offset, UINT32 size) const
	{
		UINT32 numQuads = 0;

		if(mIsEmpty)
			return numQuads;

		UINT32 penX = 0;
		UINT32 penNegativeXOffset = 0;
		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			const TextWord& word = mTextData->GetWord(i);

			if(word.IsSpacer())
			{
				// We store invisible space quads in the first page. Even though they aren't needed
				// for rendering and we could just leave an empty space, they are needed for intersection tests
				// for things like determining caret placement and selection areas
				if(page == 0)
				{
					INT32 curX = penX;
					INT32 curY = 0;

					UINT32 curVert = offset * 4;
					UINT32 curIndex = offset * 6;

					vertices[curVert + 0] = Vector2((float)curX, (float)curY);
					vertices[curVert + 1] = Vector2((float)(curX + word.GetWidth()), (float)curY);
					vertices[curVert + 2] = Vector2((float)curX, (float)curY + (float)mTextData->GetLineHeight());
					vertices[curVert + 3] = Vector2((float)(curX + word.GetWidth()), (float)curY + (float)mTextData->GetLineHeight());

					if(uvs != nullptr)
					{
						uvs[curVert + 0] = Vector2(0.0f, 0.0f);
						uvs[curVert + 1] = Vector2(0.0f, 0.0f);
						uvs[curVert + 2] = Vector2(0.0f, 0.0f);
						uvs[curVert + 3] = Vector2(0.0f, 0.0f);
					}

					// Triangles are back-facing which makes them invisible
					if(indexes != nullptr)
					{
						indexes[curIndex + 0] = curVert + 0;
						indexes[curIndex + 1] = curVert + 2;
						indexes[curIndex + 2] = curVert + 1;
						indexes[curIndex + 3] = curVert + 1;
						indexes[curIndex + 4] = curVert + 2;
						indexes[curIndex + 5] = curVert + 3;
					}

					offset++;
					numQuads++;

					if(offset > size)
						BS_EXCEPT(InternalErrorException, "Out of buffer bounds. Buffer size: " + toString(size));
				}

				penX += word.GetWidth();
			}
			else
			{
				UINT32 kerning = 0;
				for(UINT32 j = word.GetCharsStart(); j <= word.GetCharsEnd(); j++)
				{
					const CharDesc& curChar = mTextData->GetChar(j);

					INT32 curX = penX + curChar.xOffset;
					INT32 curY = ((INT32) mTextData->GetBaselineOffset() - curChar.yOffset);

					curX += penNegativeXOffset;
					penX += curChar.xAdvance + kerning;
					
					kerning = 0;
					if((j + 1) <= word.GetCharsEnd())
					{
						const CharDesc& nextChar = mTextData->GetChar(j + 1);
						for(size_t j = 0; j < curChar.kerningPairs.size(); j++)
						{
							if(curChar.kerningPairs[j].otherCharId == nextChar.charId)
							{
								kerning = curChar.kerningPairs[j].amount;
								break;
							}
						}
					}

					if(curChar.page != page)
						continue;

					UINT32 curVert = offset * 4;
					UINT32 curIndex = offset * 6;

					vertices[curVert + 0] = Vector2((float)curX, (float)curY);
					vertices[curVert + 1] = Vector2((float)(curX + curChar.width), (float)curY);
					vertices[curVert + 2] = Vector2((float)curX, (float)curY + (float)curChar.height);
					vertices[curVert + 3] = Vector2((float)(curX + curChar.width), (float)curY + (float)curChar.height);

					if(uvs != nullptr)
					{
						uvs[curVert + 0] = Vector2(curChar.uvX, curChar.uvY);
						uvs[curVert + 1] = Vector2(curChar.uvX + curChar.uvWidth, curChar.uvY);
						uvs[curVert + 2] = Vector2(curChar.uvX, curChar.uvY + curChar.uvHeight);
						uvs[curVert + 3] = Vector2(curChar.uvX + curChar.uvWidth, curChar.uvY + curChar.uvHeight);
					}

					if(indexes != nullptr)
					{
						indexes[curIndex + 0] = curVert + 0;
						indexes[curIndex + 1] = curVert + 1;
						indexes[curIndex + 2] = curVert + 2;
						indexes[curIndex + 3] = curVert + 1;
						indexes[curIndex + 4] = curVert + 3;
						indexes[curIndex + 5] = curVert + 2;
					}

					offset++;
					numQuads++;

					if(offset > size)
						BS_EXCEPT(InternalErrorException, "Out of buffer bounds. Buffer size: " + toString(size));
				}
			}
		}

		return numQuads;
	}

	UINT32 TextDataBase::TextLine::GetNumChars() const
	{
		if(mIsEmpty)
			return 0;

		UINT32 numChars = 0;
		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			TextWord& word = MemBuffer->WordBuffer[i];

			if(word.IsSpacer())
				numChars++;
			else
				numChars += (UINT32)word.GetNumChars();
		}

		return numChars;
	}

	void TextDataBase::TextLine::CalculateBounds()
	{
		mWidth = 0;
		mHeight = 0;

		if(mIsEmpty)
			return;

		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			TextWord& word = MemBuffer->WordBuffer[i];

			mWidth += word.GetWidth();
			mHeight = std::max(mHeight, word.GetHeight());
		}
	}

	TextDataBase::TextDataBase(const U32String& text, const HFont& font, UINT32 fontSize, UINT32 width, UINT32 height,
		bool wordWrap, bool wordBreak)
		: mChars(nullptr), mNumChars(0), mWords(nullptr), mNumWords(0), mLines(nullptr), mNumLines(0), mPageInfos(nullptr)
		, mNumPageInfos(0), mFont(font), mFontData(nullptr)
	{
		// In order to reduce number of memory allocations algorithm first calculates data into temporary buffers and then copies the results
		InitAlloc();

		if(font != nullptr)
		{
			UINT32 nearestSize = font->GetClosestSize(fontSize);
			mFontData = font->GetBitmap(nearestSize);
		}

		if(mFontData == nullptr || mFontData->texturePages.size() == 0)
			return;

		if(mFontData->size != fontSize)
		{
			BS_LOG(Warning, GUI, "Unable to find font with specified size ({0}). Using nearest available size: {1}",
				fontSize, mFontData->size);
		}

		bool widthIsLimited = width > 0;
		mFont = font;

		UINT32 curLineIdx = MemBuffer->AllocLine(this);
		UINT32 curHeight = mFontData->lineHeight;
		UINT32 charIdx = 0;

		while(true)
		{
			if(charIdx >= text.size())
				break;

			UINT32 charId = text[charIdx];
			const CharDesc& charDesc = mFontData->GetCharDesc(charId);

			TextLine* curLine = &MemBuffer->LineBuffer[curLineIdx];

			if(text[charIdx] == '\n' || text[charIdx] == '\r')
			{
				curLine->Finalize(true);

				curLineIdx = MemBuffer->AllocLine(this);
				curLine = &MemBuffer->LineBuffer[curLineIdx];

				curHeight += mFontData->lineHeight;

				charIdx++;

				// Check for \r\n
				if (text[charIdx - 1] == '\r' && charIdx < text.size())
				{
					if (text[charIdx] == '\n')
						charIdx++;
				}

				continue;
			}

			if (widthIsLimited && wordWrap)
			{
				UINT32 widthWithChar = 0;
				if (charIdx == SPACE_CHAR)
					widthWithChar = curLine->GetWidth() + GetSpaceWidth();
				else if (charIdx == TAB_CHAR)
					widthWithChar = curLine->GetWidth() + GetSpaceWidth() * 4;
				else
					widthWithChar = curLine->CalcWidthWithChar(charDesc);

				if (widthWithChar > width && !curLine->IsEmpty())
				{
					bool atWordBoundary = charId == SPACE_CHAR || charId == TAB_CHAR || curLine->IsAtWordBoundary();

					if (!atWordBoundary) // Need to break word into multiple pieces, or move it to next line
					{
						UINT32 lastWordIdx = curLine->RemoveLastWord();
						TextWord& lastWord = MemBuffer->WordBuffer[lastWordIdx];

						bool wordFits = lastWord.CalcWidthWithChar(charDesc) <= width;
						if (wordFits && !curLine->IsEmpty())
						{
							curLine->Finalize(false);

							curLineIdx = MemBuffer->AllocLine(this);
							curLine = &MemBuffer->LineBuffer[curLineIdx];

							curHeight += mFontData->lineHeight;

							curLine->AddWord(lastWordIdx, lastWord);
						}
						else
						{
							if (wordBreak)
							{
								curLine->AddWord(lastWordIdx, lastWord);
								curLine->Finalize(false);

								curLineIdx = MemBuffer->AllocLine(this);
								curLine = &MemBuffer->LineBuffer[curLineIdx];

								curHeight += mFontData->lineHeight;
							}
							else
							{
								if (!curLine->IsEmpty()) // Add new line unless current line is empty (to avoid constantly moving the word to new lines)
								{
									curLine->Finalize(false);

									curLineIdx = MemBuffer->AllocLine(this);
									curLine = &MemBuffer->LineBuffer[curLineIdx];

									curHeight += mFontData->lineHeight;
								}

								curLine->AddWord(lastWordIdx, lastWord);
							}
						}
					}
					else if (charId != SPACE_CHAR && charId != TAB_CHAR) // If current char is whitespace add it to the existing line even if it doesn't fit
					{
						curLine->Finalize(false);

						curLineIdx = MemBuffer->AllocLine(this);
						curLine = &MemBuffer->LineBuffer[curLineIdx];

						curHeight += mFontData->lineHeight;
					}
				}
			}

			if(charId == SPACE_CHAR)
			{
				curLine->AddSpace(GetSpaceWidth());
				MemBuffer->AddCharToPage(0, *mFontData);
			}
			else if (charId == TAB_CHAR)
			{
				curLine->AddSpace(GetSpaceWidth() * 4);
				MemBuffer->AddCharToPage(0, *mFontData);
			}
			else
			{
				curLine->Add(charIdx, charDesc);
				MemBuffer->AddCharToPage(charDesc.page, *mFontData);
			}

			charIdx++;
		}

		MemBuffer->LineBuffer[curLineIdx].Finalize(true);

		// Now that we have all the data we need, allocate the permanent buffers and copy the data
		mNumChars = (UINT32)text.size();
		mNumWords = MemBuffer->NextFreeWord;
		mNumLines = MemBuffer->NextFreeLine;
		mNumPageInfos = MemBuffer->NextFreePageInfo;
	}

	void TextDataBase::GeneratePersistentData(const U32String& text, UINT8* buffer, UINT32& size, bool freeTemporary)
	{
		UINT32 charArraySize = mNumChars * sizeof(const CharDesc*);
		UINT32 wordArraySize = mNumWords * sizeof(TextWord);
		UINT32 lineArraySize = mNumLines * sizeof(TextLine);
		UINT32 pageInfoArraySize = mNumPageInfos * sizeof(PageInfo);

		if (buffer == nullptr)
		{
			size = charArraySize + wordArraySize + lineArraySize + pageInfoArraySize;;
			return;
		}

		UINT8* dataPtr = (UINT8*)buffer;
		mChars = (const CharDesc**)dataPtr;

		for (UINT32 i = 0; i < mNumChars; i++)
		{
			UINT32 charId = text[i];
			const CharDesc& charDesc = mFontData->GetCharDesc(charId);

			mChars[i] = &charDesc;
		}

		dataPtr += charArraySize;
		mWords = (TextWord*)dataPtr;
		memcpy(mWords, &MemBuffer->WordBuffer[0], wordArraySize);

		dataPtr += wordArraySize;
		mLines = (TextLine*)dataPtr;
		memcpy(mLines, &MemBuffer->LineBuffer[0], lineArraySize);

		dataPtr += lineArraySize;
		mPageInfos = (PageInfo*)dataPtr;
		memcpy((void*)mPageInfos, (void*)&MemBuffer->PageBuffer[0], pageInfoArraySize);

		if (freeTemporary)
			MemBuffer->DeallocAll();
	}

	const HTexture& TextDataBase::GetTextureForPage(UINT32 page) const
	{
		return mFontData->texturePages[page];
	}

	INT32 TextDataBase::GetBaselineOffset() const
	{
		return mFontData->baselineOffset;
	}

	UINT32 TextDataBase::GetLineHeight() const
	{
		return mFontData->lineHeight;
	}

	UINT32 TextDataBase::GetSpaceWidth() const
	{
		return mFontData->spaceWidth;
	}

	void TextDataBase::InitAlloc()
	{
		if (MemBuffer == nullptr)
			MemBuffer = bs_new<BufferData>();
	}

	BS_THREADLOCAL TextDataBase::BufferData* TextDataBase::MemBuffer = nullptr;

	TextDataBase::BufferData::BufferData()
	{
		WordBufferSize = 2000;
		LineBufferSize = 500;
		PageBufferSize = 20;

		NextFreeWord = 0;
		NextFreeLine = 0;
		NextFreePageInfo = 0;

		WordBuffer = bs_newN<TextWord>(WordBufferSize);
		LineBuffer = bs_newN<TextLine>(LineBufferSize);
		PageBuffer = bs_newN<PageInfo>(PageBufferSize);
	}

	TextDataBase::BufferData::~BufferData()
	{
		bs_deleteN(WordBuffer, WordBufferSize);
		bs_deleteN(LineBuffer, LineBufferSize);
		bs_deleteN(PageBuffer, PageBufferSize);
	}

	UINT32 TextDataBase::BufferData::AllocWord(bool spacer)
	{
		if(NextFreeWord >= WordBufferSize)
		{
			UINT32 newBufferSize = WordBufferSize * 2;
			TextWord* newBuffer = bs_newN<TextWord>(newBufferSize);
			memcpy(WordBuffer, newBuffer, WordBufferSize);

			bs_deleteN(WordBuffer, WordBufferSize);
			WordBuffer = newBuffer;
			WordBufferSize = newBufferSize;
		}

		WordBuffer[NextFreeWord].Init(spacer);

		return NextFreeWord++;
	}

	UINT32 TextDataBase::BufferData::AllocLine(TextDataBase* textData)
	{
		if(NextFreeLine >= LineBufferSize)
		{
			UINT32 newBufferSize = LineBufferSize * 2;
			TextLine* newBuffer = bs_newN<TextLine>(newBufferSize);
			memcpy(LineBuffer, newBuffer, LineBufferSize);

			bs_deleteN(LineBuffer, LineBufferSize);
			LineBuffer = newBuffer;
			LineBufferSize = newBufferSize;
		}

		LineBuffer[NextFreeLine].Init(textData);

		return NextFreeLine++;
	}

	void TextDataBase::BufferData::DeallocAll()
	{
		NextFreeWord = 0;
		NextFreeLine = 0;
		NextFreePageInfo = 0;
	}

	void TextDataBase::BufferData::AddCharToPage(UINT32 page, const FontBitmap& fontData)
	{
		if(NextFreePageInfo >= PageBufferSize)
		{
			UINT32 newBufferSize = PageBufferSize * 2;
			PageInfo* newBuffer = bs_newN<PageInfo>(newBufferSize);
			memcpy((void*)PageBuffer, (void*)newBuffer, PageBufferSize);

			bs_deleteN(PageBuffer, PageBufferSize);
			PageBuffer = newBuffer;
			PageBufferSize = newBufferSize;
		}

		while(page >= NextFreePageInfo)
		{
			PageBuffer[NextFreePageInfo].numQuads = 0;

			NextFreePageInfo++;
		}

		PageBuffer[page].numQuads++;
	}

	UINT32 TextDataBase::GetWidth() const
	{
		UINT32 width = 0;

		for(UINT32 i = 0; i < mNumLines; i++)
			width = std::max(width, mLines[i].GetWidth());

		return width;
	}

	UINT32 TextDataBase::GetHeight() const
	{
		UINT32 height = 0;

		for(UINT32 i = 0; i < mNumLines; i++)
			height += mLines[i].GetHeight();

		return height;
	}
}
