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
	mImageSprite = bs_new<ImageSprite>();
	mTextSprite = bs_new<TextSprite>();
}

GUIInputBox::~GUIInputBox()
{
	bs_delete(mTextSprite);
	bs_delete(mImageSprite);
}

GUIInputBox* GUIInputBox::Create(bool multiline, const String& styleName)
{
	return new(bs_alloc<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(), multiline);
}

GUIInputBox* GUIInputBox::Create(bool multiline, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(options), multiline);
}

GUIInputBox* GUIInputBox::Create(const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUIInputBox>()) GUIInputBox(GetStyleName<GUIInputBox>(styleName), GUIDimensions::Create(options), false);
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
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

		mText = text;
		mNumChars = UTF8::Count(mText);

		if(mHasFocus)
		{
			TEXT_SPRITE_DESC textDesc = GetTextDesc();

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

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirtyInternal();
		else
			MarkContentAsDirtyInternal();
	}
}

void GUIInputBox::UpdateRenderElementsInternal()
{
	mImageDesc.Width = mLayoutData.Area.Width;
	mImageDesc.Height = mLayoutData.Area.Height;
	mImageDesc.BorderLeft = GetStyleInternal()->Border.Left;
	mImageDesc.BorderRight = GetStyleInternal()->Border.Right;
	mImageDesc.BorderTop = GetStyleInternal()->Border.Top;
	mImageDesc.BorderBottom = GetStyleInternal()->Border.Bottom;
	mImageDesc.Color = GetTint();

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
		mImageDesc.Texture = activeTex;

	mImageSprite->Update(mImageDesc, (u64)GetParentWidgetInternal());

	TEXT_SPRITE_DESC textDesc = GetTextDesc();
	mTextSprite->Update(textDesc, (u64)GetParentWidgetInternal());

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
		using T = impl::GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mTextSprite, 1), T::SpriteInfo(mImageSprite, 3), T::SpriteInfo(caretSprite) }, mRenderElements);

		if(mSelectionShown)
		{
			const Vector<ImageSprite*>& sprites = GetGUIManager().GetInputSelectionTool()->GetSprites();
			for(auto& entry : sprites)
			{
				for(u32 i = 0; i < entry->GetNumRenderElements(); i++)
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					entry->GetRenderElementInfo(i, renderElement);

					renderElement.Depth = 2;
					renderElement.Type = GUIMeshType::Triangle;
				}
			}
		}
	}

	GUIElement::UpdateRenderElementsInternal();
}

void GUIInputBox::UpdateClippedBounds()
{
	Vector2I offset(mLayoutData.Area.X, mLayoutData.Area.Y);
	mClippedBounds = mImageSprite->GetBounds(offset, mLayoutData.GetLocalClipRect());
}

Sprite* GUIInputBox::RenderElemToSprite(u32 renderElemIdx, u32& localRenderElemIdx) const
{
	u32 oldNumElements = 0;
	u32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
	if(renderElemIdx < newNumElements)
	{
		localRenderElemIdx = renderElemIdx - oldNumElements;
		return mTextSprite;
	}

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetNumRenderElements();

	if(renderElemIdx < newNumElements)
	{
		localRenderElemIdx = renderElemIdx - oldNumElements;
		return mImageSprite;
	}

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetNumRenderElements();

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
			newNumElements += selectionSprite->GetNumRenderElements();

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
	u32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
	if(renderElemIdx < newNumElements)
		return GetTextOffset();

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetNumRenderElements();

	if(renderElemIdx < newNumElements)
		return Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y);
	;

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetNumRenderElements();

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
			newNumElements += selectionSprite->GetNumRenderElements();

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
	u32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
	if(renderElemIdx < newNumElements)
		return GetTextClipRect();

	oldNumElements = newNumElements;
	newNumElements += mImageSprite->GetNumRenderElements();

	if(renderElemIdx < newNumElements)
		return mLayoutData.GetLocalClipRect();

	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		oldNumElements = newNumElements;
		newNumElements += GetGUIManager().GetInputCaretTool()->GetSprite()->GetNumRenderElements();

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
			newNumElements += selectionSprite->GetNumRenderElements();

			if(renderElemIdx < newNumElements)
				return GetGUIManager().GetInputSelectionTool()->GetSelectionSpriteClipRect(spriteIdx, GetTextClipRect());

			spriteIdx++;
		}
	}

	return Rect2I();
}

Vector2I GUIInputBox::GetOptimalSizeInternal() const
{
	u32 imageWidth = 0;
	u32 imageHeight = 0;

	const HSpriteTexture& activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
	{
		imageWidth = activeTex->GetWidth();
		imageHeight = activeTex->GetHeight();
	}

	Vector2I contentSize = GUIHelper::CalcOptimalContentsSize(mText, *GetStyleInternal(), GetDimensionsInternal());
	u32 contentWidth = std::max(imageWidth, (u32)contentSize.X);
	u32 contentHeight = std::max(imageHeight, (u32)contentSize.Y);

	return Vector2I(contentWidth, contentHeight);
}

Vector2I GUIInputBox::GetTextInputOffsetInternal() const
{
	return mTextOffset;
}

Rect2I GUIInputBox::GetTextInputRectInternal() const
{
	Rect2I textBounds = GetCachedContentBounds();
	textBounds.X -= mLayoutData.Area.X;
	textBounds.Y -= mLayoutData.Area.Y;

	return textBounds;
}

u32 GUIInputBox::GetRenderElementDepthRangeInternal() const
{
	return 4;
}

bool GUIInputBox::HasCustomCursorInternal(const Vector2I position, CursorType& type) const
{
	if(IsInBoundsInternal(position) && !IsDisabledInternal())
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

bool GUIInputBox::MouseEventInternal(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseOver)
	{
		if(!IsDisabledInternal())
		{
			if(!mHasFocus)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
				mState = State::Hover;
				Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

				if(origSize != newSize)
					MarkLayoutAsDirtyInternal();
				else
					MarkContentAsDirtyInternal();
			}

			mIsMouseOver = true;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabledInternal())
		{
			if(!mHasFocus)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
				mState = State::Normal;
				Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

				if(origSize != newSize)
					MarkLayoutAsDirtyInternal();
				else
					MarkContentAsDirtyInternal();
			}

			mIsMouseOver = false;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabledInternal())
		{
			ShowSelection(0);
			GetGUIManager().GetInputSelectionTool()->SelectAll();

			MarkContentAsDirtyInternal();
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDown && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabledInternal())
		{
			if(ev.IsShiftDown())
			{
				if(!mSelectionShown)
					ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());
			}
			else
			{
				bool focusGainedThisFrame = mHasFocus && mFocusGainedFrame == GetTime().GetFrameIdx();

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
			MarkContentAsDirtyInternal();
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragStart)
	{
		if(!IsDisabledInternal())
		{
			if(!ev.IsShiftDown())
			{
				mDragInProgress = true;

				u32 caretPos = GetGUIManager().GetInputCaretTool()->GetCaretPos();
				ShowSelection(caretPos);
				GetGUIManager().GetInputSelectionTool()->SelectionDragStart(caretPos);
				MarkContentAsDirtyInternal();

				return true;
			}
		}
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragEnd)
	{
		if(!IsDisabledInternal())
		{
			if(!ev.IsShiftDown())
			{
				mDragInProgress = false;

				GetGUIManager().GetInputSelectionTool()->SelectionDragEnd();
				MarkContentAsDirtyInternal();
				return true;
			}
		}
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDrag)
	{
		if(!IsDisabledInternal())
		{
			if(!ev.IsShiftDown())
			{
				if(mNumChars > 0)
					GetGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.GetPosition());
				else
					GetGUIManager().GetInputCaretTool()->MoveCaretToStart();

				GetGUIManager().GetInputSelectionTool()->SelectionDragUpdate(GetGUIManager().GetInputCaretTool()->GetCaretPos());

				ScrollTextToCaret();
				MarkContentAsDirtyInternal();
				return true;
			}
		}
	}

	return false;
}

bool GUIInputBox::TextInputEventInternal(const GUITextInputEvent& ev)
{
	if(IsDisabledInternal())
		return false;

	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

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

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirtyInternal();
	else
		MarkContentAsDirtyInternal();

	return true;
}

bool GUIInputBox::CommandEventInternal(const GUICommandEvent& ev)
{
	if(IsDisabledInternal())
		return false;

	bool baseReturn = GUIElement::CommandEventInternal(ev);

	if(ev.GetType() == GUICommandEventType::Redraw)
	{
		MarkContentAsDirtyInternal();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		mState = State::Focused;

		ShowSelection(0);
		GetGUIManager().GetInputSelectionTool()->SelectAll();

		mHasFocus = true;
		mFocusGainedFrame = GetTime().GetFrameIdx();

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirtyInternal();
		else
			MarkContentAsDirtyInternal();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		mState = State::Normal;

		HideCaret();
		ClearSelection();

		mHasFocus = false;

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirtyInternal();
		else
			MarkContentAsDirtyInternal();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Backspace)
	{
		if(mNumChars > 0)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
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

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirtyInternal();
			else
				MarkContentAsDirtyInternal();
		}

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Delete)
	{
		if(mNumChars > 0)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
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

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirtyInternal();
			else
				MarkContentAsDirtyInternal();
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
		MarkContentAsDirtyInternal();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectLeft)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretLeft();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirtyInternal();
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
		MarkContentAsDirtyInternal();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectRight)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretRight();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirtyInternal();
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
		MarkContentAsDirtyInternal();
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
		MarkContentAsDirtyInternal();
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
		MarkContentAsDirtyInternal();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectDown)
	{
		if(!mSelectionShown)
			ShowSelection(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		GetGUIManager().GetInputCaretTool()->MoveCaretDown();
		GetGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(GetGUIManager().GetInputCaretTool()->GetCaretPos());

		ScrollTextToCaret();
		MarkContentAsDirtyInternal();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::Return)
	{
		if(mIsMultiline)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

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

			Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
			if(origSize != newSize)
				MarkLayoutAsDirtyInternal();
			else
				MarkContentAsDirtyInternal();

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

bool GUIInputBox::VirtualButtonEventInternal(const GUIVirtualButtonEvent& ev)
{
	if(IsDisabledInternal())
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
		MarkContentAsDirtyInternal();

		return true;
	}

	return false;
}

void GUIInputBox::ShowCaret()
{
	mCaretShown = true;

	TEXT_SPRITE_DESC textDesc = GetTextDesc();
	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
}

void GUIInputBox::HideCaret()
{
	mCaretShown = false;
}

void GUIInputBox::ShowSelection(u32 anchorCaretPos)
{
	TEXT_SPRITE_DESC textDesc = GetTextDesc();
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
	TEXT_SPRITE_DESC textDesc = GetTextDesc();

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
	TEXT_SPRITE_DESC textDesc = GetTextDesc();

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

	TEXT_SPRITE_DESC textDesc = GetTextDesc();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::InsertChar(u32 charIdx, u32 charCode)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	String utf8chars = UTF8::FromUtF32(U32String(1, (char32_t)charCode));

	mText.insert(mText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());
	mNumChars = UTF8::Count(mText);

	TEXT_SPRITE_DESC textDesc = GetTextDesc();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
}

void GUIInputBox::EraseChar(u32 charIdx)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	u32 byteCount = UTF8::CharByteCount(mText, charIdx);

	mText.erase(byteIdx, byteCount);
	mNumChars = UTF8::Count(mText);

	TEXT_SPRITE_DESC textDesc = GetTextDesc();

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

		TEXT_SPRITE_DESC textDesc = GetTextDesc();
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

TEXT_SPRITE_DESC GUIInputBox::GetTextDesc() const
{
	TEXT_SPRITE_DESC textDesc;
	textDesc.Text = mText;
	textDesc.Font = GetStyleInternal()->Font;
	textDesc.FontSize = GetStyleInternal()->FontSize;
	textDesc.Color = GetTint() * GetActiveTextColor();

	Rect2I textBounds = GetCachedContentBounds();
	textDesc.Width = textBounds.Width;
	textDesc.Height = textBounds.Height;
	textDesc.HorzAlign = GetStyleInternal()->TextHorzAlign;
	textDesc.VertAlign = GetStyleInternal()->TextVertAlign;
	textDesc.WordWrap = mIsMultiline;

	return textDesc;
}

const HSpriteTexture& GUIInputBox::GetActiveTexture() const
{
	switch(mState)
	{
	case State::Focused:
		return GetStyleInternal()->Focused.Texture;
	case State::Hover:
		return GetStyleInternal()->Hover.Texture;
	case State::Normal:
		return GetStyleInternal()->Normal.Texture;
	}

	return GetStyleInternal()->Normal.Texture;
}

Color GUIInputBox::GetActiveTextColor() const
{
	switch(mState)
	{
	case State::Focused:
		return GetStyleInternal()->Focused.TextColor;
	case State::Hover:
		return GetStyleInternal()->Hover.TextColor;
	case State::Normal:
		return GetStyleInternal()->Normal.TextColor;
	}

	return GetStyleInternal()->Normal.TextColor;
}

SPtr<GUIContextMenu> GUIInputBox::GetContextMenuInternal() const
{
	static SPtr<GUIContextMenu> contextMenu;

	if(contextMenu == nullptr)
	{
		contextMenu = bs_shared_ptr_new<GUIContextMenu>();

		contextMenu->AddMenuItem("Cut", std::bind(&GUIInputBox::mText, const_cast<GUIInputBox*>(this)), 0);
		contextMenu->AddMenuItem("Copy", std::bind(&GUIInputBox::CopyText, const_cast<GUIInputBox*>(this)), 0);
		contextMenu->AddMenuItem("Paste", std::bind(&GUIInputBox::PasteText, const_cast<GUIInputBox*>(this)), 0);

		contextMenu->SetLocalizedName("Cut", HString("Cut"));
		contextMenu->SetLocalizedName("Copy", HString("Copy"));
		contextMenu->SetLocalizedName("Paste", HString("Paste"));
	}

	if(!IsDisabledInternal())
		return contextMenu;

	return nullptr;
}

void GUIInputBox::CutText()
{
	Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;

	CopyText();
	DeleteSelectedText();

	Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
	if(origSize != newSize)
		MarkLayoutAsDirtyInternal();
	else
		MarkContentAsDirtyInternal();
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
		Vector2I origSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		InsertString(charIdx, textInClipboard);

		u32 numChars = UTF8::Count(textInClipboard);
		if(numChars > 0)
			GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx + (numChars - 1), CARET_AFTER);

		ScrollTextToCaret();

		Vector2I newSize = mDimensions.CalculateSizeRange(GetOptimalSizeInternal()).Optimal;
		if(origSize != newSize)
			MarkLayoutAsDirtyInternal();
		else
			MarkContentAsDirtyInternal();

		if(!OnValueChanged.Empty())
			OnValueChanged(mText);
	}
}
