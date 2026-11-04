//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIInputBox.h"

#include "B3DGUIUtility.h"
#include "GUI/B3DGUIManager.h"
#include "2D/B3DImageSprite.h"
#include "Image/B3DSpriteTexture.h"
#include "2D/B3DTextSprite.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUITextInputEvent.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUICommandEvent.h"
#include "GUI/B3DGUIContextMenu.h"
#include "Utility/B3DTime.h"
#include "Platform/B3DPlatform.h"
#include "String/B3DUnicode.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

namespace
{
	/** Returns @p text with all line breaks converted to the newline character. */
	String NormalizeLineEndings(const String& text)
	{
		if(text.find('\r') == String::npos)
			return text;

		String output;
		output.reserve(text.size());

		for(size_t byteIndex = 0; byteIndex < text.size(); byteIndex++)
		{
			if(text[byteIndex] != '\r')
			{
				output.push_back(text[byteIndex]);
				continue;
			}

			output.push_back('\n');

			if((byteIndex + 1) < text.size() && text[byteIndex + 1] == '\n')
				byteIndex++;
		}

		return output;
	}
} // namespace

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

GUIInputBox::~GUIInputBox()
{
	DestroyEditingSprites();
}

void GUIInputBox::SetText(const String& text)
{
	const String normalizedText = NormalizeLineEndings(text);
	if(mText == normalizedText)
		return;

	if(mFilter != nullptr && !mFilter(normalizedText))
		return;

	const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	const bool hadSelection = HasSelection();

	mText = normalizedText;
	mCharCount = UTF8::Count(mText);

	mCaretPosition = GUITextPosition(mCharCount, false);
	mSelectionAnchorIndex = mHasFocus && hadSelection ? 0 : mCharCount;

	if(mHasFocus)
		ScrollTextToCaret();

	const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	if(originalSize != newSize)
		MarkLayoutAsDirty();
	else
		MarkContentAsDirty();
}

void GUIInputBox::UpdateRenderElements()
{
	mRenderElements.clear();

	GUISpriteHelper::BuildSpriteRenderElements(*this, mBackgroundSprite, Vector2I::kZero, 3);

	GUIContent textContent = GUIContent(HString(mText));
	GUISpriteHelper::BuildSpriteRenderElements(*this, textContent, mTextSprite, mTextOffset.To<i32>(), 1, mIsMultiline);

	GUIManager& guiManager = GetGUIManager();
	GUIWidget* const widget = GetParentWidget();

	// Update the caret, which is only displayed when there is no selection for it to be the end of
	ImageSprite* visibleCaretSprite = nullptr;
	GUIPhysicalArea caretBounds;

	mSelectionAreas.clear();
	if(mHasFocus)
	{
		const GUITextLayout& textLayout = GetTextLayout();

		if(HasSelection())
			textLayout.GetSelectionAreas(GetSelectionStart(), GetSelectionEnd(), mSelectionAreas);
		else if(guiManager.GetCaretBlinkState())
		{
			caretBounds = textLayout.GetCaretBounds(mCaretPosition);

			if(mCaretSprite == nullptr)
				mCaretSprite = B3DNew<ImageSprite>();

			ImageSpriteInformation caretSpriteInformation;
			caretSpriteInformation.Size = Size2I((i32)caretBounds.Width, (i32)caretBounds.Height);
			caretSpriteInformation.Image = guiManager.GetCaretTexture();

			mCaretSprite->Update(caretSpriteInformation, (u64)widget);
			visibleCaretSprite = mCaretSprite;
		}
	}

	// Update the selection, which needs one sprite per selected line
	for(size_t spriteIndex = mSelectionAreas.size(); spriteIndex < mSelectionSprites.size(); spriteIndex++)
		B3DDelete(mSelectionSprites[spriteIndex]);

	const size_t previousSpriteCount = mSelectionSprites.size();
	mSelectionSprites.resize(mSelectionAreas.size());

	for(size_t spriteIndex = 0; spriteIndex < mSelectionSprites.size(); spriteIndex++)
	{
		if(spriteIndex >= previousSpriteCount)
			mSelectionSprites[spriteIndex] = B3DNew<ImageSprite>();

		ImageSpriteInformation selectionSpriteInformation;
		selectionSpriteInformation.Size.Width = mSelectionAreas[spriteIndex].Width;
		selectionSpriteInformation.Size.Height = mSelectionAreas[spriteIndex].Height;
		selectionSpriteInformation.Image = guiManager.GetTextSelectionTexture();

		mSelectionSprites[spriteIndex]->Update(selectionSpriteInformation, (u64)widget);
	}

	// When text bounds are reduced the scroll needs to be adjusted so that
	// input box isn't filled with mostly empty space.
	Vector2I offset = mAbsolutePosition.To<i32>();
	ClampScrollToBounds(mTextSprite.GetTextSprite().GetBounds(offset, Area2I()).To<GUIPhysicalUnit>());

	const GUIPhysicalArea scaledContentBounds = GetScaledContentBounds();

	const GUIPhysicalPoint textOffset = mTextOffset + scaledContentBounds.GetPosition();
	const GUIPhysicalPoint caretOffset = caretBounds.GetPosition() + textOffset;

	// Populate GUI render elements from the sprites
	{
		using T = GUIRenderElementHelper;
		T::Append({ T::SpriteInfo(visibleCaretSprite, 0, caretOffset.To<float>(), scaledContentBounds.To<float>()) }, mRenderElements);

		for(size_t spriteIndex = 0; spriteIndex < mSelectionSprites.size(); spriteIndex++)
		{
			ImageSprite* const sprite = mSelectionSprites[spriteIndex];
			const Area2I& selectionArea = mSelectionAreas[spriteIndex];

			for(u32 elementIndex = 0; elementIndex < sprite->GetRenderElementCount(); elementIndex++)
			{
				mRenderElements.Add(GUIRenderElement());
				GUIRenderElement& renderElement = mRenderElements.Back();

				sprite->GetRenderElement(elementIndex, renderElement);

				renderElement.Depth = 2;
				renderElement.Type = GUIMeshType::Triangle;
				renderElement.Offset = Vector2((float)selectionArea.X, (float)selectionArea.Y) + textOffset.To<float>();
				renderElement.ClipRectangle = scaledContentBounds.To<float>();
				renderElement.UseNewFillBuffer = true;
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
				AddStateFlags(GUIElementState::Hover);

			mIsMouseOver = true;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabled())
		{
			RemoveStateFlags(GUIElementState::Hover);

			mIsMouseOver = false;
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDoubleClick && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabled())
			SelectAll();

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDown && ev.GetButton() == GUIMouseButton::Left)
	{
		if(!IsDisabled())
		{
			// Gaining focus selects all text, and the click that gained it must not undo that unless it extends the selection
			const bool focusGainedThisFrame = mHasFocus && mFocusGainedFrame == GetTime().GetCurrentFrameIndex();
			if(!focusGainedThisFrame || ev.IsShiftDown())
				MoveCaret(GetTextLayout().FindPosition(ev.GetPosition() - GetTextOffset()), ev.IsShiftDown());
		}

		return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragStart)
	{
		if(!IsDisabled())
		{
			if(!ev.IsShiftDown())
				MoveCaret(GetTextLayout().FindPosition(ev.GetDragStartPosition() - GetTextOffset()), false);

			return true;
		}
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDragEnd)
	{
		if(!IsDisabled())
			return true;
	}
	else if(ev.GetType() == GUIMouseEventType::MouseDrag)
	{
		if(!IsDisabled())
		{
			MoveCaret(GetTextLayout().FindPosition(ev.GetPosition() - GetTextOffset()), true);
			return true;
		}
	}

	return false;
}

bool GUIInputBox::DoOnTextInputEvent(const GUITextInputEvent& ev)
{
	if(IsDisabled())
		return false;

	ReplaceText(GetSelectionStart(), GetSelectionEnd(), UTF8::FromUtF32(U32String(1, ev.GetInputChar())));
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
		AddStateFlags(GUIElementState::Focus);

		mHasFocus = true;
		mFocusGainedFrame = GetTime().GetCurrentFrameIndex();

		SelectAll();
		return true;
	}

	if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		RemoveStateFlags(GUIElementState::Focus);

		// The selection itself stays, because focus moves to a context menu while its Cut and Copy items are being chosen
		mHasFocus = false;

		mTextLayout.Clear();
		DestroyEditingSprites();
		MarkContentAsDirty();

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Backspace)
	{
		const u32 caretCharacterIndex = mCaretPosition.CharacterIndex;

		if(HasSelection())
			ReplaceText(GetSelectionStart(), GetSelectionEnd(), String());
		else if(caretCharacterIndex > 0)
			ReplaceText(caretCharacterIndex - 1, caretCharacterIndex, String());

		return true;
	}

	if(ev.GetType() == GUICommandEventType::Delete)
	{
		const u32 caretCharacterIndex = mCaretPosition.CharacterIndex;

		if(HasSelection())
			ReplaceText(GetSelectionStart(), GetSelectionEnd(), String());
		else if(caretCharacterIndex < mCharCount)
			ReplaceText(caretCharacterIndex, caretCharacterIndex + 1, String());

		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveLeft)
	{
		if(HasSelection())
			MoveCaret(GUITextPosition(GetSelectionStart(), true), false);
		else
			MoveCaret(GetTextLayout().GetPositionLeftOf(mCaretPosition), false);

		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectLeft)
	{
		MoveCaret(GetTextLayout().GetPositionLeftOf(mCaretPosition), true);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveRight)
	{
		if(HasSelection())
			MoveCaret(GUITextPosition(GetSelectionEnd(), true), false);
		else
			MoveCaret(GetTextLayout().GetPositionRightOf(mCaretPosition), false);

		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectRight)
	{
		MoveCaret(GetTextLayout().GetPositionRightOf(mCaretPosition), true);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveUp)
	{
		MoveCaretToAdjacentLine(true, false);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectUp)
	{
		MoveCaretToAdjacentLine(true, true);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::MoveDown)
	{
		MoveCaretToAdjacentLine(false, false);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::SelectDown)
	{
		MoveCaretToAdjacentLine(false, true);
		return true;
	}

	if(ev.GetType() == GUICommandEventType::Return)
	{
		if(mIsMultiline)
		{
			ReplaceText(GetSelectionStart(), GetSelectionEnd(), "\n");
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
		SelectAll();

		return true;
	}

	return false;
}

const GUITextLayout& GUIInputBox::GetTextLayout()
{
	mTextLayout.Update(BuildTextSpriteInformation());
	return mTextLayout;
}

void GUIInputBox::MoveCaret(const GUITextPosition& position, bool extendSelection)
{
	mCaretPosition = position;

	if(!extendSelection)
		mSelectionAnchorIndex = position.CharacterIndex;

	ScrollTextToCaret();
	MarkContentAsDirty();
}

void GUIInputBox::MoveCaretToAdjacentLine(bool moveUp, bool extendSelection)
{
	const GUITextLayout& textLayout = GetTextLayout();
	const u32 lineIndex = textLayout.GetLineIndex(mCaretPosition);

	if(moveUp && lineIndex == 0)
		MoveCaret(GUITextPosition(), extendSelection);
	else if(!moveUp && (lineIndex + 1) >= textLayout.GetLineCount())
		MoveCaret(GUITextPosition(mCharCount, false), extendSelection);
	else
	{
		const float caretX = (float)textLayout.GetCaretBounds(mCaretPosition).X;
		MoveCaret(textLayout.FindPositionOnLine(moveUp ? lineIndex - 1 : lineIndex + 1, caretX), extendSelection);
	}
}

void GUIInputBox::SelectAll()
{
	mCaretPosition = GUITextPosition(mCharCount, false);
	mSelectionAnchorIndex = 0;

	MarkContentAsDirty();
}

bool GUIInputBox::HasSelection() const
{
	return mSelectionAnchorIndex != mCaretPosition.CharacterIndex;
}

u32 GUIInputBox::GetSelectionStart() const
{
	return std::min(mSelectionAnchorIndex, mCaretPosition.CharacterIndex);
}

u32 GUIInputBox::GetSelectionEnd() const
{
	return std::max(mSelectionAnchorIndex, mCaretPosition.CharacterIndex);
}

bool GUIInputBox::ReplaceText(u32 startCharacterIndex, u32 endCharacterIndex, const String& replacement)
{
	const String normalizedReplacement = NormalizeLineEndings(replacement);

	const u32 byteStartIndex = UTF8::CharToByteIndex(mText, startCharacterIndex);
	const u32 byteEndIndex = UTF8::CharToByteIndex(mText, endCharacterIndex);

	String newText = mText;
	newText.replace(byteStartIndex, byteEndIndex - byteStartIndex, normalizedReplacement);

	if(mFilter != nullptr && !mFilter(newText))
		return false;

	const GUILogicalSize originalSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());

	mText = std::move(newText);
	mCharCount = UTF8::Count(mText);

	MoveCaret(GUITextPosition(startCharacterIndex + UTF8::Count(normalizedReplacement), true), false);

	const GUILogicalSize newSize = mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize());
	if(originalSize != newSize)
		MarkLayoutAsDirty();

	if(!OnValueChanged.Empty())
		OnValueChanged(mText);

	return true;
}

void GUIInputBox::DestroyEditingSprites()
{
	if(mCaretSprite != nullptr)
	{
		B3DDelete(mCaretSprite);
		mCaretSprite = nullptr;
	}

	for(ImageSprite* sprite : mSelectionSprites)
		B3DDelete(sprite);

	mSelectionSprites.clear();
	mSelectionAreas.clear();
}

void GUIInputBox::ScrollTextToCaret()
{
	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();
	const GUIPhysicalArea caretBounds = GetTextLayout().GetCaretBounds(mCaretPosition);

	GUIPhysicalPoint textOffset = GetTextOffset();
	GUIPhysicalPoint caretPos = caretBounds.GetPosition() + textOffset;
	GUIPhysicalUnit caretHeight = caretBounds.Height;
	GUIPhysicalUnit caretWidth = caretBounds.Width;

	GUIPhysicalUnit left = textOffset.X - mTextOffset.X;
	// Include caret width here because we don't want to scroll if just the caret is outside the bounds
	// (Possible if the text width is exactly the maximum width)
	GUIPhysicalUnit right = left + textSpriteInformation.Size.Width + caretWidth;
	GUIPhysicalUnit top = textOffset.Y - mTextOffset.Y;
	GUIPhysicalUnit bottom = top + textSpriteInformation.Size.Height;

	// If caret is too high to display we don't want the offset to keep adjusting itself
	caretHeight = Math::Min(caretHeight, bottom - top);
	GUIPhysicalUnit caretRight = caretPos.X + caretWidth;
	GUIPhysicalUnit caretBottom = caretPos.Y + caretHeight;

	GUIPhysicalPoint offset{kZeroTag};
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
}

void GUIInputBox::ClampScrollToBounds(const GUIPhysicalArea& unclippedTextBounds)
{
	const TextSpriteInformation textSpriteInformation = BuildTextSpriteInformation();

	GUIPhysicalUnit maxScrollableWidth = Math::Max(unclippedTextBounds.Width - (GUIPhysicalUnit)textSpriteInformation.Size.Width, 0);
	GUIPhysicalUnit maxScrollableHeight = Math::Max(unclippedTextBounds.Height - (GUIPhysicalUnit)textSpriteInformation.Size.Height, 0);

	mTextOffset.X = Math::Clamp(mTextOffset.X, -maxScrollableWidth, GUIPhysicalUnit(0));
	mTextOffset.Y = Math::Clamp(mTextOffset.Y, -maxScrollableHeight, GUIPhysicalUnit(0));
}

GUIPhysicalPoint GUIInputBox::GetTextOffset() const
{
	return GetAbsoluteContentBounds().GetPosition() + mTextOffset;
}

TShared<GUIContextMenu> GUIInputBox::GetContextMenu() const
{
	static TShared<GUIContextMenu> contextMenu;

	if(contextMenu == nullptr)
	{
		contextMenu = B3DMakeShared<GUIContextMenu>();

		contextMenu->AddMenuItem("Cut", [this]() { const_cast<GUIInputBox*>(this)->CutText(); }, 0);
		contextMenu->AddMenuItem("Copy", [this]() { const_cast<GUIInputBox*>(this)->CopyText(); }, 0);
		contextMenu->AddMenuItem("Paste", [this]() { const_cast<GUIInputBox*>(this)->PasteText(); }, 0);

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
	if(!HasSelection())
		return;

	CopyText();
	ReplaceText(GetSelectionStart(), GetSelectionEnd(), String());
}

void GUIInputBox::CopyText()
{
	if(!HasSelection())
		return;

	const u32 byteStartIndex = UTF8::CharToByteIndex(mText, GetSelectionStart());
	const u32 byteEndIndex = UTF8::CharToByteIndex(mText, GetSelectionEnd());

	Platform::CopyToClipboard(mText.substr(byteStartIndex, byteEndIndex - byteStartIndex));
}

void GUIInputBox::PasteText()
{
	ReplaceText(GetSelectionStart(), GetSelectionEnd(), Platform::CopyFromClipboard());
}

TextSpriteInformation GUIInputBox::BuildTextSpriteInformation() const
{
	return GUISpriteHelper::BuildTextSpriteInformation(*this, mText, mAbsoluteScale, mIsMultiline);
}
