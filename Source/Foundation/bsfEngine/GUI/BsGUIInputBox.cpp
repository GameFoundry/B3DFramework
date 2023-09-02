//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInputBox.h"
#include "GUI/BsGUIManager.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUITextInputEvent.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIInputCaret.h"
#include "GUI/BsGUIInputSelection.h"
#include "GUI/BsGUIContextMenu.h"
#include "GUI/BsGUIHelper.h"
#include "Utility/BsTime.h"
#include "Platform/BsPlatform.h"
#include "String/BsUnicode.h"

using namespace bs;

VirtualButton GUIInputBox::mCopyVB = VirtualButton("Copy");
VirtualButton GUIInputBox::mPasteVB = VirtualButton("Paste");
VirtualButton GUIInputBox::mCutVB = VirtualButton("Cut");
VirtualButton GUIInputBox::mSelectAllVB = VirtualButton("SelectAll");

const String& GUIInputBox::GetGuiTypeName()
{
	static String name = "InputBox";
	return name;
}

GUIInputBox::GUIInputBox(const String& styleName, const GUIDimensions& dimensions, bool multiline)
	: GUIElement(styleName, dimensions, GUIElementOption::AcceptsKeyFocus), mIsMultiline(multiline)
{
	mImageSprite = B3DNew<ImageSprite>();
	mTextSprite = B3DNew<TextSprite>();
}

GUIInputBox::~GUIInputBox()
{
	B3DDelete(mTextSprite);
	B3DDelete(mImageSprite);
}

GUIInputBox* GUIInputBox::Create(bool multiline, const String& styleName)
{
	return new(B3DAllocate<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(), multiline);
}

GUIInputBox* GUIInputBox::Create(bool multiline, const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(options), multiline);
}

GUIInputBox* GUIInputBox::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(options), false);
}

void GUIInputBox::SetText(const String& text)
{
	if(mText == text)
		return;

	bool filterOkay = true;
	if(mFilter != nullptr)
	{
		filterOkay = mFilter(text);
	}

	if(filterOkay)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

		mText = text;
		mNumChars = UTF8::Count(mText);

		if(mHasFocus)
		{
			TextSpriteInformation textDesc = GetTextDesc();

			GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
			GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

			if(mNumChars > 0)
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(mNumChars - 1, CARET_AFTER);
			else
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);

			if(mSelectionShown)
				GetGUIManager().GetInputSelectionTool()->SelectAll();

			ScrollTextToCaret();
		}

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();
	}
}

void GUIInputBox::UpdateRenderElements()
{
	mImageDesc.Width = mLayoutData.Area.Width;
	mImageDesc.Height = mLayoutData.Area.Height;
	mImageDesc.BorderLeft = GetStyle()->Border.Left;
	mImageDesc.BorderRight = GetStyle()->Border.Right;
	mImageDesc.BorderTop = GetStyle()->Border.Top;
	mImageDesc.BorderBottom = GetStyle()->Border.Bottom;
	mImageDesc.Color = GetTint();

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
		mImageDesc.Image = activeTex;

	mImageSprite->Update(mImageDesc, (u64)GetParentWidget());

	TextSpriteInformation textDesc = GetTextDesc();
	mTextSprite->Update(textDesc, (u64)GetParentWidget());

	ImageSprite* caretSprite = nullptr;
	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
		GetGUIManager().GetInputCaretTool()->UpdateSprite();

		caretSprite = GetGUIManager().GetInputCaretTool()->GetSprite();
	}

	if(mSelectionShown)
	{
		GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
		GetGUIManager().GetInputSelectionTool()->UpdateSprite();
	}

	// When text bounds are reduced the scroll needs to be adjusted so that
	// input box isn't filled with mostly empty space.
	Vector2I offset(mLayoutData.Area.X, mLayoutData.Area.Y);
	ClampScrollToBounds(mTextSprite->GetBounds(offset, Rect2I()));

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mTextSprite, 1), T::SpriteInfo(mImageSprite, 3), T::SpriteInfo(caretSprite) }, mRenderElements);

		if(mSelectionShown)
		{
			const Vector<ImageSprite*>& sprites = GetGUIManager().GetInputSelectionTool()->GetSprites();
			for(auto& entry : sprites)
			{
				for(u32 i = 0; i < entry->GetRenderElementCount(); i++)
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					entry->GetRenderElement(i, renderElement);

					renderElement.Depth = 2;
					renderElement.Type = GUIMeshType::Triangle;
				}
			}
		}
	}

	GUIElement::UpdateRenderElements();
}

void GUIInputBox::UpdateClippedBounds()
{
	Vector2I offset(mLayoutData.Area.X, mLayoutData.Area.Y);
	mClippedBounds = mImageSprite->GetBounds(offset, mLayoutData.GetLocalClipRect());
}

Sprite* GUIInputBox::RenderElemToSprite(u32 renderElemIdx, u32& localRenderElemIdx) const
{
	u32 oldNumElements = 0;
	u32 newNumElements = oldNumElements + mTextSprite->GetRenderElementCount();
	if(renderElemIdx < newNumElements)
	{
		localRenderElemIdx = renderElemIdx - oldNumElements;
		return mTextSprite;
	}

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetRenderElementCount();

	if(renderElemIdx < newNumElements)
	{
		localRenderElemIdx = renderElemIdx - oldNumElements;
		return mImageSprite;
	}

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetRenderElementCount();

		if(renderElemIdx < newNumElements)
		{
			localRenderElemIdx = renderElemIdx - oldNumElements;
			return GetGUIManager().GetInputCaretTool()->GetSprite();
		}
	}

	if(mSelectionShown)
	{
		const Vector<ImageSprite*>& sprites = GetGUIManager().GetInputSelectionTool()->GetSprites();
		for(auto& selectionSprite : sprites)
		{
			oldNumElements = newNumElements;
			newNumElements += selectionSprite->GetRenderElementCount();

			if(renderElemIdx < newNumElements)
			{
				localRenderElemIdx = renderElemIdx - oldNumElements;
				return selectionSprite;
			}
		}
	}

	localRenderElemIdx = renderElemIdx;
	return nullptr;
}

Vector2I GUIInputBox::RenderElemToOffset(u32 renderElemIdx) const
{
	u32 oldNumElements = 0;
	u32 newNumElements = oldNumElements + mTextSprite->GetRenderElementCount();
	if(renderElemIdx < newNumElements)
		return GetTextOffset();

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetRenderElementCount();

	if(renderElemIdx < newNumElements)
		return Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y);
	;

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetRenderElementCount();

		if(renderElemIdx < newNumElements)
			return GetGUIManager().GetInputCaretTool()->GetSpriteOffset();
	}

	if(mSelectionShown)
	{
		u32 spriteIdx = 0;
		const Vector<ImageSprite*>& sprites = GetGUIManager().GetInputSelectionTool()->GetSprites();
		for(auto& selectionSprite : sprites)
		{
			oldNumElements = newNumElements;
			newNumElements += selectionSprite->GetRenderElementCount();

			if(renderElemIdx < newNumElements)
				return GetGUIManager().GetInputSelectionTool()->GetSelectionSpriteOffset(spriteIdx);

			spriteIdx++;
		}
	}

	return Vector2I();
}

Rect2I GUIInputBox::RenderElemToClipRect(u32 renderElemIdx) const
{
	u32 oldNumElements = 0;
	u32 newNumElements = oldNumElements + mTextSprite->GetRenderElementCount();
	if(renderElemIdx < newNumElements)
		return GetTextClipRect();

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetRenderElementCount();

	if(renderElemIdx < newNumElements)
		return mLayoutData.GetLocalClipRect();

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetRenderElementCount();

		if(renderElemIdx < newNumElements)
		{
			return GetGUIManager().GetInputCaretTool()->GetSpriteClipRect(GetTextClipRect());
		}
	}

	if(mSelectionShown)
	{
		u32 spriteIdx = 0;
		const Vector<ImageSprite*>& sprites = GetGUIManager().GetInputSelectionTool()->GetSprites();
		for(auto& selectionSprite : sprites)
		{
			oldNumElements = newNumElements;
			newNumElements += selectionSprite->GetRenderElementCount();

			if(renderElemIdx < newNumElements)
				return GetGUIManager().GetInputSelectionTool()->GetSelectionSpriteClipRect(spriteIdx, GetTextClipRect());

			spriteIdx++;
		}
	}

	return Rect2I();
}

Vector2I GUIInputBox::GetOptimalSize() const
{
	u32 imageWidth = 0;
	u32 imageHeight = 0;

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
	{
		imageWidth = activeTex->GetWidth();
		imageHeight = activeTex->GetHeight();
	}

	Vector2I contentSize = GUIHelper::CalcOptimalContentsSize(mText, *GetStyle(), GetDimensions());
	u32 contentWidth = std::max(imageWidth, (u32)contentSize.X);
	u32 contentHeight = std::max(imageHeight, (u32)contentSize.Y);

	return Vector2I(contentWidth, contentHeight);
}

Vector2I GUIInputBox::GetTextInputOffset() const
{
	return mTextOffset;
}

Rect2I GUIInputBox::GetTextInputRect() const
{
	Rect2I textBounds = GetCachedContentBounds();
	textBounds.X -= mLayoutData.Area.X;
	textBounds.Y -= mLayoutData.Area.Y;

	return textBounds;
}

u32 GUIInputBox::GetRenderElementDepthRange() const
{
	return 4;
}

bool GUIInputBox::HasCustomCursor(const Vector2I position, CursorType& type) const
{
	if(IsInBounds(position) && !IsDisabled())
	{
		type = CursorType::IBeam;
		return true;
	}

	return false;
}

void GUIInputBox::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxNumVerts,
	u32 maxNumIndices,
	u32 renderElementIdx) const
{
	u8* uvs = vertices + sizeof(Vector2);
	u32 vertexStride = sizeof(Vector2) * 2;
	u32 indexStride = sizeof(u32);

	u32 localRenderElementIdx;
	Sprite* sprite = RenderElemToSprite(renderElementIdx, localRenderElementIdx);
	Vector2I layoutOffset = RenderElemToOffset(renderElementIdx) + offset;
	Rect2I clipRect = RenderElemToClipRect(renderElementIdx);

	sprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, localRenderElementIdx, layoutOffset, clipRect);
}

bool GUIInputBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseOver)
	{
		if(!IsDisabled())
		{
			if(!mHasFocus)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
				mState = State::Hover;
				Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

				if(origSize != newSize)
					MarkLayoutAsDirty();
				else
					MarkContentAsDirty();
			}

			mIsMouseOver = true;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabled())
		{
			if(!mHasFocus)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
				mState = State::Normal;
				Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

				if(origSize != newSize)
					MarkLayoutAsDirty();
				else
					MarkContentAsDirty();
			}

			mIsMouseOver = false;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabled())
		{
			ShowSelection(0);
			GetGUIManager().GetInputSelectionTool()->SelectAll();

			MarkContentAsDirty();
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDown && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabled())
		{
			if(ev.IsShiftDown())
			{
				if(!mSelectionShown)
					ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());
			}
			else
			{
				bool focusGainedThisFrame = mHasFocus && mFocusGainedFrame == GetTime().GetCurrentFrameIndex();

				// We want to select all on focus gain, so don't override that
				if(!focusGainedThisFrame)
					ClearSelection();

				ShowCaret();
			}

			if(mNumChars > 0)
				GetGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.GetPosition());
			else
				GetGUIManager().GetInputCaretTool()->MoveCaretToStart();

			if(ev.IsShiftDown())
				GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

			ScrollTextToCaret();
			MarkContentAsDirty();
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragStart)
	{
		if(!IsDisabled())
		{
			if(!ev.IsShiftDown())
			{
				mDragInProgress = true;

				u32 caretPos = GetGUIManager().GetInputCaretTool()->GetCaretPos();
				ShowSelection(caretPos);
				GetGUIManager().GetInputSelectionTool()->SelectionDragStart(caretPos);
				MarkContentAsDirty();

				return true;
			}
		}
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragEnd)
	{
		if(!IsDisabled())
		{
			if(!ev.IsShiftDown())
			{
				mDragInProgress = false;

				GetGUIManager().GetInputSelectionTool()->SelectionDragEnd();
				MarkContentAsDirty();
				return true;
			}
		}
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDrag)
	{
		if(!IsDisabled())
		{
			if(!ev.IsShiftDown())
			{
				if(mNumChars > 0)
					GetGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.GetPosition());
				else
					GetGUIManager().GetInputCaretTool()->MoveCaretToStart();

				GetGUIManager().GetInputSelectionTool()->SelectionDragUpdate(GetGUIManager().GetInputCaretTool()->GetCaretPos());

				ScrollTextToCaret();
				MarkContentAsDirty();
				return true;
			}
		}
	}

	return false;
}

bool GUIInputBox::DoOnTextInputEvent(const GUITextInputEvent& ev)
{
	if(IsDisabled())
		return false;

	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	if(mSelectionShown)
		DeleteSelectedText(true);

	u32 charIdx = GetGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

	bool filterOkay = true;
	if(mFilter != nullptr)
	{
		String newText = mText;
		u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
		String utf8chars = UTF8::FromUtF32(U32String(1, ev.GetInputChar()));
		newText.insert(newText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());

		filterOkay = mFilter(newText);
	}

	if(filterOkay)
	{
		InsertChar(charIdx, ev.GetInputChar());

		GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);
		ScrollTextToCaret();

		if(!OnValueChanged.Empty())
			OnValueChanged(mText);
	}

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();

	return true;
}

bool GUIInputBox::DoOnCommandEvent(const GUICommandEvent& ev)
{
	if(IsDisabled())
		return false;

	bool baseReturn = GUIElement::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::Redraw)
	{
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		mState = State::Focused;

		ShowSelection(0);
		GetGUIManager().GetInputSelectionTool()->SelectAll();

		mHasFocus = true;
		mFocusGainedFrame = GetTime().GetCurrentFrameIndex();

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		mState = State::Normal;

		HideCaret();
		ClearSelection();

		mHasFocus = false;

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Backspace)
	{
		if(mNumChars > 0)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
			if(mSelectionShown)
			{
				DeleteSelectedText();
			}
			else
			{
				u32 charIdx = GetGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos() - 1;

				if(charIdx < mNumChars)
				{
					bool filterOkay = true;
					if(mFilter != nullptr)
					{
						String newText = mText;
						u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
						u32 byteCount = UTF8::CharByteCount(mText, charIdx);
						newText.erase(byteIdx, byteCount);

						filterOkay = mFilter(newText);
					}

					if(filterOkay)
					{
						EraseChar(charIdx);

						if(charIdx > 0)
						{
							charIdx--;

							GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);
						}
						else
							GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_BEFORE);

						ScrollTextToCaret();

						if(!OnValueChanged.Empty())
							OnValueChanged(mText);
					}
				}
			}

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirty();
			else
				MarkContentAsDirty();
		}

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Delete)
	{
		if(mNumChars > 0)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
			if(mSelectionShown)
			{
				DeleteSelectedText();
			}
			else
			{
				u32 charIdx = GetGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();
				if(charIdx < mNumChars)
				{
					bool filterOkay = true;
					if(mFilter != nullptr)
					{
						String newText = mText;
						u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
						u32 byteCount = UTF8::CharByteCount(mText, charIdx);
						newText.erase(byteIdx, byteCount);

						filterOkay = mFilter(newText);
					}

					if(filterOkay)
					{
						EraseChar(charIdx);

						if(charIdx > 0)
							charIdx--;

						GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);

						ScrollTextToCaret();

						if(!OnValueChanged.Empty())
							OnValueChanged(mText);
					}
				}
			}

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirty();
			else
				MarkContentAsDirty();
		}

		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveLeft)
	{
		if(mSelectionShown)
		{
			u32 selStart = GetGUIManager().GetInputSelectionTool()->GetSelectionStart();
			ClearSelection();

			if(!mCaretShown)
				ShowCaret();

			if(selStart > 0)
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(selStart - 1, CARET_AFTER);
			else
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
		}
		else
			GetGUIManager().GetInputCaretTool()->MoveCaretLeft();

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectLeft)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretLeft();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveRight)
	{
		if(mSelectionShown)
		{
			u32 selEnd = GetGUIManager().GetInputSelectionTool()->GetSelectionEnd();
			ClearSelection();

			if(!mCaretShown)
				ShowCaret();

			if(selEnd > 0)
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(selEnd - 1, CARET_AFTER);
			else
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
		}
		else
			GetGUIManager().GetInputCaretTool()->MoveCaretRight();

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectRight)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretRight();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveUp)
	{
		if(mSelectionShown)
			ClearSelection();

		if(!mCaretShown)
			ShowCaret();

		GetGUIManager().GetInputCaretTool()->MoveCaretUp();

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectUp)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());
		;

		GetGUIManager().GetInputCaretTool()->MoveCaretUp();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveDown)
	{
		if(mSelectionShown)
			ClearSelection();

		if(!mCaretShown)
			ShowCaret();

		GetGUIManager().GetInputCaretTool()->MoveCaretDown();

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectDown)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretDown();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::Return)
	{
		if(mIsMultiline)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

			if(mSelectionShown)
				DeleteSelectedText();

			u32 charIdx = GetGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

			bool filterOkay = true;
			if(mFilter != nullptr)
			{
				String newText = mText;
				u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
				newText.insert(newText.begin() + byteIdx, '\n');

				filterOkay = mFilter(newText);
			}

			if(filterOkay)
			{
				InsertChar(charIdx, '\n');

				GetGUIManager().GetInputCaretTool()->MoveCaretRight();
				ScrollTextToCaret();

				if(!OnValueChanged.Empty())
					OnValueChanged(mText);
			}

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirty();
			else
				MarkContentAsDirty();

			return true;
		}
	}

	if(ev.GetType() == GUICommandEventType::Confirm)
	{
		OnConfirm();
		return true;
	}

	return baseReturn;
}

bool GUIInputBox::DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& ev)
{
	if(IsDisabled())
		return false;

	if(ev.GetButton() == mCutVB)
	{
		CutText();

		return true;
	}
	else if(ev.GetButton() == mCopyVB)
	{
		CopyText();

		return true;
	}
	else if(ev.GetButton() == mPasteVB)
	{
		PasteText();
		return true;
	}
	else if(ev.GetButton() == mSelectAllVB)
	{
		ShowSelection(0);
		GetGUIManager().GetInputSelectionTool()->SelectAll();
		MarkContentAsDirty();

		return true;
	}

	return false;
}

void GUIInputBox::ShowCaret()
{
	mCaretShown = true;

	TextSpriteInformation textDesc = GetTextDesc();
	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
}

void GUIInputBox::HideCaret()
{
	mCaretShown = false;
}

void GUIInputBox::ShowSelection(u32 anchorCaretPos)
{
	TextSpriteInformation textDesc = GetTextDesc();
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

	GetGUIManager().GetInputSelectionTool()->ShowSelection(anchorCaretPos);
	mSelectionShown = true;
}

void GUIInputBox::ClearSelection()
{
	GetGUIManager().GetInputSelectionTool()->ClearSelectionVisuals();
	mSelectionShown = false;
}

void GUIInputBox::ScrollTextToCaret()
{
	TextSpriteInformation textDesc = GetTextDesc();

	Vector2I textOffset = GetTextOffset();
	Vector2I caretPos = GetGUIManager().GetInputCaretTool()->GetCaretPosition(textOffset);
	u32 caretHeight = GetGUIManager().GetInputCaretTool()->GetCaretHeight();
	u32 caretWidth = 1;

	i32 left = textOffset.X - mTextOffset.X;
	// Include caret width here because we don't want to scroll if just the caret is outside the bounds
	// (Possible if the text width is exactly the maximum width)
	i32 right = left + (i32)textDesc.Width + caretWidth;
	i32 top = textOffset.Y - mTextOffset.Y;
	i32 bottom = top + (i32)textDesc.Height;

	// If caret is too high to display we don't want the offset to keep adjusting itself
	caretHeight = std::min(caretHeight, (u32)(bottom - top));
	i32 caretRight = caretPos.X + (i32)caretWidth;
	i32 caretBottom = caretPos.Y + (i32)caretHeight;

	Vector2I offset;
	if(caretPos.X < left)
	{
		offset.X = left - caretPos.X;
	}
	else if(caretRight > right)
	{
		offset.X = -(caretRight - right);
	}

	if(caretPos.Y < top)
	{
		offset.Y = top - caretPos.Y;
	}
	else if(caretBottom > bottom)
	{
		offset.Y = -(caretBottom - bottom);
	}

	mTextOffset += offset;

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::ClampScrollToBounds(Rect2I unclippedTextBounds)
{
	TextSpriteInformation textDesc = GetTextDesc();

	Vector2I newTextOffset;
	i32 maxScrollableWidth = std::max(0, (i32)unclippedTextBounds.Width - (i32)textDesc.Width);
	i32 maxScrollableHeight = std::max(0, (i32)unclippedTextBounds.Height - (i32)textDesc.Height);
	newTextOffset.X = Math::Clamp(mTextOffset.X, -maxScrollableWidth, 0);
	newTextOffset.Y = Math::Clamp(mTextOffset.Y, -maxScrollableHeight, 0);

	if(newTextOffset != mTextOffset)
	{
		mTextOffset = newTextOffset;

		GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
	}
}

void GUIInputBox::InsertString(u32 charIdx, const String& string)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);

	mText.insert(mText.begin() + byteIdx, string.begin(), string.end());
	mNumChars = UTF8::Count(mText);

	TextSpriteInformation textDesc = GetTextDesc();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::InsertChar(u32 charIdx, u32 charCode)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	String utf8chars = UTF8::FromUtF32(U32String(1, (char32_t)charCode));

	mText.insert(mText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());
	mNumChars = UTF8::Count(mText);

	TextSpriteInformation textDesc = GetTextDesc();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::EraseChar(u32 charIdx)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	u32 byteCount = UTF8::CharByteCount(mText, charIdx);

	mText.erase(byteIdx, byteCount);
	mNumChars = UTF8::Count(mText);

	TextSpriteInformation textDesc = GetTextDesc();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::DeleteSelectedText(bool internal)
{
	u32 selStart = GetGUIManager().GetInputSelectionTool()->GetSelectionStart();
	u32 selEnd = GetGUIManager().GetInputSelectionTool()->GetSelectionEnd();

	u32 byteStart = UTF8::CharToByteIndex(mText, selStart);
	u32 byteEnd = UTF8::CharToByteIndex(mText, selEnd);

	bool filterOkay = true;
	if(!internal && mFilter != nullptr)
	{
		String newText = mText;
		newText.erase(newText.begin() + byteStart, newText.begin() + byteEnd);

		filterOkay = mFilter(newText);
	}

	if(!mCaretShown)
		ShowCaret();

	if(filterOkay)
	{
		mText.erase(mText.begin() + byteStart, mText.begin() + byteEnd);
		mNumChars = UTF8::Count(mText);

		TextSpriteInformation textDesc = GetTextDesc();
		GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

		if(selStart > 0)
		{
			u32 newCaretPos = selStart - 1;
			GetGUIManager().GetInputCaretTool()->MoveCaretToChar(newCaretPos, CARET_AFTER);
		}
		else
		{
			GetGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
		}

		ScrollTextToCaret();

		if(!internal)
			OnValueChanged(mText);
	}

	ClearSelection();
}

String GUIInputBox::GetSelectedText()
{
	u32 selStart = GetGUIManager().GetInputSelectionTool()->GetSelectionStart();
	u32 selEnd = GetGUIManager().GetInputSelectionTool()->GetSelectionEnd();

	u32 byteStart = UTF8::CharToByteIndex(mText, selStart);
	u32 byteEnd = UTF8::CharToByteIndex(mText, selEnd);

	return mText.substr(byteStart, byteEnd - byteStart);
}

Vector2I GUIInputBox::GetTextOffset() const
{
	Rect2I textBounds = GetCachedContentBounds();
	return Vector2I(textBounds.X, textBounds.Y) + mTextOffset;
}

Rect2I GUIInputBox::GetTextClipRect() const
{
	Rect2I contentClipRect = GetCachedContentClipRect();
	return Rect2I(contentClipRect.X - mTextOffset.X, contentClipRect.Y - mTextOffset.Y, contentClipRect.Width, contentClipRect.Height);
}

TextSpriteInformation GUIInputBox::GetTextDesc() const
{
	TextSpriteInformation textDesc;
	textDesc.Text = mText;
	textDesc.Font = GetStyle()->Font;
	textDesc.FontSize = GetStyle()->FontSize;
	textDesc.Color = GetTint() * GetActiveTextColor();

	Rect2I textBounds = GetCachedContentBounds();
	textDesc.Width = textBounds.Width;
	textDesc.Height = textBounds.Height;
	textDesc.HorzAlign = GetStyle()->TextHorzAlign;
	textDesc.VertAlign = GetStyle()->TextVertAlign;
	textDesc.WordWrap = mIsMultiline;

	return textDesc;
}

const HSpriteTexture& GUIInputBox::GetActiveTexture() const
{
	switch(mState)
	{
	case State::Focused:
		return GetStyle()->Focused.Texture;
	case State::Hover:
		return GetStyle()->Hover.Texture;
	case State::Normal:
		return GetStyle()->Normal.Texture;
	}

	return GetStyle()->Normal.Texture;
}

Color GUIInputBox::GetActiveTextColor() const
{
	switch(mState)
	{
	case State::Focused:
		return GetStyle()->Focused.TextColor;
	case State::Hover:
		return GetStyle()->Hover.TextColor;
	case State::Normal:
		return GetStyle()->Normal.TextColor;
	}

	return GetStyle()->Normal.TextColor;
}

SPtr<GUIContextMenu> GUIInputBox::GetContextMenu() const
{
	static SPtr<GUIContextMenu> contextMenu;

	if(contextMenu == nullptr)
	{
		contextMenu = B3DMakeShared<GUIContextMenu>();

		contextMenu->AddMenuItem("Cut", std::bind(&GUIInputBox::mText, const_cast<GUIInputBox*>(this)), 0);
		contextMenu->AddMenuItem("Copy", std::bind(&GUIInputBox::CopyText, const_cast<GUIInputBox*>(this)), 0);
		contextMenu->AddMenuItem("Paste", std::bind(&GUIInputBox::PasteText, const_cast<GUIInputBox*>(this)), 0);

		contextMenu->SetLocalizedName("Cut", HString("Cut"));
		contextMenu->SetLocalizedName("Copy", HString("Copy"));
		contextMenu->SetLocalizedName("Paste", HString("Paste"));
	}

	if(!IsDisabled())
		return contextMenu;

	return nullptr;
}

void GUIInputBox::CutText()
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;

	CopyText();
	DeleteSelectedText();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUIInputBox::CopyText()
{
	Platform::CopyToClipboard(GetSelectedText());
}

void GUIInputBox::PasteText()
{
	DeleteSelectedText(true);

	String textInClipboard = Platform::CopyFromClipboard();
	u32 charIdx = GetGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

	bool filterOkay = true;
	if(mFilter != nullptr)
	{
		String newText = mText;

		u32 byteIdx = UTF8::CharToByteIndex(newText, charIdx);
		newText.insert(newText.begin() + byteIdx, textInClipboard.begin(), textInClipboard.end());

		filterOkay = mFilter(newText);
	}

	if(filterOkay)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		InsertString(charIdx, textInClipboard);

		u32 numChars = UTF8::Count(textInClipboard);
		if(numChars > 0)
			GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx + (numChars - 1), CARET_AFTER);

		ScrollTextToCaret();

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSize()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		if(!OnValueChanged.Empty())
			OnValueChanged(mText);
	}
}
