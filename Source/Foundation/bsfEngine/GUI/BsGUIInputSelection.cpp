//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInputSelection.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIManager.h"

using namespace bs;

GUIInputSelection::~GUIInputSelection()
{
	for(auto& sprite : mSprites)
		bs_delete(sprite);
}

void GUIInputSelection::UpdateSprite()
{
	mSelectionRects = GetSelectionRects();

	i32 diff = (i32)(mSprites.size() - mSelectionRects.size());

	if(diff > 0)
	{
		for(u32 i = (u32)mSelectionRects.size(); i < (u32)mSprites.size(); i++)
			bs_delete(mSprites[i]);

		mSprites.erase(mSprites.begin() + mSelectionRects.size(), mSprites.end());
	}
	else if(diff < 0)
	{
		for(i32 i = diff; i < 0; i++)
		{
			ImageSprite* newSprite = bs_new<ImageSprite>();
			mSprites.push_back(newSprite);
		}
	}

	const GUIWidget* widget = nullptr;
	if(mElement != nullptr)
		widget = mElement->GetParentWidgetInternal();

	u32 idx = 0;
	for(auto& sprite : mSprites)
	{
		IMAGE_SPRITE_DESC desc;
		desc.Width = mSelectionRects[idx].Width;
		desc.Height = mSelectionRects[idx].Height;
		desc.Texture = GUIManager::Instance().GetTextSelectionTexture();

		sprite->Update(desc, (u64)widget);
		idx++;
	}
}

Vector2I GUIInputSelection::GetSelectionSpriteOffset(u32 spriteIdx) const
{
	return Vector2I(mSelectionRects[spriteIdx].X, mSelectionRects[spriteIdx].Y) + GetTextOffset();
}

Rect2I GUIInputSelection::GetSelectionSpriteClipRect(u32 spriteIdx, const Rect2I& parentClipRect) const
{
	Vector2I selectionOffset(mSelectionRects[spriteIdx].X, mSelectionRects[spriteIdx].Y);
	Vector2I clipOffset = selectionOffset + mElement->GetTextInputOffsetInternal();

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

Vector<Rect2I> GUIInputSelection::GetSelectionRects() const
{
	Vector<Rect2I> selectionRects;

	if(mSelectionStart == mSelectionEnd)
		return selectionRects;

	u32 startLine = GetLineForChar(mSelectionStart);

	u32 endLine = startLine;
	if(mSelectionEnd > 0)
		endLine = GetLineForChar(mSelectionEnd - 1, true);

	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(startLine);

		u32 startCharIdx = mSelectionStart;

		u32 endCharIdx = mSelectionEnd - 1;
		if(startLine != endLine)
		{
			endCharIdx = lineDesc.GetEndChar(false);
			if(endCharIdx > 0)
				endCharIdx = endCharIdx - 1;
		}

		if(!IsNewlineChar(startCharIdx) && !IsNewlineChar(endCharIdx))
		{
			Rect2I startChar = GetLocalCharRect(startCharIdx);
			Rect2I endChar = GetLocalCharRect(endCharIdx);

			Rect2I selectionRect;
			selectionRect.X = startChar.X;
			selectionRect.Y = lineDesc.GetLineYStart();
			selectionRect.Height = lineDesc.GetLineHeight();
			selectionRect.Width = (endChar.X + endChar.Width) - startChar.X;

			selectionRects.push_back(selectionRect);
		}
	}

	for(u32 i = startLine + 1; i < endLine; i++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(i);
		if(lineDesc.GetStartChar() == lineDesc.GetEndChar() || IsNewlineChar(lineDesc.GetStartChar()))
			continue;

		u32 endCharIdx = lineDesc.GetEndChar(false);
		if(endCharIdx > 0)
			endCharIdx = endCharIdx - 1;

		Rect2I startChar = GetLocalCharRect(lineDesc.GetStartChar());
		Rect2I endChar = GetLocalCharRect(endCharIdx);

		Rect2I selectionRect;
		selectionRect.X = startChar.X;
		selectionRect.Y = lineDesc.GetLineYStart();
		selectionRect.Height = lineDesc.GetLineHeight();
		selectionRect.Width = (endChar.X + endChar.Width) - startChar.X;

		selectionRects.push_back(selectionRect);
	}

	if(startLine != endLine)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(endLine);

		if(lineDesc.GetStartChar() != lineDesc.GetEndChar() && !IsNewlineChar(lineDesc.GetStartChar()))
		{
			u32 endCharIdx = mSelectionEnd - 1;

			if(!IsNewlineChar(endCharIdx))
			{
				Rect2I startChar = GetLocalCharRect(lineDesc.GetStartChar());
				Rect2I endChar = GetLocalCharRect(endCharIdx);

				Rect2I selectionRect;
				selectionRect.X = startChar.X;
				selectionRect.Y = lineDesc.GetLineYStart();
				selectionRect.Height = lineDesc.GetLineHeight();
				selectionRect.Width = (endChar.X + endChar.Width) - startChar.X;

				selectionRects.push_back(selectionRect);
			}
		}
	}

	return selectionRects;
}

void GUIInputSelection::ShowSelection(u32 anchorCaretPos)
{
	u32 charIdx = GetCharIdxAtInputIdx(anchorCaretPos);

	mSelectionStart = charIdx;
	mSelectionEnd = charIdx;
	mSelectionAnchor = charIdx;
}

void GUIInputSelection::ClearSelectionVisuals()
{
	for(auto& sprite : mSprites)
		bs_delete(sprite);

	mSprites.clear();
}

void GUIInputSelection::SelectionDragStart(u32 caretPos)
{
	ClearSelectionVisuals();

	ShowSelection(caretPos);
	mSelectionDragAnchor = caretPos;
}

void GUIInputSelection::SelectionDragUpdate(u32 caretPos)
{
	if(caretPos < mSelectionDragAnchor)
	{
		mSelectionStart = GetCharIdxAtInputIdx(caretPos);
		mSelectionEnd = GetCharIdxAtInputIdx(mSelectionDragAnchor);

		mSelectionAnchor = mSelectionStart;
	}

	if(caretPos > mSelectionDragAnchor)
	{
		mSelectionStart = GetCharIdxAtInputIdx(mSelectionDragAnchor);
		mSelectionEnd = GetCharIdxAtInputIdx(caretPos);

		mSelectionAnchor = mSelectionEnd;
	}

	if(caretPos == mSelectionDragAnchor)
	{
		mSelectionStart = mSelectionAnchor;
		mSelectionEnd = mSelectionAnchor;
	}
}

void GUIInputSelection::SelectionDragEnd()
{
	if(IsSelectionEmpty())
		ClearSelectionVisuals();
}

void GUIInputSelection::MoveSelectionToCaret(u32 caretPos)
{
	u32 charIdx = GetCharIdxAtInputIdx(caretPos);

	if(charIdx > mSelectionAnchor)
	{
		mSelectionStart = mSelectionAnchor;
		mSelectionEnd = charIdx;
	}
	else
	{
		mSelectionStart = charIdx;
		mSelectionEnd = mSelectionAnchor;
	}

	if(mSelectionStart == mSelectionEnd)
		ClearSelectionVisuals();
}

void GUIInputSelection::SelectAll()
{
	mSelectionStart = 0;
	mSelectionEnd = mNumChars;
}

bool GUIInputSelection::IsSelectionEmpty() const
{
	return mSelectionStart == mSelectionEnd;
}
