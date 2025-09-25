//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
	mNumChars = UTF8::Count(mTextDesc.Text);

	mLineDescs.clear();

	B3DMarkAllocatorFrame();
	{
		const U32String utf32text = UTF8::ToUtF32(mTextDesc.Text);
		TTextGeometry<FrameAllocatorTag> textGeometry(utf32text, mTextDesc.Font, mTextDesc.FontSize, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.WordWrap, mTextDesc.WordBreak);

		u32 numLines = textGeometry.GetLineCount();
		u32 numPages = textGeometry.GetPageCount();

		mNumQuads = 0;
		for(u32 i = 0; i < numPages; i++)
			mNumQuads += textGeometry.GetQuadCount(i);

		if(mQuads != nullptr)
			B3DDelete(mQuads);

		mQuads = B3DNewMultiple<Vector2>(mNumQuads * 4);

		TextSprite::BuildTextQuads(textGeometry, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.HorzAlign, mTextDesc.VertAlign, mTextDesc.Anchor, mQuads, nullptr, nullptr, mNumQuads);

		// Store cached line data
		u32 curCharIdx = 0;
		u32 curLineIdx = 0;

		Vector2I* alignmentOffsets = B3DFrameNew<Vector2I>(numLines);
		TextSprite::GetAlignmentOffsets(textGeometry, (u32)mTextDesc.Size.Width, (u32)mTextDesc.Size.Height, mTextDesc.HorzAlign, mTextDesc.VertAlign, alignmentOffsets);

		for(u32 i = 0; i < numLines; i++)
		{
			const TextGeometry::Line& line = textGeometry.GetLine(i);

			// Line has a newline char only if it wasn't created by word wrap and it isn't the last line
			bool hasNewline = line.HasNewlineChar() && (curLineIdx != (numLines - 1));

			u32 startChar = curCharIdx;
			u32 endChar = curCharIdx + line.GetCharacterCount() + (hasNewline ? 1 : 0);
			u32 lineHeight = Math::RoundToU32(line.GetYOffset());
			i32 lineYStart = alignmentOffsets[curLineIdx].Y;

			GUIInputLineDesc lineDesc(startChar, endChar, lineHeight, lineYStart, hasNewline);
			mLineDescs.push_back(lineDesc);

			curCharIdx = lineDesc.GetEndChar();
			curLineIdx++;
		}

		B3DFrameDelete(alignmentOffsets);
	}
	B3DClearAllocatorFrame();
}

Area2I GUIInputTool::GetCharacterBounds(u32 characterIndex) const
{
	u32 lineIdx = GetLineForChar(characterIndex);

	// If char is newline we don't have any geometry to return
	const GUIInputLineDesc& lineDesc = GetLineDesc(lineIdx);
	if(lineDesc.IsNewline(characterIndex))
		return Area2I();

	u32 numNewlineChars = 0;
	for(u32 i = 0; i < lineIdx; i++)
		numNewlineChars += (GetLineDesc(i).HasNewlineChar() ? 1 : 0);

	i32 quadIdx = (i32)(characterIndex - numNewlineChars);
	if(quadIdx >= 0 && quadIdx < (i32)mNumQuads)
	{
		u32 vertIdx = quadIdx * 4;

		Area2I charRect;
		charRect.X = Math::RoundToI32(mQuads[vertIdx + 0].X);
		charRect.Y = Math::RoundToI32(mQuads[vertIdx + 0].Y);
		charRect.Width = Math::RoundToI32(mQuads[vertIdx + 3].X - charRect.X);
		charRect.Height = Math::RoundToI32(mQuads[vertIdx + 3].Y - charRect.Y);

		return charRect;
	}

	B3D_LOG(Error, GUI, "Invalid character index: {0}", characterIndex);
	return Area2I();
}

i32 GUIInputTool::GetCharIdxAtPos(const GUIPhysicalPoint& pos) const
{
	Vector2 vecPos = pos.To<float>();

	u32 lineStartChar = 0;
	u32 lineEndChar = 0;
	u32 numNewlineChars = 0;
	u32 lineIdx = 0;
	for(auto& line : mLineDescs)
	{
		i32 lineStart = line.GetLineYStart();
		if(pos.Y >= lineStart && pos.Y < (lineStart + (i32)line.GetLineHeight()))
		{
			lineStartChar = line.GetStartChar();
			lineEndChar = line.GetEndChar(false);
			break;
		}

		// Newline chars count in the startChar/endChar variables, but don't actually exist in the buffers
		// so we need to filter them out
		numNewlineChars += (line.HasNewlineChar() ? 1 : 0);

		lineIdx++;
	}

	u32 lineStartQuad = lineStartChar - numNewlineChars;
	u32 lineEndQuad = lineEndChar - numNewlineChars;

	float nearestDist = std::numeric_limits<float>::max();
	u32 nearestChar = 0;
	bool foundChar = false;

	for(u32 i = lineStartQuad; i < lineEndQuad; i++)
	{
		u32 curVert = i * 4;

		float centerX = mQuads[curVert + 0].X + mQuads[curVert + 1].X;
		centerX *= 0.5f;

		float dist = Math::Abs(centerX - vecPos.X);
		if(dist < nearestDist)
		{
			nearestChar = i + numNewlineChars;
			nearestDist = dist;
			foundChar = true;
		}
	}

	if(!foundChar)
		return -1;

	return nearestChar;
}

u32 GUIInputTool::GetLineForChar(u32 charIdx, bool newlineCountsOnNextLine) const
{
	u32 idx = 0;
	for(auto& line : mLineDescs)
	{
		if((charIdx >= line.GetStartChar() && charIdx < line.GetEndChar()) ||
		   (charIdx == line.GetStartChar() && line.GetStartChar() == line.GetEndChar()))
		{
			if(line.IsNewline(charIdx) && newlineCountsOnNextLine)
				return idx + 1; // Incrementing is safe because next line must exist, since we just found a newline char

			return idx;
		}

		idx++;
	}

	B3D_LOG(Error, GUI, "Invalid character index: {0}", charIdx);
	return 0;
}

u32 GUIInputTool::GetCharIdxAtInputIdx(u32 inputIdx) const
{
	if(mNumChars == 0)
		return 0;

	u32 numLines = GetNumLines();
	u32 curPos = 0;
	u32 curCharIdx = 0;
	for(u32 i = 0; i < numLines; i++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(i);

		if(curPos == inputIdx)
			return lineDesc.GetStartChar();

		curPos++; // Move past line start position

		u32 numChars = lineDesc.GetEndChar() - lineDesc.GetStartChar();
		u32 numCaretPositions = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
		if(inputIdx >= (curPos + numCaretPositions))
		{
			curCharIdx += numChars;
			curPos += numCaretPositions;
			continue;
		}

		u32 diff = inputIdx - curPos;
		curCharIdx += diff + 1; // Character after the caret

		return curCharIdx;
	}

	return 0;
}

bool GUIInputTool::IsNewline(u32 inputIdx) const
{
	if(mNumChars == 0)
		return true;

	u32 numLines = GetNumLines();
	u32 curPos = 0;
	for(u32 i = 0; i < numLines; i++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(i);

		if(curPos == inputIdx)
			return true;

		u32 numChars = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
		curPos += numChars;
	}

	return false;
}

bool GUIInputTool::IsNewlineChar(u32 charIdx) const
{
	u32 byteIdx = UTF8::CharToByteIndex(mTextDesc.Text, charIdx);

	return mTextDesc.Text[byteIdx] == '\n';
}

bool GUIInputTool::IsDescValid() const
{
	// We we have some text but line descs are empty we may assume
	// something went wrong when creating the line descs, therefore it is
	// not valid and no text is displayed.
	if(mNumChars > 0)
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

bool GUIInputLineDesc::IsNewline(u32 charIdx) const
{
	if(mIncludesNewline)
	{
		return (mEndChar - 1) == charIdx;
	}
	else
		return false;
}
