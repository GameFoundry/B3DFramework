//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInputCaret.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIManager.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUIElement.h"
#include "Text/BsFont.h"

namespace bs
{
	GUIInputCaret::GUIInputCaret()
	{
		mCaretSprite = bs_new<ImageSprite>();
	}

	GUIInputCaret::~GUIInputCaret()
	{
		bs_delete(mCaretSprite);
	}

	Vector2I GUIInputCaret::GetSpriteOffset() const
	{
		return GetCaretPosition(GetTextOffset());
	}

	Rect2I GUIInputCaret::GetSpriteClipRect(const Rect2I& parentClipRect) const
	{
		Vector2I offset(mElement->GetLayoutDataInternal().Area.X, mElement->GetLayoutDataInternal().Area.Y);

		Vector2I clipOffset = GetSpriteOffset() - offset -
			Vector2I(mElement->GetTextInputRectInternal().X, mElement->GetTextInputRectInternal().Y);

		Rect2I clipRect(-clipOffset.X, -clipOffset.Y, mTextDesc.Width, mTextDesc.Height);

		Rect2I localParentCliprect = parentClipRect;

		// Move parent rect to our space
		localParentCliprect.X += mElement->GetTextInputOffsetInternal().X + clipRect.X;
		localParentCliprect.Y += mElement->GetTextInputOffsetInternal().Y + clipRect.Y;

		// Clip our rectangle so its not larger then the parent
		clipRect.Clip(localParentCliprect);

		// Increase clip size by 1, so we can fit the caret in case it is fully at the end of the text
		clipRect.Width += 1;

		return clipRect;
	}

	void GUIInputCaret::UpdateSprite()
	{
		IMAGE_SPRITE_DESC mCaretDesc;
		mCaretDesc.Width = 1;
		mCaretDesc.Height = GetCaretHeight();
		mCaretDesc.Texture = GUIManager::Instance().GetCaretTexture();

		GUIWidget* widget = nullptr;
		if(mElement != nullptr)
			widget = mElement->GetParentWidgetInternal();

		mCaretSprite->Update(mCaretDesc, (u64)widget);
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
		u32 charIdx = GetCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;

		u32 lineIdx = GetLineForChar(charIdx);
		const GUIInputLineDesc& desc = GetLineDesc(lineIdx);
		// If char is a newline, I want that to count as being on the next line because that's
		// how user sees it
		if(desc.IsNewline(charIdx))
			lineIdx++;

		if(lineIdx == 0)
		{
			MoveCaretToStart();
			return;
		}

		Vector2I caretCoords = GetCaretPosition(mElement->GetTextInputOffsetInternal());
		caretCoords.Y -= GetCaretHeight();

		MoveCaretToPos(caretCoords);
	}

	void GUIInputCaret::MoveCaretDown()
	{
		u32 charIdx = GetCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;

		u32 lineIdx = GetLineForChar(charIdx);
		const GUIInputLineDesc& desc = GetLineDesc(lineIdx);
		// If char is a newline, I want that to count as being on the next line because that's
		// how user sees it
		if(desc.IsNewline(charIdx))
			lineIdx++;

		if(lineIdx == (GetNumLines() - 1))
		{
			MoveCaretToEnd();
			return;
		}

		Vector2I caretCoords = GetCaretPosition(mElement->GetTextInputOffsetInternal());
		caretCoords.Y += GetCaretHeight();

		MoveCaretToPos(caretCoords);
	}

	void GUIInputCaret::MoveCaretToPos(const Vector2I& pos)
	{
		i32 charIdx = GetCharIdxAtPos(pos);

		if(charIdx != -1)
		{
			Rect2I charRect = GetCharRect(charIdx);

			float xCenter = charRect.X + charRect.Width * 0.5f;
			if(pos.X <= xCenter)
				MoveCaretToChar(charIdx, CARET_BEFORE);
			else
				MoveCaretToChar(charIdx, CARET_AFTER);
		}
		else
		{
			u32 numLines = GetNumLines();

			if(numLines == 0)
			{
				mCaretPos = 0;
				return;
			}

			u32 curPos = 0;
			for(u32 i = 0; i < numLines; i++)
			{
				const GUIInputLineDesc& line = GetLineDesc(i);

				i32 lineStart = line.GetLineYStart() + GetTextOffset().Y;
				if(pos.Y >= lineStart && pos.Y < (lineStart + (i32)line.GetLineHeight()))
				{
					mCaretPos = curPos;
					return;
				}

				u32 numChars = line.GetEndChar(false) - line.GetStartChar() + 1; // +1 For extra line start position
				curPos += numChars;
			}

			{
				const GUIInputLineDesc& firstLine = GetLineDesc(0);
				i32 lineStart = firstLine.GetLineYStart() + GetTextOffset().Y;

				if(pos.Y < lineStart) // Before first line
					mCaretPos = 0;
				else // After last line
					mCaretPos = curPos - 1;
			}
		}
	}

	void GUIInputCaret::MoveCaretToChar(u32 charIdx, CaretPos caretPos)
	{
		if(charIdx >= mNumChars)
		{
			mCaretPos = 0;
			return;
		}

		u32 numLines = GetNumLines();
		u32 curPos = 0;
		u32 curCharIdx = 0;
		for(u32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = GetLineDesc(i);

			curPos++; // Move past line start position

			u32 numChars = lineDesc.GetEndChar() - lineDesc.GetStartChar();
			u32 numCaretPositions = lineDesc.GetEndChar(false) - lineDesc.GetStartChar();
			if(charIdx >= (curCharIdx + numChars))
			{
				curCharIdx += numChars;
				curPos += numCaretPositions;
				continue;
			}

			u32 diff = charIdx - curCharIdx;

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

	Vector2I GUIInputCaret::GetCaretPosition(const Vector2I& offset) const
	{
		if(mNumChars > 0 && IsDescValid())
		{
			u32 curPos = 0;
			u32 numLines = GetNumLines();

			for(u32 i = 0; i < numLines; i++)
			{
				const GUIInputLineDesc& lineDesc = GetLineDesc(i);

				if(mCaretPos == curPos)
				{
					// Caret is on line start
					return Vector2I(offset.X, lineDesc.GetLineYStart() + GetTextOffset().Y);
				}

				curPos += lineDesc.GetEndChar(false) - lineDesc.GetStartChar() + 1; // + 1 for special line start position
			}

			u32 charIdx = GetCharIdxAtCaretPos();
			if(charIdx > 0)
				charIdx -= 1;

			charIdx = std::min((u32)(mNumChars - 1), charIdx);

			Rect2I charRect = GetCharRect(charIdx);
			u32 lineIdx = GetLineForChar(charIdx);
			u32 yOffset = GetLineDesc(lineIdx).GetLineYStart() + GetTextOffset().Y;

			return Vector2I(charRect.X + charRect.Width, yOffset);
		}

		return offset;
	}

	u32 GUIInputCaret::GetCaretHeight() const
	{
		u32 charIdx = GetCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;

		if(charIdx < mNumChars && IsDescValid())
		{
			u32 lineIdx = GetLineForChar(charIdx);
			return GetLineDesc(lineIdx).GetLineHeight();
		}
		else
		{
			if(mTextDesc.Font != nullptr)
			{
				u32 nearestSize = mTextDesc.Font->GetClosestSize(mTextDesc.FontSize);
				SPtr<const FontBitmap> fontData = mTextDesc.Font->GetBitmap(nearestSize);

				if(fontData != nullptr)
					return fontData->LineHeight;
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
		if(mNumChars == 0)
			return 0;

		u32 numLines = GetNumLines();
		u32 maxPos = 0;
		for(u32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = GetLineDesc(i);

			u32 numChars = lineDesc.GetEndChar(false) - lineDesc.GetStartChar() + 1; // + 1 for special line start position
			maxPos += numChars;
		}

		return maxPos - 1;
	}
} // namespace bs
