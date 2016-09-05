//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextData.h"
#include "BsFont.h"
#include "BsVector2.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	const int SPACE_CHAR = 32;
	const int TAB_CHAR = 9;

	void TextDataBase::TextWord::init(bool spacer)
	{
		mWidth = mHeight = 0;
		mSpacer = spacer;
		mSpaceWidth = 0;
		mCharsStart = 0;
		mCharsEnd = 0;
		mLastChar = nullptr;
	}

	// Assumes charIdx is an index right after last char in the list (if any). All chars need to be sequential.
	UINT32 TextDataBase::TextWord::addChar(UINT32 charIdx, const CHAR_DESC& desc)
	{
		UINT32 charWidth = calcCharWidth(mLastChar, desc);

		mWidth += charWidth;
		mHeight = std::max(mHeight, desc.height);

		if(mLastChar == nullptr) // First char
			mCharsStart = mCharsEnd = charIdx;
		else
			mCharsEnd = charIdx;

		mLastChar = &desc;

		return charWidth;
	}

	UINT32 TextDataBase::TextWord::calcWidthWithChar(const CHAR_DESC& desc)
	{
		return mWidth + calcCharWidth(mLastChar, desc);
	}

	UINT32 TextDataBase::TextWord::calcCharWidth(const CHAR_DESC* prevDesc, const CHAR_DESC& desc)
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

	void TextDataBase::TextWord::addSpace(UINT32 spaceWidth)
	{
		mSpaceWidth += spaceWidth;
		mWidth = mSpaceWidth;
		mHeight = 0;
	}

	void TextDataBase::TextLine::init(TextDataBase* textData)
	{
		mWidth = 0;
		mHeight = 0;
		mIsEmpty = true; 
		mTextData = textData;
		mWordsStart = mWordsEnd = 0;
	}

	void TextDataBase::TextLine::finalize(bool hasNewlineChar)
	{
		mHasNewline = hasNewlineChar;
	}

	void TextDataBase::TextLine::add(UINT32 charIdx, const CHAR_DESC& charDesc)
	{
		UINT32 charWidth = 0;
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = MemBuffer->allocWord(false);
			mIsEmpty = false;
		}
		else
		{
			if(MemBuffer->WordBuffer[mWordsEnd].isSpacer())
				mWordsEnd = MemBuffer->allocWord(false);
		}

		TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
		charWidth = lastWord.addChar(charIdx, charDesc);

		mWidth += charWidth;
		mHeight = std::max(mHeight, lastWord.getHeight());
	}

	void TextDataBase::TextLine::addSpace(UINT32 spaceWidth)
	{
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = MemBuffer->allocWord(true);
			mIsEmpty = false;
		}
		else
			mWordsEnd = MemBuffer->allocWord(true); // Each space is counted as its own word, to make certain operations easier

		TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
		lastWord.addSpace(spaceWidth);

		mWidth += spaceWidth;
	}

	// Assumes wordIdx is an index right after last word in the list (if any). All words need to be sequential.
	void TextDataBase::TextLine::addWord(UINT32 wordIdx, const TextWord& word)
	{
		if(mIsEmpty)
		{
			mWordsStart = mWordsEnd = wordIdx;
			mIsEmpty = false;
		}
		else
			mWordsEnd = wordIdx;

		mWidth += word.getWidth();
		mHeight = std::max(mHeight, word.getHeight());
	}

	UINT32 TextDataBase::TextLine::removeLastWord()
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

		calculateBounds();

		return lastWord;
	}

	UINT32 TextDataBase::TextLine::calcWidthWithChar(const CHAR_DESC& desc)
	{
		UINT32 charWidth = 0;

		if (!mIsEmpty)
		{
			TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
			if (lastWord.isSpacer())
				charWidth = TextWord::calcCharWidth(nullptr, desc);
			else
				charWidth = lastWord.calcWidthWithChar(desc) - lastWord.getWidth();
		}
		else
		{
			charWidth = TextWord::calcCharWidth(nullptr, desc);
		}

		return mWidth + charWidth;
	}

	bool TextDataBase::TextLine::isAtWordBoundary() const
	{
		return mIsEmpty || MemBuffer->WordBuffer[mWordsEnd].isSpacer();
	}

	UINT32 TextDataBase::TextLine::fillBuffer(UINT32 page, Vector2* vertices, Vector2* uvs, UINT32* indexes, UINT32 offset, UINT32 size) const
	{
		UINT32 numQuads = 0;

		if(mIsEmpty)
			return numQuads;

		UINT32 penX = 0;
		UINT32 penNegativeXOffset = 0;
		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			const TextWord& word = mTextData->getWord(i);

			if(word.isSpacer())
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
					vertices[curVert + 1] = Vector2((float)(curX + word.getWidth()), (float)curY);
					vertices[curVert + 2] = Vector2((float)curX, (float)curY + (float)mTextData->getLineHeight());
					vertices[curVert + 3] = Vector2((float)(curX + word.getWidth()), (float)curY + (float)mTextData->getLineHeight());

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

				penX += word.getWidth();
			}
			else
			{
				UINT32 kerning = 0;
				for(UINT32 j = word.getCharsStart(); j <= word.getCharsEnd(); j++)
				{
					const CHAR_DESC& curChar = mTextData->getChar(j);

					INT32 curX = penX + curChar.xOffset;
					INT32 curY = ((INT32) mTextData->getBaselineOffset() - curChar.yOffset);

					// If index is negative, offset it so the text always begins at X=0. This works under the assumption
					// that only the first character on a line can have a negative offset.
					if (curX < 0)
						penNegativeXOffset = penX - curChar.xOffset;

					curX += penNegativeXOffset;
					penX += curChar.xAdvance + kerning;
					
					kerning = 0;
					if((j + 1) <= word.getCharsEnd())
					{
						const CHAR_DESC& nextChar = mTextData->getChar(j + 1);
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

	UINT32 TextDataBase::TextLine::getNumChars() const
	{
		if(mIsEmpty)
			return 0;

		UINT32 numChars = 0;
		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			TextWord& word = MemBuffer->WordBuffer[i];

			if(word.isSpacer())
				numChars++;
			else
				numChars += (UINT32)word.getNumChars();
		}

		return numChars;
	}

	void TextDataBase::TextLine::calculateBounds()
	{
		mWidth = 0;
		mHeight = 0;

		if(mIsEmpty)
			return;

		for(UINT32 i = mWordsStart; i <= mWordsEnd; i++)
		{
			TextWord& word = MemBuffer->WordBuffer[i];

			mWidth += word.getWidth();
			mHeight = std::max(mHeight, word.getHeight());
		}
	}

	TextDataBase::TextDataBase(const WString& text, const HFont& font, UINT32 fontSize, UINT32 width, UINT32 height, bool wordWrap, bool wordBreak)
		: mChars(nullptr), mNumChars(0), mWords(nullptr), mNumWords(0), mLines(nullptr), mNumLines(0), mPageInfos(nullptr)
		, mNumPageInfos(0), mFont(font), mFontData(nullptr)
	{
		// In order to reduce number of memory allocations algorithm first calculates data into temporary buffers and then copies the results
		initAlloc();

		if(font != nullptr)
		{
			UINT32 nearestSize = font->getClosestSize(fontSize);
			mFontData = font->getBitmap(nearestSize);
		}

		if(mFontData == nullptr || mFontData->texturePages.size() == 0)
			return;

		if(mFontData->size != fontSize)
		{
			LOGWRN("Unable to find font with specified size (" + toString(fontSize) + "). Using nearest available size: " + toString(mFontData->size));
		}

		bool widthIsLimited = width > 0;
		mFont = font;

		UINT32 curLineIdx = MemBuffer->allocLine(this);
		UINT32 curHeight = mFontData->fontDesc.lineHeight;
		UINT32 charIdx = 0;

		while(true)
		{
			if(charIdx >= text.size())
				break;

			UINT32 charId = text[charIdx];
			const CHAR_DESC& charDesc = mFontData->getCharDesc(charId);

			TextLine* curLine = &MemBuffer->LineBuffer[curLineIdx];

			if(text[charIdx] == '\n' || text[charIdx] == '\r')
			{
				curLine->finalize(true);

				curLineIdx = MemBuffer->allocLine(this);
				curLine = &MemBuffer->LineBuffer[curLineIdx];

				curHeight += mFontData->fontDesc.lineHeight;

				charIdx++;

				// Check for \r\n
				if (charIdx < text.size())
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
					widthWithChar = curLine->getWidth() + getSpaceWidth();
				else if (charIdx == TAB_CHAR)
					widthWithChar = curLine->getWidth() + getSpaceWidth() * 4;
				else
					widthWithChar = curLine->calcWidthWithChar(charDesc);

				if (widthWithChar > width && !curLine->isEmpty())
				{
					bool atWordBoundary = charId == SPACE_CHAR || charId == TAB_CHAR || curLine->isAtWordBoundary();

					if (!atWordBoundary) // Need to break word into multiple pieces, or move it to next line
					{
						UINT32 lastWordIdx = curLine->removeLastWord();
						TextWord& lastWord = MemBuffer->WordBuffer[lastWordIdx];

						bool wordFits = lastWord.calcWidthWithChar(charDesc) <= width;
						if (wordFits && !curLine->isEmpty())
						{
							curLine->finalize(false);

							curLineIdx = MemBuffer->allocLine(this);
							curLine = &MemBuffer->LineBuffer[curLineIdx];

							curHeight += mFontData->fontDesc.lineHeight;

							curLine->addWord(lastWordIdx, lastWord);
						}
						else
						{
							if (wordBreak)
							{
								curLine->addWord(lastWordIdx, lastWord);
								curLine->finalize(false);

								curLineIdx = MemBuffer->allocLine(this);
								curLine = &MemBuffer->LineBuffer[curLineIdx];

								curHeight += mFontData->fontDesc.lineHeight;
							}
							else
							{
								if (!curLine->isEmpty()) // Add new line unless current line is empty (to avoid constantly moving the word to new lines)
								{
									curLine->finalize(false);

									curLineIdx = MemBuffer->allocLine(this);
									curLine = &MemBuffer->LineBuffer[curLineIdx];

									curHeight += mFontData->fontDesc.lineHeight;
								}

								curLine->addWord(lastWordIdx, lastWord);
							}
						}
					}
					else if (charId != SPACE_CHAR && charId != TAB_CHAR) // If current char is whitespace add it to the existing line even if it doesn't fit
					{
						curLine->finalize(false);

						curLineIdx = MemBuffer->allocLine(this);
						curLine = &MemBuffer->LineBuffer[curLineIdx];

						curHeight += mFontData->fontDesc.lineHeight;
					}
				}
			}

			if(charId == SPACE_CHAR)
			{
				curLine->addSpace(getSpaceWidth());
				MemBuffer->addCharToPage(0, *mFontData);
			}
			else if (charId == TAB_CHAR)
			{
				curLine->addSpace(getSpaceWidth() * 4);
				MemBuffer->addCharToPage(0, *mFontData);
			}
			else
			{
				curLine->add(charIdx, charDesc);
				MemBuffer->addCharToPage(charDesc.page, *mFontData);
			}

			charIdx++;
		}

		MemBuffer->LineBuffer[curLineIdx].finalize(true);

		// Now that we have all the data we need, allocate the permanent buffers and copy the data
		mNumChars = (UINT32)text.size();
		mNumWords = MemBuffer->NextFreeWord;
		mNumLines = MemBuffer->NextFreeLine;
		mNumPageInfos = MemBuffer->NextFreePageInfo;
	}

	void TextDataBase::generatePersistentData(const WString& text, UINT8* buffer, UINT32& size, bool freeTemporary)
	{
		UINT32 charArraySize = mNumChars * sizeof(const CHAR_DESC*);
		UINT32 wordArraySize = mNumWords * sizeof(TextWord);
		UINT32 lineArraySize = mNumLines * sizeof(TextLine);
		UINT32 pageInfoArraySize = mNumPageInfos * sizeof(PageInfo);

		if (buffer == nullptr)
		{
			size = charArraySize + wordArraySize + lineArraySize + pageInfoArraySize;;
			return;
		}

		UINT8* dataPtr = (UINT8*)buffer;
		mChars = (const CHAR_DESC**)dataPtr;

		for (UINT32 i = 0; i < mNumChars; i++)
		{
			UINT32 charId = text[i];
			const CHAR_DESC& charDesc = mFontData->getCharDesc(charId);

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
			MemBuffer->deallocAll();
	}

	const HTexture& TextDataBase::getTextureForPage(UINT32 page) const 
	{ 
		return mFontData->texturePages[page]; 
	}

	INT32 TextDataBase::getBaselineOffset() const 
	{ 
		return mFontData->fontDesc.baselineOffset; 
	}

	UINT32 TextDataBase::getLineHeight() const 
	{ 
		return mFontData->fontDesc.lineHeight; 
	}

	UINT32 TextDataBase::getSpaceWidth() const 
	{ 
		return mFontData->fontDesc.spaceWidth; 
	}

	void TextDataBase::initAlloc()
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

	UINT32 TextDataBase::BufferData::allocWord(bool spacer)
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

		WordBuffer[NextFreeWord].init(spacer);

		return NextFreeWord++;
	}

	UINT32 TextDataBase::BufferData::allocLine(TextDataBase* textData)
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

		LineBuffer[NextFreeLine].init(textData);

		return NextFreeLine++;
	}

	void TextDataBase::BufferData::deallocAll()
	{
		NextFreeWord = 0;
		NextFreeLine = 0;
		NextFreePageInfo = 0;
	}

	void TextDataBase::BufferData::addCharToPage(UINT32 page, const FontBitmap& fontData)
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

	UINT32 TextDataBase::getWidth() const
	{
		UINT32 width = 0;

		for(UINT32 i = 0; i < mNumLines; i++)
			width = std::max(width, mLines[i].getWidth());

		return width;
	}

	UINT32 TextDataBase::getHeight() const
	{
		UINT32 height = 0;

		for(UINT32 i = 0; i < mNumLines; i++)
			height += mLines[i].getHeight();

		return height;
	}
}