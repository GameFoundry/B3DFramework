//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIInputCaret.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUIManager.h"
#include "2D/B3DImageSprite.h"
#include "GUI/B3DGUIInteractable.h"
#include "Text/B3DFont.h"

using namespace b3d;

GUIInputCaret::GUIInputCaret()
{
	mCaretSprite = B3DNew<ImageSprite>();
}

GUIInputCaret::~GUIInputCaret()
{
	B3DDelete(mCaretSprite);
}

GUIPhysicalArea GUIInputCaret::GetBounds() const
{
	const GUIPhysicalPoint caretPosition = GetCaretPosition();
	return GUIPhysicalArea(caretPosition.X, caretPosition.Y, 1, GetCaretHeight());
}

void GUIInputCaret::UpdateSprite()
{
	ImageSpriteInformation createSpriteInformation;
	createSpriteInformation.Size = Size2I(1, (i32)GetCaretHeight());
	createSpriteInformation.Image = GUIManager::Instance().GetCaretTexture();

	GUIWidget* widget = nullptr;
	if(mElement != nullptr)
		widget = mElement->GetParentWidget();

	mCaretSprite->Update(createSpriteInformation, (u64)widget);
}

void GUIInputCaret::MoveCaretToStart()
{
	mCaretPos = 0;
}

void GUIInputCaret::MoveCaretToEnd()
{
	mCaretPos = GetMaxCaretPos();
}

void GUIInputCaret::MoveCaretLeft()
{
	mCaretPos = (u32)std::max(0, (i32)mCaretPos - 1);
}

void GUIInputCaret::MoveCaretRight()
{
	u32 maxCaretPos = GetMaxCaretPos();

	mCaretPos = std::min(maxCaretPos, mCaretPos + 1);
}

void GUIInputCaret::MoveCaretUp()
{
	u32 characterIndex = GetCharIdxAtCaretPos();
	if(characterIndex > 0)
		characterIndex -= 1;

	u32 lineIndex = GetLineForChar(characterIndex);
	const GUIInputLineDesc& desc = GetLineDesc(lineIndex);
	// If char is a newline, I want that to count as being on the next line because that's
	// how user sees it
	if(desc.IsNewline(characterIndex))
		lineIndex++;

	if(lineIndex == 0)
	{
		MoveCaretToStart();
		return;
	}

	GUIPhysicalPoint caretCoords = GetCaretPosition();
	caretCoords.Y -= GetCaretHeight();

	MoveCaretToPos(caretCoords);
}

void GUIInputCaret::MoveCaretDown()
{
	u32 characterIndex = GetCharIdxAtCaretPos();
	if(characterIndex > 0)
		characterIndex -= 1;

	u32 lineIndex = GetLineForChar(characterIndex);
	const GUIInputLineDesc& desc = GetLineDesc(lineIndex);
	// If char is a newline, I want that to count as being on the next line because that's
	// how user sees it
	if(desc.IsNewline(characterIndex))
		lineIndex++;

	if(lineIndex == (GetLineCount() - 1))
	{
		MoveCaretToEnd();
		return;
	}

	GUIPhysicalPoint caretCoords = GetCaretPosition();
	caretCoords.Y += GetCaretHeight();

	MoveCaretToPos(caretCoords);
}

void GUIInputCaret::MoveCaretToPos(const GUIPhysicalPoint& pos)
{
	i32 characterIndex = GetCharIdxAtPos(pos);

	if(characterIndex != -1)
	{
		Area2I charRect = GetCharacterBounds(characterIndex);

		float xCenter = (float)charRect.X + (float)charRect.Width * 0.5f;
		if((float)pos.X <= xCenter)
			MoveCaretToChar(characterIndex, CARET_BEFORE);
		else
			MoveCaretToChar(characterIndex, CARET_AFTER);
	}
	else
	{
		u32 lineCount = GetLineCount();

		if(lineCount == 0)
		{
			mCaretPos = 0;
			return;
		}

		u32 curPos = 0;
		for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			const GUIInputLineDesc& line = GetLineDesc(lineIndex);

			i32 lineStart = line.GetLineYStart();
			if(pos.Y >= lineStart && pos.Y < (lineStart + (i32)line.GetLineHeight()))
			{
				mCaretPos = curPos;
				return;
			}

			u32 characterCount = line.GetEndChar(false) - line.GetStartChar() + 1; // +1 For extra line start position
			curPos += characterCount;
		}

		{
			const GUIInputLineDesc& firstLine = GetLineDesc(0);
			i32 lineStart = firstLine.GetLineYStart();

			if(pos.Y < lineStart) // Before first line
				mCaretPos = 0;
			else // After last line
				mCaretPos = curPos - 1;
		}
	}
}

void GUIInputCaret::MoveCaretToChar(u32 characterIndex, CaretPos caretPos)
{
	if(characterIndex >= mCharacterCount)
	{
		mCaretPos = 0;
		return;
	}

	u32 lineCount = GetLineCount();
	u32 curPos = 0;
	u32 curCharIdx = 0;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);

		curPos++; // Move past line start position

		u32 characterCount = lineDesc.GetEndChar() - lineDesc.GetStartChar();
		u32 caretPositionCount = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
		if(characterIndex >= (curCharIdx + characterCount))
		{
			curCharIdx += characterCount;
			curPos += caretPositionCount;
			continue;
		}

		u32 diff = characterIndex - curCharIdx;

		if(caretPos == CARET_BEFORE)
			curPos += diff - 1;
		else
			curPos += diff;

		break;
	}

	mCaretPos = curPos;
}

u32 GUIInputCaret::GetCharIdxAtCaretPos() const
{
	return GetCharIdxAtInputIdx(mCaretPos);
}

GUIPhysicalPoint GUIInputCaret::GetCaretPosition() const
{
	if(mCharacterCount > 0 && IsDescValid())
	{
		u32 curPos = 0;
		u32 lineCount = GetLineCount();

		for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);

			if(mCaretPos == curPos)
			{
				// Caret is on line start
				return GUIPhysicalPoint(0, lineDesc.GetLineYStart());
			}

			curPos += lineDesc.GetEndChar(false) - lineDesc.GetStartChar() + 1; // + 1 for special line start position
		}

		u32 characterIndex = GetCharIdxAtCaretPos();
		if(characterIndex > 0)
			characterIndex -= 1;

		characterIndex = std::min((u32)(mCharacterCount - 1), characterIndex);

		Area2I charRect = GetCharacterBounds(characterIndex);
		u32 lineIndex = GetLineForChar(characterIndex);
		u32 yOffset = GetLineDesc(lineIndex).GetLineYStart();

		return GUIPhysicalPoint(charRect.X + charRect.Width, yOffset);
	}

	return GUIPhysicalPoint(0, 0);
}

GUIPhysicalUnit GUIInputCaret::GetCaretHeight() const
{
	u32 characterIndex = GetCharIdxAtCaretPos();
	if(characterIndex > 0)
		characterIndex -= 1;

	if(characterIndex < mCharacterCount && IsDescValid())
	{
		u32 lineIndex = GetLineForChar(characterIndex);
		return (i32)GetLineDesc(lineIndex).GetLineHeight();
	}
	else
	{
		if(mTextDesc.Font != nullptr)
		{
			const float nearestSize = mTextDesc.Font->GetClosestExistingBitmapSize(mTextDesc.FontSize);
			SPtr<const FontBitmapInformation> fontData = mTextDesc.Font->GetBitmap(nearestSize);

			if(fontData != nullptr)
				return Math::RoundToI32(fontData->LineHeight);
		}
	}

	return 0;
}

bool GUIInputCaret::IsCaretAtNewline() const
{
	return IsNewline(mCaretPos);
}

u32 GUIInputCaret::GetMaxCaretPos() const
{
	if(mCharacterCount == 0)
		return 0;

	u32 lineCount = GetLineCount();
	u32 maxPos = 0;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);

		u32 characterCount = lineDesc.GetEndChar(false) - lineDesc.GetStartChar() + 1; // + 1 for special line start position
		maxPos += characterCount;
	}

	return maxPos - 1;
}
