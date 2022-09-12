//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInputTool.h"
#include "GUI/BsGUIElement.h"
#include "Math/BsMath.h"
#include "Math/BsVector2.h"
#include "Text/BsFont.h"
#include "String/BsUnicode.h"

namespace bs
{
	void GUIInputTool::UpdateText(const GUIElement* element, const TEXT_SPRITE_DESC& textDesc)
	{
		mElement = element;
		mTextDesc = textDesc;
		mNumChars = UTF8::count(mTextDesc.text);

		mLineDescs.Clear();

		bs_frame_mark();
		{
			const U32String utf32text = UTF8::toUTF32(mTextDesc.text);
			TextData<FrameAlloc> textData(utf32text, mTextDesc.font, mTextDesc.fontSize,
				mTextDesc.width, mTextDesc.height, mTextDesc.wordWrap, mTextDesc.wordBreak);

			UINT32 numLines = textData.GetNumLines();
			UINT32 numPages = textData.GetNumPages();

			mNumQuads = 0;
			for (UINT32 i = 0; i < numPages; i++)
				mNumQuads += textData.GetNumQuadsForPage(i);

			if (mQuads != nullptr)
				bs_delete(mQuads);

			mQuads = bs_newN<Vector2>(mNumQuads * 4);

			TextSprite::genTextQuads(textData, mTextDesc.width, mTextDesc.height, mTextDesc.horzAlign, mTextDesc.vertAlign, mTextDesc.anchor,
				mQuads, nullptr, nullptr, mNumQuads);

			// Store cached line data
			UINT32 curCharIdx = 0;
			UINT32 curLineIdx = 0;

			Vector2I* alignmentOffsets = bs_frame_new<Vector2I>(numLines);
			TextSprite::getAlignmentOffsets(textData, mTextDesc.width, mTextDesc.height, mTextDesc.horzAlign,
				mTextDesc.vertAlign, alignmentOffsets);

			for (UINT32 i = 0; i < numLines; i++)
			{
				const TextDataBase::TextLine& line = textData.GetLine(i);

				// Line has a newline char only if it wasn't created by word wrap and it isn't the last line
				bool hasNewline = line.HasNewlineChar() && (curLineIdx != (numLines - 1));

				UINT32 startChar = curCharIdx;
				UINT32 endChar = curCharIdx + line.GetNumChars() + (hasNewline ? 1 : 0);
				UINT32 lineHeight = line.GetYOffset();
				INT32 lineYStart = alignmentOffsets[curLineIdx].y;

				GUIInputLineDesc LineDesc(startChar, endChar, lineHeight, lineYStart, hasNewline);
				mLineDescs.push_back(lineDesc);

				curCharIdx = lineDesc.GetEndChar();
				curLineIdx++;
			}

			bs_frame_delete(alignmentOffsets);
		}
		bs_frame_clear();
	}

	Vector2I GUIInputTool::GetTextOffset() const
	{
		Vector2I Offset(mElement->_getLayoutData().area.x, mElement->_getLayoutData().area.y);

		return offset + mElement->_getTextInputOffset() + Vector2I(mElement->_getTextInputRect().x, mElement->_getTextInputRect().y);
	}

	Rect2I GUIInputTool::GetCharRect(UINT32 charIdx) const
	{
		Rect2I charRect = getLocalCharRect(charIdx);
		Vector2I textOffset = getTextOffset();

		charRect.x += textOffset.x;
		charRect.y += textOffset.y;

		return charRect;
	}

	Rect2I GUIInputTool::GetLocalCharRect(UINT32 charIdx) const
	{
		UINT32 lineIdx = getLineForChar(charIdx);

		// If char is newline we don't have any geometry to return
		const GUIInputLineDesc& lineDesc = getLineDesc(lineIdx);
		if(lineDesc.IsNewline(charIdx))
			return Rect2I();

		UINT32 numNewlineChars = 0;
		for(UINT32 i = 0; i < lineIdx; i++)
			numNewlineChars += (getLineDesc(i).HasNewlineChar() ? 1 : 0);

		INT32 quadIdx = (INT32)(charIdx - numNewlineChars);
		if(quadIdx >= 0 && quadIdx < (INT32)mNumQuads)
		{
			UINT32 vertIdx = quadIdx * 4;

			Rect2I charRect;
			charRect.x = Math::roundToInt(mQuads[vertIdx + 0].x);
			charRect.y = Math::roundToInt(mQuads[vertIdx + 0].y);
			charRect.width = Math::roundToInt(mQuads[vertIdx + 3].x - charRect.x);
			charRect.height = Math::roundToInt(mQuads[vertIdx + 3].y - charRect.y);

			return charRect;
		}

		BS_LOG(Error, GUI, "Invalid character index: {0}", charIdx);
		return Rect2I();
	}

	INT32 GUIInputTool::GetCharIdxAtPos(const Vector2I& pos) const
	{
		Vector2 VecPos((float)pos.x, (float)pos.y);

		UINT32 lineStartChar = 0;
		UINT32 lineEndChar = 0;
		UINT32 numNewlineChars = 0;
		UINT32 lineIdx = 0;
		for(auto& line : mLineDescs)
		{
			INT32 lineStart = line.GetLineYStart() + getTextOffset().y;
			if(pos.y >= lineStart && pos.y < (lineStart + (INT32)line.GetLineHeight()))
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

		UINT32 lineStartQuad = lineStartChar - numNewlineChars;
		UINT32 lineEndQuad = lineEndChar - numNewlineChars;

		float nearestDist = std::numeric_limits<float>::max();
		UINT32 nearestChar = 0;
		bool foundChar = false;

		Vector2I textOffset = getTextOffset();
		for(UINT32 i = lineStartQuad; i < lineEndQuad; i++)
		{
			UINT32 curVert = i * 4;

			float centerX = mQuads[curVert + 0].x + mQuads[curVert + 1].x;
			centerX *= 0.5f;
			centerX += textOffset.x;

			float dist = Math::abs(centerX - vecPos.x);
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

	UINT32 GUIInputTool::GetLineForChar(UINT32 charIdx, bool newlineCountsOnNextLine) const
	{
		UINT32 idx = 0;
		for(auto& line : mLineDescs)
		{
			if((charIdx >= line.GetStartChar() && charIdx < line.getEndChar()) ||
				(charIdx == line.GetStartChar() && line.getStartChar() == line.getEndChar()))
			{
				if(line.IsNewline(charIdx) && newlineCountsOnNextLine)
					return idx + 1; // Incrementing is safe because next line must exist, since we just found a newline char

				return idx;
			}

			idx++;
		}

		BS_LOG(Error, GUI, "Invalid character index: {0}", charIdx);
		return 0;
	}

	UINT32 GUIInputTool::GetCharIdxAtInputIdx(UINT32 inputIdx) const
	{
		if(mNumChars == 0)
			return 0;

		UINT32 numLines = getNumLines();
		UINT32 curPos = 0;
		UINT32 curCharIdx = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(i);

			if(curPos == inputIdx)
				return lineDesc.GetStartChar();

			curPos++; // Move past line start position

			UINT32 numChars = lineDesc.GetEndChar() - lineDesc.getStartChar();
			UINT32 numCaretPositions = lineDesc.GetEndChar(false) - lineDesc.getStartChar();
			if(inputIdx >= (curPos + numCaretPositions))
			{
				curCharIdx += numChars;
				curPos += numCaretPositions;
				continue;
			}

			UINT32 diff = inputIdx - curPos;
			curCharIdx += diff + 1; // Character after the caret

			return curCharIdx;
		}

		return 0;
	}

	bool GUIInputTool::IsNewline(UINT32 inputIdx) const
	{
		if(mNumChars == 0)
			return true;

		UINT32 numLines = getNumLines();
		UINT32 curPos = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(i);

			if(curPos == inputIdx)
				return true;

			UINT32 numChars = lineDesc.GetEndChar(false) - lineDesc.getStartChar();
			curPos += numChars;
		}

		return false;
	}

	bool GUIInputTool::IsNewlineChar(UINT32 charIdx) const
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mTextDesc.text, charIdx);

		return mTextDesc.text[byteIdx] == '\n';
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

	GUIInputLineDesc::GUIInputLineDesc(UINT32 startChar, UINT32 endChar, UINT32 lineHeight, INT32 lineYStart, bool includesNewline)
		:mStartChar(startChar), mEndChar(endChar), mLineHeight(lineHeight), mLineYStart(lineYStart), mIncludesNewline(includesNewline)
	{

	}

	UINT32 GUIInputLineDesc::GetEndChar(bool includeNewline) const
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

	bool GUIInputLineDesc::IsNewline(UINT32 charIdx) const
	{
		if(mIncludesNewline)
		{
			return (mEndChar - 1) == charIdx;
		}
		else
			return false;
	}
}
