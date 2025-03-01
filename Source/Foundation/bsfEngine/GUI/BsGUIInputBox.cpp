//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIInputBox.h"

#include "BsGUIUtility.h"
#include "GUI/BsGUIManager.h"
#include "2D/BsImageSprite.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUITextInputEvent.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIInputCaret.h"
#include "GUI/BsGUIInputSelection.h"
#include "GUI/BsGUIContextMenu.h"
#include "Utility/BsTime.h"
#include "Platform/BsPlatform.h"
#include "String/BsUnicode.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

VirtualButton GUIInputBox::mCopyVB = VirtualInput::GetOrCreateVirtualButton("Copy");
VirtualButton GUIInputBox::mPasteVB = VirtualInput::GetOrCreateVirtualButton("Paste");
VirtualButton GUIInputBox::mCutVB = VirtualInput::GetOrCreateVirtualButton("Cut");
VirtualButton GUIInputBox::mSelectAllVB = VirtualInput::GetOrCreateVirtualButton("SelectAll");

const String& GUIInputBox::GetGuiTypeName()
{
	static String name = "InputBox";
	return name;
}

GUIInputBox::GUIInputBox(PrivatelyConstruct, const GUIInputBoxContent& content, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIInteractable(styleName, sizeConstraints, GUIElementOption::AcceptsKeyFocus), mIsMultiline(content.AllowMultiline)
{ }

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
		const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

		mText = text;
		mNumChars = UTF8::Count(mText);

		if(mHasFocus)
		{
			const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

			GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
			GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);

			if(mNumChars > 0)
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(mNumChars - 1, CARET_AFTER);
			else
				GetGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);

			if(mSelectionShown)
				GetGUIManager().GetInputSelectionTool()->SelectAll();

			ScrollTextToCaret();
		}

		const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		if(originalSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();
	}
}

void GUIInputBox::UpdateRenderElements()
{
	mRenderElements.clear();

	GUISpriteHelper::BuildSpriteRenderElements(*this, mState, mBackgroundSprite, Vector2I::kZero, 3);

	GUIContent textContent = GUIContent(HString(mText));
	GUISpriteHelper::BuildSpriteRenderElements(*this, mState, textContent, mTextSprite, mTextOffset.To<i32>(), 1, mIsMultiline);

	GUIInputCaret* const caret = GetGUIManager().GetInputCaretTool();
	GUIInputSelection* const selection = GetGUIManager().GetInputSelectionTool();

	Optional<TextSpriteInformation> textSpriteInformation;

	ImageSprite* caretSprite = nullptr;
	Rect2I caretBounds;
	if(mCaretShown && GetGUIManager().GetCaretBlinkState())
	{
		textSpriteInformation = BuildTextSpriteInformation();

		caret->UpdateText(this, *textSpriteInformation); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
		caret->UpdateSprite();

		caretSprite = caret->GetSprite();
		caretBounds = caret->GetBounds();
	}

	if(mSelectionShown)
	{
		if(!textSpriteInformation.has_value())
			textSpriteInformation = BuildTextSpriteInformation();

		selection->UpdateText(this, *textSpriteInformation); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
		selection->UpdateSprite();
	}

	// When text bounds are reduced the scroll needs to be adjusted so that
	// input box isn't filled with mostly empty space.
	Vector2I offset = mAbsolutePosition.To<i32>();
	ClampScrollToBounds(mTextSprite.GetTextSprite().GetBounds(offset, Rect2I()));

	const GUIPhysicalArea scaledContentBounds = GetScaledContentBounds();

	const GUIPhysicalPoint textOffset = mTextOffset + scaledContentBounds.GetPosition();
	const GUIPhysicalPoint caretOffset = GUIPhysicalPoint(caretBounds.X, caretBounds.Y) + textOffset;

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(caretSprite, 0, caretOffset.To<float>(), scaledContentBounds.ToRect2()) }, mRenderElements);

		if(mSelectionShown)
		{
			const Vector<ImageSprite*>& sprites = selection->GetSprites();
			for(u32 selectionSpriteIndex = 0; selectionSpriteIndex < (u32)sprites.size(); ++selectionSpriteIndex)
			{
				ImageSprite* const sprite = sprites[selectionSpriteIndex];
				const Rect2 selectionBounds = (Rect2)selection->GetBounds(selectionSpriteIndex);

				for(u32 renderElementIndex = 0; renderElementIndex < sprite->GetRenderElementCount(); renderElementIndex++)
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					sprite->GetRenderElement(renderElementIndex, renderElement);

					renderElement.Depth = 2;
					renderElement.Type = GUIMeshType::Triangle;
					renderElement.Offset = Vector2(selectionBounds.X, selectionBounds.Y) + textOffset.To<float>();
					renderElement.ClipRectangle = scaledContentBounds.ToRect2();
					renderElement.UseNewFillBuffer = true;
				}
			}
		}
	}

	GUIInteractable::UpdateRenderElements();
}

GUILogicalSize GUIInputBox::CalculateUnconstrainedOptimalSize() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset == nullptr)
		return GUILogicalSize::kZero;

	const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
	return GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(mText, styleSheetRules, GetSizeConstraints().MaximumWidth);
}

u32 GUIInputBox::GetRenderElementDepthRange() const
{
	return 4;
}

bool GUIInputBox::HasCustomCursor(const GUIPhysicalPoint& position, CursorType& type) const
{
	if(IsInInteractionBounds(position) && !IsDisabled())
	{
		type = CursorType::IBeam;
		return true;
	}

	return false;
}

bool GUIInputBox::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseOver)
	{
		if(!IsDisabled())
		{
			if(!mHasFocus)
			{
				AddStateFlags(GUIElementStateFlag::Hover);

				const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
				mState = GUIElementState::Hover;
				const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

				if(originalSize != newSize)
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
			RemoveStateFlags(GUIElementStateFlag::Hover);

			if(!mHasFocus)
			{
				const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
				mState = GUIElementState::Normal;
				const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

				if(originalSize != newSize)
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
				GetGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.GetPosition() - GetTextOffset());
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
					GetGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.GetPosition() - GetTextOffset());
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

	const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

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

	const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();

	return true;
}

bool GUIInputBox::DoOnCommandEvent(const GUICommandEvent& ev)
{
	if(IsDisabled())
		return false;

	bool baseReturn = GUIInteractable::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::Redraw)
	{
		MarkContentAsDirty();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		AddStateFlags(GUIElementStateFlag::Focus);

		const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		mState = GUIElementState::Focused;

		ShowSelection(0);
		GetGUIManager().GetInputSelectionTool()->SelectAll();

		mHasFocus = true;
		mFocusGainedFrame = GetTime().GetCurrentFrameIndex();

		const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		if(originalSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		RemoveStateFlags(GUIElementStateFlag::Focus);

		const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		mState = GUIElementState::Normal;

		HideCaret();
		ClearSelection();

		mHasFocus = false;

		const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		if(originalSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Backspace)
	{
		if(mNumChars > 0)
		{
			const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
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

			const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
			if(originalSize != newSize)
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
			const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
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

			const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
			if(originalSize != newSize)
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
			const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

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

			const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
			if(originalSize != newSize)
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

	GetGUIManager().GetInputCaretTool()->UpdateText(this, BuildTextSpriteInformation());
}

void GUIInputBox::HideCaret()
{
	mCaretShown = false;
}

void GUIInputBox::ShowSelection(u32 anchorCaretPos)
{
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, BuildTextSpriteInformation());

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
	TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GUIPhysicalPoint textOffset = GetTextOffset();
	GUIPhysicalPoint caretPos = GetGUIManager().GetInputCaretTool()->GetCaretPosition() + textOffset;
	GUIPhysicalUnit caretHeight = GetGUIManager().GetInputCaretTool()->GetCaretHeight();
	GUIPhysicalUnit caretWidth = 1;

	GUIPhysicalUnit left = textOffset.X - mTextOffset.X;
	// Include caret width here because we don't want to scroll if just the caret is outside the bounds
	// (Possible if the text width is exactly the maximum width)
	GUIPhysicalUnit right = left + (i32)textSpriteInformation.Width + caretWidth;
	GUIPhysicalUnit top = textOffset.Y - mTextOffset.Y;
	GUIPhysicalUnit bottom = top + (i32)textSpriteInformation.Height;

	// If caret is too high to display we don't want the offset to keep adjusting itself
	caretHeight = Math::Min(caretHeight, bottom - top);
	GUIPhysicalUnit caretRight = caretPos.X + (i32)caretWidth;
	GUIPhysicalUnit caretBottom = caretPos.Y + (i32)caretHeight;

	GUIPhysicalPoint offset{BsZero};
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

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);
}

void GUIInputBox::ClampScrollToBounds(Rect2I unclippedTextBounds)
{
	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GUIPhysicalPoint newTextOffset;
	GUIPhysicalUnit maxScrollableWidth = std::max(0, (i32)unclippedTextBounds.Width - (i32)textSpriteInformation.Width);
	GUIPhysicalUnit maxScrollableHeight = std::max(0, (i32)unclippedTextBounds.Height - (i32)textSpriteInformation.Height);
	newTextOffset.X = Math::Clamp(mTextOffset.X, -maxScrollableWidth, GUIPhysicalUnit(0));
	newTextOffset.Y = Math::Clamp(mTextOffset.Y, -maxScrollableHeight, GUIPhysicalUnit(0));

	if(newTextOffset != mTextOffset)
	{
		mTextOffset = newTextOffset;

		GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
		GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);
	}
}

void GUIInputBox::InsertString(u32 charIdx, const String& string)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);

	mText.insert(mText.begin() + byteIdx, string.begin(), string.end());
	mNumChars = UTF8::Count(mText);

	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);
}

void GUIInputBox::InsertChar(u32 charIdx, u32 charCode)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	String utf8chars = UTF8::FromUtF32(U32String(1, (char32_t)charCode));

	mText.insert(mText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());
	mNumChars = UTF8::Count(mText);

	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);
}

void GUIInputBox::EraseChar(u32 charIdx)
{
	u32 byteIdx = UTF8::CharToByteIndex(mText, charIdx);
	u32 byteCount = UTF8::CharByteCount(mText, charIdx);

	mText.erase(byteIdx, byteCount);
	mNumChars = UTF8::Count(mText);

	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
	GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);
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

		const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();
		GetGUIManager().GetInputCaretTool()->UpdateText(this, textSpriteInformation);
		GetGUIManager().GetInputSelectionTool()->UpdateText(this, textSpriteInformation);

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

GUIPhysicalPoint GUIInputBox::GetTextOffset() const
{
	return GetAbsoluteContentBounds().GetPosition() + mTextOffset;
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
	const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;

	CopyText();
	DeleteSelectedText();

	const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
	if(originalSize != newSize)
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
		const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		InsertString(charIdx, textInClipboard);

		u32 numChars = UTF8::Count(textInClipboard);
		if(numChars > 0)
			GetGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx + (numChars - 1), CARET_AFTER);

		ScrollTextToCaret();

		const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize()).Optimal;
		if(originalSize != newSize)
			MarkLayoutAsDirty();
		else
			MarkContentAsDirty();

		if(!OnValueChanged.Empty())
			OnValueChanged(mText);
	}
}

TextSpriteInformation GUIInputBox::BuildTextSpriteInformation() const
{
	return GUISpriteHelper::BuildTextSpriteInformation(*this, mState, mText, mAbsoluteScale, mIsMultiline);
}

