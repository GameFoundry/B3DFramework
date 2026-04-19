//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIInputTool.h"
#include "GUI/B3DGUIInteractable.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector2.h"
#include "Text/B3DFont.h"
#include "String/B3DUnicode.h"

using namespace b3d;

void GUIInputTool::UpdateText(const GUIInteractable* element, const TextSpriteInformation& textDesc)
{
	mElement = element;
	mTextDesc = textDesc;
	mCharacterCount = UTF8::Count(mTextDesc.Text);

	mLineDescs.clear();

	B3DMarkAllocatorFrame();
	{
		const U32String utf32text = UTF8::ToUtF32(mTextDesc.Text);
		TTextGeometry<FrameAllocatorTag> textGeometry(utf32text, mTextDesc.Font, mTextDesc.FontSize, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.WordWrap, mTextDesc.WordBreak);

		u32 lineCount = textGeometry.GetLineCount();
		u32 pageCount = textGeometry.GetPageCount();

		mQuadCount = 0;
		for(u32 pageIndex = 0; pageIndex < pageCount; pageIndex++)
			mQuadCount += textGeometry.GetQuadCount(pageIndex);

		if(mQuads != nullptr)
			B3DDelete(mQuads);

		mQuads = B3DNewMultiple<Vector2>(mQuadCount * 4);

		TextSprite::BuildTextQuads(textGeometry, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.HorzAlign, mTextDesc.VertAlign, mTextDesc.Anchor, mQuads, nullptr, nullptr, mQuadCount);

		// Store cached line data
		u32 currentCharacterIndex = 0;
		u32 currentLineIndex = 0;

		Vector2I* alignmentOffsets = B3DFrameNew<Vector2I>(lineCount);
		TextSprite::GetAlignmentOffsets(textGeometry, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.HorzAlign, mTextDesc.VertAlign, alignmentOffsets);

		for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			const TextGeometry::Line& line = textGeometry.GetLine(lineIndex);

			// Line has a newline char only if it wasn't created by word wrap and it isn't the last line
			bool hasNewline = line.HasNewlineChar() && (currentLineIndex != (lineCount - 1));

			u32 startChar = currentCharacterIndex;
			u32 endChar = currentCharacterIndex + line.GetCharacterCount() + (hasNewline ? 1 : 0);
			u32 lineHeight = Math::RoundToU32(line.GetYOffset());
			i32 lineYStart = alignmentOffsets[currentLineIndex].Y;

			GUIInputLineDesc lineDesc(startChar, endChar, lineHeight, lineYStart, hasNewline);
			mLineDescs.push_back(lineDesc);

			currentCharacterIndex = lineDesc.GetEndChar();
			currentLineIndex++;
		}

		B3DFrameDelete(alignmentOffsets);
	}
	B3DClearAllocatorFrame();
}

Area2I GUIInputTool::GetCharacterBounds(u32 characterIndex) const
{
	u32 lineIndex = GetLineForChar(characterIndex);

	// If char is newline we don't have any geometry to return
	const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);
	if(lineDesc.IsNewline(characterIndex))
		return Area2I();

	u32 newlineCharacterCount = 0;
	for(u32 previousLineIndex = 0; previousLineIndex < lineIndex; previousLineIndex++)
		newlineCharacterCount += (GetLineDesc(previousLineIndex).HasNewlineChar() ? 1 : 0);

	i32 quadIndex = (i32)(characterIndex - newlineCharacterCount);
	if(quadIndex >= 0 && quadIndex < (i32)mQuadCount)
	{
		u32 vertexIndex = quadIndex * 4;

		Area2I charRect;
		charRect.X = Math::RoundToI32(mQuads[vertexIndex + 0].X);
		charRect.Y = Math::RoundToI32(mQuads[vertexIndex + 0].Y);
		charRect.Width = Math::RoundToI32(mQuads[vertexIndex + 3].X - charRect.X);
		charRect.Height = Math::RoundToI32(mQuads[vertexIndex + 3].Y - charRect.Y);

		return charRect;
	}

	B3D_LOG(Error, LogGUI, "Invalid character index: {0}", characterIndex);
	return Area2I();
}

i32 GUIInputTool::GetCharIdxAtPos(const GUIPhysicalPoint& position) const
{
	Vector2 vectorPosition = position.To<float>();

	u32 lineStartChar = 0;
	u32 lineEndChar = 0;
	u32 newlineCharacterCount = 0;
	u32 lineIndex = 0;
	for(auto& line : mLineDescs)
	{
		i32 lineStart = line.GetLineYStart();
		if(position.Y >= lineStart && position.Y < (lineStart + (i32)line.GetLineHeight()))
		{
			lineStartChar = line.GetStartChar();
			lineEndChar = line.GetEndChar(false);
			break;
		}

		// Newline chars count in the startChar/endChar variables, but don't actually exist in the buffers
		// so we need to filter them out
		newlineCharacterCount += (line.HasNewlineChar() ? 1 : 0);

		lineIndex++;
	}

	u32 lineStartQuad = lineStartChar - newlineCharacterCount;
	u32 lineEndQuad = lineEndChar - newlineCharacterCount;

	float nearestDistance = std::numeric_limits<float>::max();
	u32 nearestCharacter = 0;
	bool foundCharacter = false;

	for(u32 quadIndex = lineStartQuad; quadIndex < lineEndQuad; quadIndex++)
	{
		u32 currentVertex = quadIndex * 4;

		float centerX = mQuads[currentVertex + 0].X + mQuads[currentVertex + 1].X;
		centerX *= 0.5f;

		float distance = Math::Abs(centerX - vectorPosition.X);
		if(distance < nearestDistance)
		{
			nearestCharacter = quadIndex + newlineCharacterCount;
			nearestDistance = distance;
			foundCharacter = true;
		}
	}

	if(!foundCharacter)
		return -1;

	return nearestCharacter;
}

u32 GUIInputTool::GetLineForChar(u32 characterIndex, bool newlineCountsOnNextLine) const
{
	u32 lineIndex = 0;
	for(auto& line : mLineDescs)
	{
		if((characterIndex >= line.GetStartChar() && characterIndex < line.GetEndChar()) ||
		   (characterIndex == line.GetStartChar() && line.GetStartChar() == line.GetEndChar()))
		{
			if(line.IsNewline(characterIndex) && newlineCountsOnNextLine)
				return lineIndex + 1; // Incrementing is safe because next line must exist, since we just found a newline char

			return lineIndex;
		}

		lineIndex++;
	}

	B3D_LOG(Error, LogGUI, "Invalid character index: {0}", characterIndex);
	return 0;
}

u32 GUIInputTool::GetCharIdxAtInputIdx(u32 inputIdx) const
{
	if(mCharacterCount == 0)
		return 0;

	u32 lineCount = GetLineCount();
	u32 currentPosition = 0;
	u32 currentCharacterIndex = 0;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);

		if(currentPosition == inputIdx)
			return lineDesc.GetStartChar();

		currentPosition++; // Move past line start position

		u32 characterCount = lineDesc.GetEndChar() - lineDesc.GetStartChar();
		u32 caretPositionCount = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
		if(inputIdx >= (currentPosition + caretPositionCount))
		{
			currentCharacterIndex += characterCount;
			currentPosition += caretPositionCount;
			continue;
		}

		u32 difference = inputIdx - currentPosition;
		currentCharacterIndex += difference + 1; // Character after the caret

		return currentCharacterIndex;
	}

	return 0;
}

bool GUIInputTool::IsNewline(u32 inputIdx) const
{
	if(mCharacterCount == 0)
		return true;

	u32 lineCount = GetLineCount();
	u32 currentPosition = 0;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);

		if(currentPosition == inputIdx)
			return true;

		u32 characterCount = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
		currentPosition += characterCount;
	}

	return false;
}

bool GUIInputTool::IsNewlineChar(u32 characterIndex) const
{
	u32 byteIndex = UTF8::CharToByteIndex(mTextDesc.Text, characterIndex);

	return mTextDesc.Text[byteIndex] == '\n';
}

bool GUIInputTool::IsDescValid() const
{
	// We we have some text but line descs are empty we may assume
	// something went wrong when creating the line descs, therefore it is
	// not valid and no text is displayed.
	if(mCharacterCount > 0)
		return !mLineDescs.empty();

	return true;
}

GUIInputLineDesc::GUIInputLineDesc(u32 startChar, u32 endChar, u32 lineHeight, i32 lineYStart, bool includesNewline)
	: mStartChar(startChar), mEndChar(endChar), mLineHeight(lineHeight), mLineYStart(lineYStart), mIncludesNewline(includesNewline)
{
}

u32 GUIInputLineDesc::GetEndChar(bool includeNewline) const
{
	if(mIncludesNewline)
	{
		if(includeNewline)
			return mEndChar;
		else
		{
			if(mEndChar > 0)
				return mEndChar - 1;
			else
				return mStartChar;
		}
	}
	else
		return mEndChar;
}

bool GUIInputLineDesc::IsNewline(u32 characterIndex) const
{
	if(mIncludesNewline)
	{
		return (mEndChar - 1) == characterIndex;
	}
	else
		return false;
}
