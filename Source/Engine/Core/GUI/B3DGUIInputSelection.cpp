//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIInputSelection.h"
#include "Image/B3DSpriteTexture.h"
#include "2D/B3DImageSprite.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIManager.h"

using namespace b3d;

GUIInputSelection::~GUIInputSelection()
{
	for(auto& sprite : mSprites)
		B3DDelete(sprite);
}

void GUIInputSelection::UpdateSprite()
{
	mSelectionRects = GetSelectionRects();

	i32 diff = (i32)(mSprites.size() - mSelectionRects.size());

	if(diff > 0)
	{
		for(u32 i = (u32)mSelectionRects.size(); i < (u32)mSprites.size(); i++)
			B3DDelete(mSprites[i]);

		mSprites.erase(mSprites.begin() + mSelectionRects.size(), mSprites.end());
	}
	else if(diff < 0)
	{
		for(i32 i = diff; i < 0; i++)
		{
			ImageSprite* newSprite = B3DNew<ImageSprite>();
			mSprites.push_back(newSprite);
		}
	}

	const GUIWidget* widget = nullptr;
	if(mElement != nullptr)
		widget = mElement->GetParentWidget();

	u32 index = 0;
	for(auto& sprite : mSprites)
	{
		ImageSpriteInformation imageSpriteInformation;
		imageSpriteInformation.Size.Width = mSelectionRects[index].Width;
		imageSpriteInformation.Size.Height = mSelectionRects[index].Height;
		imageSpriteInformation.Image = GUIManager::Instance().GetTextSelectionTexture();

		sprite->Update(imageSpriteInformation, (u64)widget);
		index++;
	}
}

Vector<Area2I> GUIInputSelection::GetSelectionRects() const
{
	Vector<Area2I> selectionRects;

	if(mSelectionStart == mSelectionEnd)
		return selectionRects;

	u32 startLine = GetLineForChar(mSelectionStart);

	u32 endLine = startLine;
	if(mSelectionEnd > 0)
		endLine = GetLineForChar(mSelectionEnd - 1, true);

	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(startLine);

		u32 startCharacterIndex = mSelectionStart;

		u32 endCharacterIndex = mSelectionEnd - 1;
		if(startLine != endLine)
		{
			endCharacterIndex = lineDesc.GetEndChar(false);
			if(endCharacterIndex > 0)
				endCharacterIndex = endCharacterIndex - 1;
		}

		if(!IsNewlineChar(startCharacterIndex) && !IsNewlineChar(endCharacterIndex))
		{
			Area2I startChar = GetCharacterBounds(startCharacterIndex);
			Area2I endChar = GetCharacterBounds(endCharacterIndex);

			Area2I selectionRect;
			selectionRect.X = startChar.X;
			selectionRect.Y = lineDesc.GetLineYStart();
			selectionRect.Height = lineDesc.GetLineHeight();
			selectionRect.Width = (endChar.X + endChar.Width) - startChar.X;

			selectionRects.push_back(selectionRect);
		}
	}

	for(u32 lineIndex = startLine + 1; lineIndex < endLine; lineIndex++)
	{
		const GUIInputLineDesc& lineDesc = GetLineDesc(lineIndex);
		if(lineDesc.GetStartChar() == lineDesc.GetEndChar() || IsNewlineChar(lineDesc.GetStartChar()))
			continue;

		u32 endCharacterIndex = lineDesc.GetEndChar(false);
		if(endCharacterIndex > 0)
			endCharacterIndex = endCharacterIndex - 1;

		Area2I startChar = GetCharacterBounds(lineDesc.GetStartChar());
		Area2I endChar = GetCharacterBounds(endCharacterIndex);

		Area2I selectionRect;
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
			u32 endCharacterIndex = mSelectionEnd - 1;

			if(!IsNewlineChar(endCharacterIndex))
			{
				Area2I startChar = GetCharacterBounds(lineDesc.GetStartChar());
				Area2I endChar = GetCharacterBounds(endCharacterIndex);

				Area2I selectionRect;
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

Area2I GUIInputSelection::GetBounds(u32 selectionIndex) const
{
	return mSelectionRects[selectionIndex];
}

void GUIInputSelection::ShowSelection(u32 anchorCaretPos)
{
	u32 characterIndex = GetCharIdxAtInputIdx(anchorCaretPos);

	mSelectionStart = characterIndex;
	mSelectionEnd = characterIndex;
	mSelectionAnchor = characterIndex;
}

void GUIInputSelection::ClearSelectionVisuals()
{
	for(auto& sprite : mSprites)
		B3DDelete(sprite);

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
	u32 characterIndex = GetCharIdxAtInputIdx(caretPos);

	if(characterIndex > mSelectionAnchor)
	{
		mSelectionStart = mSelectionAnchor;
		mSelectionEnd = characterIndex;
	}
	else
	{
		mSelectionStart = characterIndex;
		mSelectionEnd = mSelectionAnchor;
	}

	if(mSelectionStart == mSelectionEnd)
		ClearSelectionVisuals();
}

void GUIInputSelection::SelectAll()
{
	mSelectionStart = 0;
	mSelectionEnd = mCharacterCount;
}

bool GUIInputSelection::IsSelectionEmpty() const
{
	return mSelectionStart == mSelectionEnd;
}
