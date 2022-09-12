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
		return GetCaretPosition(getTextOffset());
	}

	Rect2I GUIInputCaret::GetSpriteClipRect(const Rect2I& parentClipRect) const
	{
		Vector2I Offset(mElement->_getLayoutData().area.x, mElement->_getLayoutData().area.y);

		Vector2I clipOffset = getSpriteOffset() - offset -
			Vector2I(mElement->_getTextInputRect().x, mElement->_getTextInputRect().y);

		Rect2I ClipRect(-clipOffset.x, -clipOffset.y, mTextDesc.width, mTextDesc.height);

		Rect2I localParentCliprect = parentClipRect;

		// Move parent rect to our space
		localParentCliprect.x += mElement->_getTextInputOffset().x + clipRect.x;
		localParentCliprect.y += mElement->_getTextInputOffset().y + clipRect.y;

		// Clip our rectangle so its not larger then the parent
		clipRect.clip(localParentCliprect);

		// Increase clip size by 1, so we can fit the caret in case it is fully at the end of the text
		clipRect.width += 1;

		return clipRect;
	}

	void GUIInputCaret::UpdateSprite()
	{
		IMAGE_SPRITE_DESC mCaretDesc;
		mCaretDesc.width = 1;
		mCaretDesc.height = getCaretHeight();
		mCaretDesc.texture = GUIManager::instance().getCaretTexture();

		GUIWidget* widget = nullptr;
		if (mElement != nullptr)
			widget = mElement->_getParentWidget();

		mCaretSprite->update(mCaretDesc, (UINT64)widget);
	}

	void GUIInputCaret::MoveCaretToStart()
	{
		mCaretPos = 0;
	}

	void GUIInputCaret::MoveCaretToEnd()
	{
		mCaretPos = getMaxCaretPos();
	}

	void GUIInputCaret::MoveCaretLeft()
	{
		mCaretPos = (UINT32)std::max(0, (INT32)mCaretPos - 1);
	}

	void GUIInputCaret::MoveCaretRight()
	{
		UINT32 maxCaretPos = getMaxCaretPos();

		mCaretPos = std::min(maxCaretPos, mCaretPos + 1);
	}

	void GUIInputCaret::MoveCaretUp()
	{
		UINT32 charIdx = getCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;	

		UINT32 lineIdx = getLineForChar(charIdx);
		const GUIInputLineDesc& desc = getLineDesc(lineIdx);
		// If char is a newline, I want that to count as being on the next line because that's
		// how user sees it
		if(desc.isNewline(charIdx))
			lineIdx++;	

		if(lineIdx == 0)
		{
			moveCaretToStart();
			return;
		}

		Vector2I caretCoords = getCaretPosition(mElement->_getTextInputOffset());
		caretCoords.y -= getCaretHeight();

		moveCaretToPos(caretCoords);
	}

	void GUIInputCaret::MoveCaretDown()
	{
		UINT32 charIdx = getCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;	

		UINT32 lineIdx = getLineForChar(charIdx);
		const GUIInputLineDesc& desc = getLineDesc(lineIdx);
		// If char is a newline, I want that to count as being on the next line because that's
		// how user sees it
		if(desc.isNewline(charIdx))
			lineIdx++;					

		if(lineIdx == (getNumLines() - 1))
		{
			moveCaretToEnd();
			return;
		}

		Vector2I caretCoords = getCaretPosition(mElement->_getTextInputOffset());
		caretCoords.y += getCaretHeight();

		moveCaretToPos(caretCoords);
	}

	void GUIInputCaret::MoveCaretToPos(const Vector2I& pos)
	{
		INT32 charIdx = getCharIdxAtPos(pos);

		if(charIdx != -1)
		{
			Rect2I charRect = getCharRect(charIdx);

			float xCenter = charRect.x + charRect.width * 0.5f;
			if(pos.x <= xCenter)
				moveCaretToChar(charIdx, CARET_BEFORE);
			else
				moveCaretToChar(charIdx, CARET_AFTER);
		}
		else
		{
			UINT32 numLines = getNumLines();

			if(numLines == 0)
			{
				mCaretPos = 0;
				return;
			}

			UINT32 curPos = 0;
			for(UINT32 i = 0; i < numLines; i++)
			{
				const GUIInputLineDesc& line = getLineDesc(i);

				INT32 lineStart = line.getLineYStart() + getTextOffset().y;
				if(pos.y >= lineStart && pos.y < (lineStart + (INT32)line.getLineHeight()))
				{
					mCaretPos = curPos;
					return;
				}

				UINT32 numChars = line.getEndChar(false) - line.getStartChar() + 1; // +1 For extra line start position
				curPos += numChars;
			}

			{
				const GUIInputLineDesc& firstLine = getLineDesc(0);
				INT32 lineStart = firstLine.getLineYStart() + getTextOffset().y;

				if(pos.y < lineStart) // Before first line
					mCaretPos = 0;
				else // After last line
					mCaretPos = curPos - 1;
			}
		}
	}

	void GUIInputCaret::MoveCaretToChar(UINT32 charIdx, CaretPos caretPos)
	{
		if(charIdx >= mNumChars)
		{
			mCaretPos = 0;
			return;
		}

		UINT32 numLines = getNumLines();
		UINT32 curPos = 0;
		UINT32 curCharIdx = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(i);
		
			curPos++; // Move past line start position

			UINT32 numChars = lineDesc.getEndChar() - lineDesc.getStartChar();
			UINT32 numCaretPositions = lineDesc.getEndChar(false) - lineDesc.getStartChar();
			if(charIdx >= (curCharIdx + numChars))
			{
				curCharIdx += numChars;
				curPos += numCaretPositions;
				continue;
			}

			UINT32 diff = charIdx - curCharIdx;

			if(caretPos == CARET_BEFORE)
				curPos += diff - 1;
			else
				curPos += diff;

			break;
		}

		mCaretPos = curPos;
	}

	UINT32 GUIInputCaret::GetCharIdxAtCaretPos() const
	{
		return GetCharIdxAtInputIdx(mCaretPos);
	}

	Vector2I GUIInputCaret::GetCaretPosition(const Vector2I& offset) const
	{
		if(mNumChars > 0 && isDescValid())
		{
			UINT32 curPos = 0;
			UINT32 numLines = getNumLines();

			for(UINT32 i = 0; i < numLines; i++)
			{
				const GUIInputLineDesc& lineDesc = getLineDesc(i);

				if(mCaretPos == curPos)
				{
					// Caret is on line start
					return Vector2I(offset.x, lineDesc.getLineYStart() + getTextOffset().y);
				}

				curPos += lineDesc.getEndChar(false) - lineDesc.getStartChar() + 1; // + 1 for special line start position
			}

			UINT32 charIdx = getCharIdxAtCaretPos();
			if(charIdx > 0)
				charIdx -= 1;			

			charIdx = std::min((UINT32)(mNumChars - 1), charIdx);

			Rect2I charRect = getCharRect(charIdx);
			UINT32 lineIdx = getLineForChar(charIdx);
			UINT32 yOffset = getLineDesc(lineIdx).getLineYStart() + getTextOffset().y;

			return Vector2I(charRect.x + charRect.width, yOffset);
		}

		return offset;
	}

	UINT32 GUIInputCaret::GetCaretHeight() const
	{
		UINT32 charIdx = getCharIdxAtCaretPos();
		if(charIdx > 0)
			charIdx -= 1;	

		if(charIdx < mNumChars && isDescValid())
		{
			UINT32 lineIdx = getLineForChar(charIdx);
			return GetLineDesc(lineIdx).getLineHeight();
		}
		else
		{
			if(mTextDesc.font != nullptr)
			{
				UINT32 nearestSize = mTextDesc.font->getClosestSize(mTextDesc.fontSize);
				SPtr<const FontBitmap> fontData = mTextDesc.font->getBitmap(nearestSize);

				if(fontData != nullptr)
					return fontData->lineHeight;
			}
		}

		return 0;
	}

	bool GUIInputCaret::IsCaretAtNewline() const
	{
		return IsNewline(mCaretPos);
	}

	UINT32 GUIInputCaret::GetMaxCaretPos() const
	{
		if(mNumChars == 0)
			return 0;

		UINT32 numLines = getNumLines();
		UINT32 maxPos = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const GUIInputLineDesc& lineDesc = getLineDesc(i);

			UINT32 numChars = lineDesc.getEndChar(false) - lineDesc.getStartChar() + 1; // + 1 for special line start position
			maxPos += numChars;
		}

		return maxPos - 1;
	}
}
