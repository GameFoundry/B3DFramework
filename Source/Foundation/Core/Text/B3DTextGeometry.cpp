//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/B3DTextGeometry.h"
#include "Text/B3DFont.h"
#include "Math/B3DVector2.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

const int kSpaceChar = 32;
const int kTabChar = 9;

void TextGeometry::Word::Initialize(bool isSpacer)
{
	mWidth = 0.0f;
	mHeight = 0.0f;
	mIsSpacer = isSpacer;
	mSpaceWidth = 0.0f;
	mCharacterStartIndex = 0;
	mCharacterEndIndex = 0;
	mLastCharacter = nullptr;
}

// Assumes characterIndex is an index right after last char in the list (if any). All chars need to be sequential.
float TextGeometry::Word::AddCharacter(u32 characterIndex, const CharacterInformation& characterInformation)
{
	const float charWidth = CalculateCharacterWidth(mLastCharacter, characterInformation);

	mWidth += charWidth;
	mHeight = std::max(mHeight, characterInformation.Height);

	if(mLastCharacter == nullptr) // First char
		mCharacterStartIndex = mCharacterEndIndex = characterIndex;
	else
		mCharacterEndIndex = characterIndex;

	mLastCharacter = &characterInformation;

	return charWidth;
}

float TextGeometry::Word::CalculateWidthWithCharacter(const CharacterInformation& characterInformation) const
{
	return mWidth + CalculateCharacterWidth(mLastCharacter, characterInformation);
}

float TextGeometry::Word::CalculateCharacterWidth(const CharacterInformation* previousCharacter, const CharacterInformation& currentCharacter)
{
	float characterWidth = currentCharacter.XAdvance;
	if(previousCharacter != nullptr)
	{
		float kerning = 0.0f;
		for(size_t j = 0; j < previousCharacter->KerningPairs.size(); j++)
		{
			if(previousCharacter->KerningPairs[j].OtherCharId == currentCharacter.CharId)
			{
				kerning = previousCharacter->KerningPairs[j].Amount;
				break;
			}
		}

		characterWidth += kerning;
	}

	return characterWidth;
}

void TextGeometry::Word::AddSpace(float spaceWidth)
{
	mSpaceWidth += spaceWidth;
	mWidth = mSpaceWidth;
	mHeight = 0;
}

void TextGeometry::Line::Initialize(TextGeometry* textData)
{
	mWidth = 0.0f;
	mHeight = 0.0f;
	mIsEmpty = true;
	mTextData = textData;
	mWordStartIndex = 0;
	mWordEndIndex = 0;
}

void TextGeometry::Line::Finalize(bool hasNewlineChar)
{
	mHasNewline = hasNewlineChar;
}

void TextGeometry::Line::Add(u32 characterIndex, const CharacterInformation& characterInformation)
{
	float characterWidth = 0.0f;
	if(mIsEmpty)
	{
		mWordStartIndex = mWordEndIndex = PerThreadTemporaryBuffer->AllocWord(false);
		mIsEmpty = false;
	}
	else
	{
		if(PerThreadTemporaryBuffer->WordBuffer[mWordEndIndex].IsSpacer())
			mWordEndIndex = PerThreadTemporaryBuffer->AllocWord(false);
	}

	Word& lastWord = PerThreadTemporaryBuffer->WordBuffer[mWordEndIndex];
	characterWidth = lastWord.AddCharacter(characterIndex, characterInformation);

	mWidth += characterWidth;
	mHeight = std::max(mHeight, lastWord.GetHeight());
}

void TextGeometry::Line::AddSpace(float spaceWidth)
{
	if(mIsEmpty)
	{
		mWordStartIndex = mWordEndIndex = PerThreadTemporaryBuffer->AllocWord(true);
		mIsEmpty = false;
	}
	else
		mWordEndIndex = PerThreadTemporaryBuffer->AllocWord(true); // Each space is counted as its own word, to make certain operations easier

	Word& lastWord = PerThreadTemporaryBuffer->WordBuffer[mWordEndIndex];
	lastWord.AddSpace(spaceWidth);

	mWidth += spaceWidth;
}

// Assumes wordIndex is an index right after last word in the list (if any). All words need to be sequential.
void TextGeometry::Line::AddWord(u32 wordIndex, const Word& word)
{
	if(mIsEmpty)
	{
		mWordStartIndex = mWordEndIndex = wordIndex;
		mIsEmpty = false;
	}
	else
		mWordEndIndex = wordIndex;

	mWidth += word.GetWidth();
	mHeight = std::max(mHeight, word.GetHeight());
}

u32 TextGeometry::Line::RemoveLastWord()
{
	if(mIsEmpty)
	{
		B3D_ASSERT(false);
		return 0;
	}

	u32 lastWord = mWordEndIndex--;
	if(mWordStartIndex == lastWord)
	{
		mIsEmpty = true;
		mWordStartIndex = mWordEndIndex = 0;
	}

	CalculateBounds();

	return lastWord;
}

float TextGeometry::Line::CalculateWidthWithChararacter(const CharacterInformation& characterInformation) const
{
	float characterWidth = 0.0f;

	if(!mIsEmpty)
	{
		Word& lastWord = PerThreadTemporaryBuffer->WordBuffer[mWordEndIndex];
		if(lastWord.IsSpacer())
			characterWidth = Word::CalculateCharacterWidth(nullptr, characterInformation);
		else
			characterWidth = lastWord.CalculateWidthWithCharacter(characterInformation) - lastWord.GetWidth();
	}
	else
	{
		characterWidth = Word::CalculateCharacterWidth(nullptr, characterInformation);
	}

	return mWidth + characterWidth;
}

bool TextGeometry::Line::IsAtWordBoundary() const
{
	return mIsEmpty || PerThreadTemporaryBuffer->WordBuffer[mWordEndIndex].IsSpacer();
}

u32 TextGeometry::Line::FillBuffer(u32 page, Vector2* outVertices, Vector2* outUVs, u32* outIndices, u32 offset, u32 size) const
{
	u32 quadCount = 0;

	if(mIsEmpty)
		return quadCount;

	float penX = 0;
	for(u32 wordIndex = mWordStartIndex; wordIndex <= mWordEndIndex; wordIndex++)
	{
		const Word& word = mTextData->GetWord(wordIndex);

		if(word.IsSpacer())
		{
			// We store invisible space quads in the first page. Even though they aren't needed
			// for rendering and we could just leave an empty space, they are needed for intersection tests
			// for things like determining caret placement and selection areas
			if(page == 0)
			{
				float curX = penX;
				float curY = 0.0f;

				u32 curVert = offset * 4;
				u32 curIndex = offset * 6;

				outVertices[curVert + 0] = Vector2(curX, curY);
				outVertices[curVert + 1] = Vector2((curX + word.GetWidth()), curY);
				outVertices[curVert + 2] = Vector2(curX, curY + mTextData->GetLineHeight());
				outVertices[curVert + 3] = Vector2((curX + word.GetWidth()), curY + mTextData->GetLineHeight());

				if(outUVs != nullptr)
				{
					outUVs[curVert + 0] = Vector2(0.0f, 0.0f);
					outUVs[curVert + 1] = Vector2(0.0f, 0.0f);
					outUVs[curVert + 2] = Vector2(0.0f, 0.0f);
					outUVs[curVert + 3] = Vector2(0.0f, 0.0f);
				}

				// Triangles are back-facing which makes them invisible
				if(outIndices != nullptr)
				{
					outIndices[curIndex + 0] = curVert + 0;
					outIndices[curIndex + 1] = curVert + 2;
					outIndices[curIndex + 2] = curVert + 1;
					outIndices[curIndex + 3] = curVert + 1;
					outIndices[curIndex + 4] = curVert + 2;
					outIndices[curIndex + 5] = curVert + 3;
				}

				offset++;
				quadCount++;

				if(offset > size)
					B3D_EXCEPT(InternalErrorException, "Out of buffer bounds. Buffer size: " + ToString(size));
			}

			penX += word.GetWidth();
		}
		else
		{
			float kerning = 0.0f;
			for(u32 characterIndex = word.GetStartCharacterIndex(); characterIndex <= word.GetEndCharacterIndex(); characterIndex++)
			{
				const CharacterInformation& currentCharacterInformation = mTextData->GetCharacter(characterIndex);

				float curX = penX + currentCharacterInformation.XOffset;
				float curY = mTextData->GetBaselineOffset() - currentCharacterInformation.YOffset;

				penX += currentCharacterInformation.XAdvance + kerning;

				kerning = 0.0f;
				if((characterIndex + 1) <= word.GetEndCharacterIndex())
				{
					const CharacterInformation& nextChar = mTextData->GetCharacter(characterIndex + 1);
					for(size_t j = 0; j < currentCharacterInformation.KerningPairs.size(); j++)
					{
						if(currentCharacterInformation.KerningPairs[j].OtherCharId == nextChar.CharId)
						{
							kerning = currentCharacterInformation.KerningPairs[j].Amount;
							break;
						}
					}
				}

				if(currentCharacterInformation.Page != page)
					continue;

				u32 curVert = offset * 4;
				u32 curIndex = offset * 6;

				outVertices[curVert + 0] = Vector2(curX, curY);
				outVertices[curVert + 1] = Vector2((curX + currentCharacterInformation.Width), curY);
				outVertices[curVert + 2] = Vector2(curX, curY + currentCharacterInformation.Height);
				outVertices[curVert + 3] = Vector2(curX + currentCharacterInformation.Width, curY + currentCharacterInformation.Height);

				if(outUVs != nullptr)
				{
					outUVs[curVert + 0] = Vector2(currentCharacterInformation.UvX, currentCharacterInformation.UvY);
					outUVs[curVert + 1] = Vector2(currentCharacterInformation.UvX + currentCharacterInformation.UvWidth, currentCharacterInformation.UvY);
					outUVs[curVert + 2] = Vector2(currentCharacterInformation.UvX, currentCharacterInformation.UvY + currentCharacterInformation.UvHeight);
					outUVs[curVert + 3] = Vector2(currentCharacterInformation.UvX + currentCharacterInformation.UvWidth, currentCharacterInformation.UvY + currentCharacterInformation.UvHeight);
				}

				if(outIndices != nullptr)
				{
					outIndices[curIndex + 0] = curVert + 0;
					outIndices[curIndex + 1] = curVert + 1;
					outIndices[curIndex + 2] = curVert + 2;
					outIndices[curIndex + 3] = curVert + 1;
					outIndices[curIndex + 4] = curVert + 3;
					outIndices[curIndex + 5] = curVert + 2;
				}

				offset++;
				quadCount++;

				if(offset > size)
					B3D_EXCEPT(InternalErrorException, "Out of buffer bounds. Buffer size: " + ToString(size));
			}
		}
	}

	return quadCount;
}

u32 TextGeometry::Line::GetCharacterCount() const
{
	if(mIsEmpty)
		return 0;

	u32 characterCount = 0;
	for(u32 i = mWordStartIndex; i <= mWordEndIndex; i++)
	{
		Word& word = PerThreadTemporaryBuffer->WordBuffer[i];

		if(word.IsSpacer())
			characterCount++;
		else
			characterCount += word.GetCharacterCount();
	}

	return characterCount;
}

void TextGeometry::Line::CalculateBounds()
{
	mWidth = 0;
	mHeight = 0;

	if(mIsEmpty)
		return;

	for(u32 wordIndex = mWordStartIndex; wordIndex <= mWordEndIndex; wordIndex++)
	{
		Word& word = PerThreadTemporaryBuffer->WordBuffer[wordIndex];

		mWidth += word.GetWidth();
		mHeight = std::max(mHeight, word.GetHeight());
	}
}

TextGeometry::TextGeometry(const U32String& text, const HFont& font, float fontSize, u32 width, u32 height, bool wordWrap, bool wordBreak)
	: mCharacters(nullptr), mCharacterCount(0), mWords(nullptr), mWordCount(0), mLines(nullptr), mLineCount(0), mPageInfos(nullptr), mPageCount(0), mFont(font), mFontBitmapInformation(nullptr)
{
	// In order to reduce number of memory allocations algorithm first calculates data into temporary buffers and then copies the results
	EnsurePerThreadTemporaryBufferIsInitialized();

	if(font != nullptr)
	{
		font->RenderGlyphs(fontSize, TArrayView((u32*)text.data(), text.size()));
		mFontBitmapInformation = font->GetBitmap(fontSize);
	}

	if(mFontBitmapInformation == nullptr)
		return;

	bool widthIsLimited = width > 0;
	mFont = font;

	u32 curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
	float curHeight = mFontBitmapInformation->LineHeight;
	u32 charIdx = 0;

	while(true)
	{
		if(charIdx >= text.size())
			break;

		u32 charId = text[charIdx];
		const CharacterInformation& charDesc = mFontBitmapInformation->GetCharacterInformation(charId);

		Line* curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

		if(text[charIdx] == '\n' || text[charIdx] == '\r')
		{
			curLine->Finalize(true);

			curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
			curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

			curHeight += mFontBitmapInformation->LineHeight;

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
			float widthWithChar = 0.0f;
			if(charIdx == kSpaceChar)
				widthWithChar = curLine->GetWidth() + GetSpaceWidth();
			else if(charIdx == kTabChar)
				widthWithChar = curLine->GetWidth() + GetSpaceWidth() * 4;
			else
				widthWithChar = curLine->CalculateWidthWithChararacter(charDesc);

			if(widthWithChar > (float)width && !curLine->IsEmpty())
			{
				bool atWordBoundary = charId == kSpaceChar || charId == kTabChar || curLine->IsAtWordBoundary();

				if(!atWordBoundary) // Need to break word into multiple pieces, or move it to next line
				{
					u32 lastWordIdx = curLine->RemoveLastWord();
					Word& lastWord = PerThreadTemporaryBuffer->WordBuffer[lastWordIdx];

					bool wordFits = lastWord.CalculateWidthWithCharacter(charDesc) <= (float)width;
					if(wordFits && !curLine->IsEmpty())
					{
						curLine->Finalize(false);

						curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
						curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

						curHeight += mFontBitmapInformation->LineHeight;

						curLine->AddWord(lastWordIdx, lastWord);
					}
					else
					{
						if(wordBreak)
						{
							curLine->AddWord(lastWordIdx, lastWord);
							curLine->Finalize(false);

							curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
							curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

							curHeight += mFontBitmapInformation->LineHeight;
						}
						else
						{
							if(!curLine->IsEmpty()) // Add new line unless current line is empty (to avoid constantly moving the word to new lines)
							{
								curLine->Finalize(false);

								curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
								curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

								curHeight += mFontBitmapInformation->LineHeight;
							}

							curLine->AddWord(lastWordIdx, lastWord);
						}
					}
				}
				else if(charId != kSpaceChar && charId != kTabChar) // If current char is whitespace add it to the existing line even if it doesn't fit
				{
					curLine->Finalize(false);

					curLineIdx = PerThreadTemporaryBuffer->AllocLine(this);
					curLine = &PerThreadTemporaryBuffer->LineBuffer[curLineIdx];

					curHeight += mFontBitmapInformation->LineHeight;
				}
			}
		}

		if(charId == kSpaceChar)
		{
			curLine->AddSpace(GetSpaceWidth());
			PerThreadTemporaryBuffer->AddCharToPage(0, *mFontBitmapInformation);
		}
		else if(charId == kTabChar)
		{
			curLine->AddSpace(GetSpaceWidth() * 4);
			PerThreadTemporaryBuffer->AddCharToPage(0, *mFontBitmapInformation);
		}
		else
		{
			curLine->Add(charIdx, charDesc);
			PerThreadTemporaryBuffer->AddCharToPage(charDesc.Page, *mFontBitmapInformation);
		}

		charIdx++;
	}

	PerThreadTemporaryBuffer->LineBuffer[curLineIdx].Finalize(true);

	// Now that we have all the data we need, allocate the permanent buffers and copy the data
	mCharacterCount = (u32)text.size();
	mWordCount = PerThreadTemporaryBuffer->NextFreeWord;
	mLineCount = PerThreadTemporaryBuffer->NextFreeLine;
	mPageCount = PerThreadTemporaryBuffer->NextFreePageInfo;
}

void TextGeometry::GeneratePersistentData(const U32String& text, u8* buffer, u32& size, bool freeTemporary)
{
	u32 charArraySize = mCharacterCount * sizeof(const CharacterInformation*);
	u32 wordArraySize = mWordCount * sizeof(Word);
	u32 lineArraySize = mLineCount * sizeof(Line);
	u32 pageInfoArraySize = mPageCount * sizeof(PageInfo);

	if(buffer == nullptr)
	{
		size = charArraySize + wordArraySize + lineArraySize + pageInfoArraySize;
		;
		return;
	}

	u8* dataPtr = (u8*)buffer;
	mCharacters = (const CharacterInformation**)dataPtr;

	for(u32 i = 0; i < mCharacterCount; i++)
	{
		u32 charId = text[i];
		const CharacterInformation& charDesc = mFontBitmapInformation->GetCharacterInformation(charId);

		mCharacters[i] = &charDesc;
	}

	dataPtr += charArraySize;
	mWords = (Word*)dataPtr;
	memcpy(mWords, &PerThreadTemporaryBuffer->WordBuffer[0], wordArraySize);

	dataPtr += wordArraySize;
	mLines = (Line*)dataPtr;
	memcpy(mLines, &PerThreadTemporaryBuffer->LineBuffer[0], lineArraySize);

	dataPtr += lineArraySize;
	mPageInfos = (PageInfo*)dataPtr;
	memcpy((void*)mPageInfos, (void*)&PerThreadTemporaryBuffer->PageBuffer[0], pageInfoArraySize);

	if(freeTemporary)
		PerThreadTemporaryBuffer->DeallocAll();
}

const HTexture& TextGeometry::GetTextureForPage(u32 page) const
{
	return mFont->GetPage(page).Texture;
}

float TextGeometry::GetBaselineOffset() const
{
	return mFontBitmapInformation->BaselineOffset;
}

float TextGeometry::GetLineHeight() const
{
	return mFontBitmapInformation->LineHeight;
}

float TextGeometry::GetSpaceWidth() const
{
	return mFontBitmapInformation->SpaceWidth;
}

void TextGeometry::EnsurePerThreadTemporaryBufferIsInitialized()
{
	if(PerThreadTemporaryBuffer == nullptr)
		PerThreadTemporaryBuffer = B3DNew<TemporaryBuffer>();
}

B3D_THREADLOCAL TextGeometry::TemporaryBuffer* TextGeometry::PerThreadTemporaryBuffer = nullptr;

TextGeometry::TemporaryBuffer::TemporaryBuffer()
{
	WordBufferSize = 2000;
	LineBufferSize = 500;
	PageBufferSize = 20;

	NextFreeWord = 0;
	NextFreeLine = 0;
	NextFreePageInfo = 0;

	WordBuffer = B3DNewMultiple<Word>(WordBufferSize);
	LineBuffer = B3DNewMultiple<Line>(LineBufferSize);
	PageBuffer = B3DNewMultiple<PageInfo>(PageBufferSize);
}

TextGeometry::TemporaryBuffer::~TemporaryBuffer()
{
	B3DDeleteMultiple(WordBuffer, WordBufferSize);
	B3DDeleteMultiple(LineBuffer, LineBufferSize);
	B3DDeleteMultiple(PageBuffer, PageBufferSize);
}

u32 TextGeometry::TemporaryBuffer::AllocWord(bool spacer)
{
	if(NextFreeWord >= WordBufferSize)
	{
		u32 newBufferSize = WordBufferSize * 2;
		Word* newBuffer = B3DNewMultiple<Word>(newBufferSize);
		memcpy(newBuffer, WordBuffer, WordBufferSize * sizeof(Word));

		B3DDeleteMultiple(WordBuffer, WordBufferSize);
		WordBuffer = newBuffer;
		WordBufferSize = newBufferSize;
	}

	WordBuffer[NextFreeWord].Initialize(spacer);

	return NextFreeWord++;
}

u32 TextGeometry::TemporaryBuffer::AllocLine(TextGeometry* textData)
{
	if(NextFreeLine >= LineBufferSize)
	{
		u32 newBufferSize = LineBufferSize * 2;
		Line* newBuffer = B3DNewMultiple<Line>(newBufferSize);
		memcpy(newBuffer, LineBuffer, LineBufferSize * sizeof(Line));

		B3DDeleteMultiple(LineBuffer, LineBufferSize);
		LineBuffer = newBuffer;
		LineBufferSize = newBufferSize;
	}

	LineBuffer[NextFreeLine].Initialize(textData);

	return NextFreeLine++;
}

void TextGeometry::TemporaryBuffer::DeallocAll()
{
	NextFreeWord = 0;
	NextFreeLine = 0;
	NextFreePageInfo = 0;
}

void TextGeometry::TemporaryBuffer::AddCharToPage(u32 page, const FontBitmapInformation& fontData)
{
	if(NextFreePageInfo >= PageBufferSize)
	{
		u32 newBufferSize = PageBufferSize * 2;
		PageInfo* newBuffer = B3DNewMultiple<PageInfo>(newBufferSize);
		memcpy(newBuffer, PageBuffer, PageBufferSize * sizeof(PageInfo));

		B3DDeleteMultiple(PageBuffer, PageBufferSize);
		PageBuffer = newBuffer;
		PageBufferSize = newBufferSize;
	}

	while(page >= NextFreePageInfo)
	{
		PageBuffer[NextFreePageInfo].QuadCount = 0;

		NextFreePageInfo++;
	}

	PageBuffer[page].QuadCount++;
}

float TextGeometry::GetWidth() const
{
	float width = 0.0f;

	for(u32 lineIndex = 0; lineIndex < mLineCount; lineIndex++)
		width = std::max(width, mLines[lineIndex].GetWidth());

	return width;
}

float TextGeometry::GetHeight() const
{
	float height = 0.0f;

	for(u32 lineIndex = 0; lineIndex < mLineCount; lineIndex++)
		height += mLines[lineIndex].GetHeight();

	return height;
}
