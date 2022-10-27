//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsTextData.h"
#include "Text/BsFont.h"
#include "Math/BsVector2.h"
#include "Debug/BsDebug.h"

using namespace bs;

const int kSpaceChar = 32;
const int kTabChar = 9;

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
u32 TextDataBase::TextWord::AddChar(u32 charIdx, const CharDesc& desc)
{
	u32 charWidth = CalcCharWidth(mLastChar, desc);

	mWidth += charWidth;
	mHeight = std::max(mHeight, desc.Height);

	if(mLastChar == nullptr) // First char
		mCharsStart = mCharsEnd = charIdx;
	else
		mCharsEnd = charIdx;

	mLastChar = &desc;

	return charWidth;
}

u32 TextDataBase::TextWord::CalcWidthWithChar(const CharDesc& desc)
{
	return mWidth + CalcCharWidth(mLastChar, desc);
}

u32 TextDataBase::TextWord::CalcCharWidth(const CharDesc* prevDesc, const CharDesc& desc)
{
	u32 charWidth = desc.XAdvance;
	if(prevDesc != nullptr)
	{
		u32 kerning = 0;
		for(size_t j = 0; j < prevDesc->KerningPairs.size(); j++)
		{
			if(prevDesc->KerningPairs[j].OtherCharId == desc.CharId)
			{
				kerning = prevDesc->KerningPairs[j].Amount;
				break;
			}
		}

		charWidth += kerning;
	}

	return charWidth;
}

void TextDataBase::TextWord::AddSpace(u32 spaceWidth)
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

void TextDataBase::TextLine::Add(u32 charIdx, const CharDesc& charDesc)
{
	u32 charWidth = 0;
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

void TextDataBase::TextLine::AddSpace(u32 spaceWidth)
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
void TextDataBase::TextLine::AddWord(u32 wordIdx, const TextWord& word)
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

u32 TextDataBase::TextLine::RemoveLastWord()
{
	if(mIsEmpty)
	{
		assert(false);
		return 0;
	}

	u32 lastWord = mWordsEnd--;
	if(mWordsStart == lastWord)
	{
		mIsEmpty = true;
		mWordsStart = mWordsEnd = 0;
	}

	CalculateBounds();

	return lastWord;
}

u32 TextDataBase::TextLine::CalcWidthWithChar(const CharDesc& desc)
{
	u32 charWidth = 0;

	if(!mIsEmpty)
	{
		TextWord& lastWord = MemBuffer->WordBuffer[mWordsEnd];
		if(lastWord.IsSpacer())
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

u32 TextDataBase::TextLine::FillBuffer(u32 page, Vector2* vertices, Vector2* uvs, u32* indexes, u32 offset, u32 size) const
{
	u32 numQuads = 0;

	if(mIsEmpty)
		return numQuads;

	u32 penX = 0;
	u32 penNegativeXOffset = 0;
	for(u32 i = mWordsStart; i <= mWordsEnd; i++)
	{
		const TextWord& word = mTextData->GetWord(i);

		if(word.IsSpacer())
		{
			// We store invisible space quads in the first page. Even though they aren't needed
			// for rendering and we could just leave an empty space, they are needed for intersection tests
			// for things like determining caret placement and selection areas
			if(page == 0)
			{
				i32 curX = penX;
				i32 curY = 0;

				u32 curVert = offset * 4;
				u32 curIndex = offset * 6;

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
			u32 kerning = 0;
			for(u32 j = word.GetCharsStart(); j <= word.GetCharsEnd(); j++)
			{
				const CharDesc& curChar = mTextData->GetChar(j);

				i32 curX = penX + curChar.XOffset;
				i32 curY = ((i32)mTextData->GetBaselineOffset() - curChar.YOffset);

				curX += penNegativeXOffset;
				penX += curChar.XAdvance + kerning;

				kerning = 0;
				if((j + 1) <= word.GetCharsEnd())
				{
					const CharDesc& nextChar = mTextData->GetChar(j + 1);
					for(size_t j = 0; j < curChar.KerningPairs.size(); j++)
					{
						if(curChar.KerningPairs[j].OtherCharId == nextChar.CharId)
						{
							kerning = curChar.KerningPairs[j].Amount;
							break;
						}
					}
				}

				if(curChar.Page != page)
					continue;

				u32 curVert = offset * 4;
				u32 curIndex = offset * 6;

				vertices[curVert + 0] = Vector2((float)curX, (float)curY);
				vertices[curVert + 1] = Vector2((float)(curX + curChar.Width), (float)curY);
				vertices[curVert + 2] = Vector2((float)curX, (float)curY + (float)curChar.Height);
				vertices[curVert + 3] = Vector2((float)(curX + curChar.Width), (float)curY + (float)curChar.Height);

				if(uvs != nullptr)
				{
					uvs[curVert + 0] = Vector2(curChar.UvX, curChar.UvY);
					uvs[curVert + 1] = Vector2(curChar.UvX + curChar.UvWidth, curChar.UvY);
					uvs[curVert + 2] = Vector2(curChar.UvX, curChar.UvY + curChar.UvHeight);
					uvs[curVert + 3] = Vector2(curChar.UvX + curChar.UvWidth, curChar.UvY + curChar.UvHeight);
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

u32 TextDataBase::TextLine::GetNumChars() const
{
	if(mIsEmpty)
		return 0;

	u32 numChars = 0;
	for(u32 i = mWordsStart; i <= mWordsEnd; i++)
	{
		TextWord& word = MemBuffer->WordBuffer[i];

		if(word.IsSpacer())
			numChars++;
		else
			numChars += (u32)word.GetNumChars();
	}

	return numChars;
}

void TextDataBase::TextLine::CalculateBounds()
{
	mWidth = 0;
	mHeight = 0;

	if(mIsEmpty)
		return;

	for(u32 i = mWordsStart; i <= mWordsEnd; i++)
	{
		TextWord& word = MemBuffer->WordBuffer[i];

		mWidth += word.GetWidth();
		mHeight = std::max(mHeight, word.GetHeight());
	}
}

TextDataBase::TextDataBase(const U32String& text, const HFont& font, u32 fontSize, u32 width, u32 height, bool wordWrap, bool wordBreak)
	: mChars(nullptr), mNumChars(0), mWords(nullptr), mNumWords(0), mLines(nullptr), mNumLines(0), mPageInfos(nullptr), mNumPageInfos(0), mFont(font), mFontData(nullptr)
{
	// In order to reduce number of memory allocations algorithm first calculates data into temporary buffers and then copies the results
	InitAlloc();

	if(font != nullptr)
	{
		u32 nearestSize = font->GetClosestSize(fontSize);
		mFontData = font->GetBitmap(nearestSize);
	}

	if(mFontData == nullptr || mFontData->TexturePages.size() == 0)
		return;

	if(mFontData->Size != fontSize)
	{
		BS_LOG(Warning, GUI, "Unable to find font with specified size ({0}). Using nearest available size: {1}", fontSize, mFontData->Size);
	}

	bool widthIsLimited = width > 0;
	mFont = font;

	u32 curLineIdx = MemBuffer->AllocLine(this);
	u32 curHeight = mFontData->LineHeight;
	u32 charIdx = 0;

	while(true)
	{
		if(charIdx >= text.size())
			break;

		u32 charId = text[charIdx];
		const CharDesc& charDesc = mFontData->GetCharDesc(charId);

		TextLine* curLine = &MemBuffer->LineBuffer[curLineIdx];

		if(text[charIdx] == '\n' || text[charIdx] == '\r')
		{
			curLine->Finalize(true);

			curLineIdx = MemBuffer->AllocLine(this);
			curLine = &MemBuffer->LineBuffer[curLineIdx];

			curHeight += mFontData->LineHeight;

			charIdx++;

			// Check for \r\n
			if(text[charIdx - 1] == '\r' && charIdx < text.size())
			{
				if(text[charIdx] == '\n')
					charIdx++;
			}

			continue;
		}

		if(widthIsLimited && wordWrap)
		{
			u32 widthWithChar = 0;
			if(charIdx == kSpaceChar)
				widthWithChar = curLine->GetWidth() + GetSpaceWidth();
			else if(charIdx == kTabChar)
				widthWithChar = curLine->GetWidth() + GetSpaceWidth() * 4;
			else
				widthWithChar = curLine->CalcWidthWithChar(charDesc);

			if(widthWithChar > width && !curLine->IsEmpty())
			{
				bool atWordBoundary = charId == kSpaceChar || charId == kTabChar || curLine->IsAtWordBoundary();

				if(!atWordBoundary) // Need to break word into multiple pieces, or move it to next line
				{
					u32 lastWordIdx = curLine->RemoveLastWord();
					TextWord& lastWord = MemBuffer->WordBuffer[lastWordIdx];

					bool wordFits = lastWord.CalcWidthWithChar(charDesc) <= width;
					if(wordFits && !curLine->IsEmpty())
					{
						curLine->Finalize(false);

						curLineIdx = MemBuffer->AllocLine(this);
						curLine = &MemBuffer->LineBuffer[curLineIdx];

						curHeight += mFontData->LineHeight;

						curLine->AddWord(lastWordIdx, lastWord);
					}
					else
					{
						if(wordBreak)
						{
							curLine->AddWord(lastWordIdx, lastWord);
							curLine->Finalize(false);

							curLineIdx = MemBuffer->AllocLine(this);
							curLine = &MemBuffer->LineBuffer[curLineIdx];

							curHeight += mFontData->LineHeight;
						}
						else
						{
							if(!curLine->IsEmpty()) // Add new line unless current line is empty (to avoid constantly moving the word to new lines)
							{
								curLine->Finalize(false);

								curLineIdx = MemBuffer->AllocLine(this);
								curLine = &MemBuffer->LineBuffer[curLineIdx];

								curHeight += mFontData->LineHeight;
							}

							curLine->AddWord(lastWordIdx, lastWord);
						}
					}
				}
				else if(charId != kSpaceChar && charId != kTabChar) // If current char is whitespace add it to the existing line even if it doesn't fit
				{
					curLine->Finalize(false);

					curLineIdx = MemBuffer->AllocLine(this);
					curLine = &MemBuffer->LineBuffer[curLineIdx];

					curHeight += mFontData->LineHeight;
				}
			}
		}

		if(charId == kSpaceChar)
		{
			curLine->AddSpace(GetSpaceWidth());
			MemBuffer->AddCharToPage(0, *mFontData);
		}
		else if(charId == kTabChar)
		{
			curLine->AddSpace(GetSpaceWidth() * 4);
			MemBuffer->AddCharToPage(0, *mFontData);
		}
		else
		{
			curLine->Add(charIdx, charDesc);
			MemBuffer->AddCharToPage(charDesc.Page, *mFontData);
		}

		charIdx++;
	}

	MemBuffer->LineBuffer[curLineIdx].Finalize(true);

	// Now that we have all the data we need, allocate the permanent buffers and copy the data
	mNumChars = (u32)text.size();
	mNumWords = MemBuffer->NextFreeWord;
	mNumLines = MemBuffer->NextFreeLine;
	mNumPageInfos = MemBuffer->NextFreePageInfo;
}

void TextDataBase::GeneratePersistentData(const U32String& text, u8* buffer, u32& size, bool freeTemporary)
{
	u32 charArraySize = mNumChars * sizeof(const CharDesc*);
	u32 wordArraySize = mNumWords * sizeof(TextWord);
	u32 lineArraySize = mNumLines * sizeof(TextLine);
	u32 pageInfoArraySize = mNumPageInfos * sizeof(PageInfo);

	if(buffer == nullptr)
	{
		size = charArraySize + wordArraySize + lineArraySize + pageInfoArraySize;
		;
		return;
	}

	u8* dataPtr = (u8*)buffer;
	mChars = (const CharDesc**)dataPtr;

	for(u32 i = 0; i < mNumChars; i++)
	{
		u32 charId = text[i];
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

	if(freeTemporary)
		MemBuffer->DeallocAll();
}

const HTexture& TextDataBase::GetTextureForPage(u32 page) const
{
	return mFontData->TexturePages[page];
}

i32 TextDataBase::GetBaselineOffset() const
{
	return mFontData->BaselineOffset;
}

u32 TextDataBase::GetLineHeight() const
{
	return mFontData->LineHeight;
}

u32 TextDataBase::GetSpaceWidth() const
{
	return mFontData->SpaceWidth;
}

void TextDataBase::InitAlloc()
{
	if(MemBuffer == nullptr)
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

u32 TextDataBase::BufferData::AllocWord(bool spacer)
{
	if(NextFreeWord >= WordBufferSize)
	{
		u32 newBufferSize = WordBufferSize * 2;
		TextWord* newBuffer = bs_newN<TextWord>(newBufferSize);
		memcpy(WordBuffer, newBuffer, WordBufferSize);

		bs_deleteN(WordBuffer, WordBufferSize);
		WordBuffer = newBuffer;
		WordBufferSize = newBufferSize;
	}

	WordBuffer[NextFreeWord].Init(spacer);

	return NextFreeWord++;
}

u32 TextDataBase::BufferData::AllocLine(TextDataBase* textData)
{
	if(NextFreeLine >= LineBufferSize)
	{
		u32 newBufferSize = LineBufferSize * 2;
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

void TextDataBase::BufferData::AddCharToPage(u32 page, const FontBitmap& fontData)
{
	if(NextFreePageInfo >= PageBufferSize)
	{
		u32 newBufferSize = PageBufferSize * 2;
		PageInfo* newBuffer = bs_newN<PageInfo>(newBufferSize);
		memcpy((void*)PageBuffer, (void*)newBuffer, PageBufferSize);

		bs_deleteN(PageBuffer, PageBufferSize);
		PageBuffer = newBuffer;
		PageBufferSize = newBufferSize;
	}

	while(page >= NextFreePageInfo)
	{
		PageBuffer[NextFreePageInfo].NumQuads = 0;

		NextFreePageInfo++;
	}

	PageBuffer[page].NumQuads++;
}

u32 TextDataBase::GetWidth() const
{
	u32 width = 0;

	for(u32 i = 0; i < mNumLines; i++)
		width = std::max(width, mLines[i].GetWidth());

	return width;
}

u32 TextDataBase::GetHeight() const
{
	u32 height = 0;

	for(u32 i = 0; i < mNumLines; i++)
		height += mLines[i].GetHeight();

	return height;
}
