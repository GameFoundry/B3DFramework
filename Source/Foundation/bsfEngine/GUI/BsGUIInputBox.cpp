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

namespace bs
{
	VirtualButton GUIInputBox::mCopyVB = VirtualButton("Copy");
	VirtualButton GUIInputBox::mPasteVB = VirtualButton("Paste");
	VirtualButton GUIInputBox::mCutVB = VirtualButton("Cut");
	VirtualButton GUIInputBox::mSelectAllVB = VirtualButton("SelectAll");

	const String& GUIInputBox::GetGUITypeName()
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

	GUIInputBox* GUIInputBox::create(bool multiline, const String& styleName)
	{
		return new (bs_alloc<GUIInputBox>()) GUIInputBox(getStyleName<GUIInputBox>(styleName), GUIDimensions::create(), multiline);
	}

	GUIInputBox* GUIInputBox::create(bool multiline, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIInputBox>()) GUIInputBox(getStyleName<GUIInputBox>(styleName), GUIDimensions::create(options), multiline);
	}

	GUIInputBox* GUIInputBox::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIInputBox>()) GUIInputBox(getStyleName<GUIInputBox>(styleName), GUIDimensions::create(options), false);
	}

	void GUIInputBox::SetText(const String& text)
	{
		if (mText == text)
			return;

		bool filterOkay = true;
		if(mFilter != nullptr)
		{
			filterOkay = mFilter(text);
		}

		if(filterOkay)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

			mText = text;
			mNumChars = UTF8::count(mText);

			if (mHasFocus)
			{
				TEXT_SPRITE_DESC textDesc = getTextDesc();

				gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
				gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

				if (mNumChars > 0)
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(mNumChars - 1, CARET_AFTER);
				else
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);

				if (mSelectionShown)
					gGUIManager().GetInputSelectionTool()->SelectAll();

				scrollTextToCaret();
			}

			Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();
		}
	}

	void GUIInputBox::UpdateRenderElementsInternal()
	{		
		mImageDesc.width = mLayoutData.area.width;
		mImageDesc.height = mLayoutData.area.height;
		mImageDesc.borderLeft = _getStyle()->border.left;
		mImageDesc.borderRight = _getStyle()->border.right;
		mImageDesc.borderTop = _getStyle()->border.top;
		mImageDesc.borderBottom = _getStyle()->border.bottom;
		mImageDesc.color = getTint();

		const HSpriteTexture& activeTex = getActiveTexture();
		if(SpriteTexture::checkIsLoaded(activeTex))
			mImageDesc.texture = activeTex;

		mImageSprite->Update(mImageDesc, (UINT64)_getParentWidget());

		TEXT_SPRITE_DESC textDesc = getTextDesc();
		mTextSprite->Update(textDesc, (UINT64)_getParentWidget());

		ImageSprite* caretSprite = nullptr;
		if(mCaretShown && gGUIManager().GetCaretBlinkState())
		{
			gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
			gGUIManager().GetInputCaretTool()->UpdateSprite();

			caretSprite = gGUIManager().GetInputCaretTool()->GetSprite();
		}

		if(mSelectionShown)
		{
			gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
			gGUIManager().GetInputSelectionTool()->UpdateSprite();
		}

		// When text bounds are reduced the scroll needs to be adjusted so that
		// input box isn't filled with mostly empty space.
		Vector2I Offset(mLayoutData.area.x, mLayoutData.area.y);
		clampScrollToBounds(mTextSprite->GetBounds(offset, Rect2I()));

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mTextSprite, 1), T::SpriteInfo(mImageSprite, 3), T::SpriteInfo(caretSprite) }, mRenderElements);

			if (mSelectionShown)
			{
				const Vector<ImageSprite*>& sprites = gGUIManager().GetInputSelectionTool()->GetSprites();
				for (auto& entry : sprites)
				{
					for (UINT32 i = 0; i < entry->GetNumRenderElements(); i++)
					{
						mRenderElements.Add(GUIRenderElement());
						GUIRenderElement& renderElement = mRenderElements.Back();

						entry->GetRenderElementInfo(i, renderElement);

						renderElement.depth = 2;
						renderElement.type = GUIMeshType::Triangle;
					}
				}
			}
		}

		GUIElement::updateRenderElementsInternal();
	}

	void GUIInputBox::UpdateClippedBounds()
	{
		Vector2I Offset(mLayoutData.area.x, mLayoutData.area.y);
		mClippedBounds = mImageSprite->GetBounds(offset, mLayoutData.GetLocalClipRect());
	}

	Sprite* GUIInputBox::renderElemToSprite(UINT32 renderElemIdx, UINT32& localRenderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
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

		if(mCaretShown && gGUIManager().GetCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().GetInputCaretTool()->GetSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
			{
				localRenderElemIdx = renderElemIdx - oldNumElements;
				return GGUIManager().GetInputCaretTool()->GetSprite();
			}
		}

		if(mSelectionShown)
		{
			const Vector<ImageSprite*>& sprites = gGUIManager().GetInputSelectionTool()->GetSprites();
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

	Vector2I GUIInputBox::RenderElemToOffset(UINT32 renderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
		if(renderElemIdx < newNumElements)
			return GetTextOffset();

		oldNumElements = newNumElements;
		newNumElements += mImageSprite->GetNumRenderElements();

		if(renderElemIdx < newNumElements)
			return Vector2I(mLayoutData.area.x, mLayoutData.area.y);;

		if(mCaretShown && gGUIManager().GetCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().GetInputCaretTool()->GetSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
				return GGUIManager().GetInputCaretTool()->GetSpriteOffset();
		}

		if(mSelectionShown)
		{
			UINT32 spriteIdx = 0;
			const Vector<ImageSprite*>& sprites = gGUIManager().GetInputSelectionTool()->GetSprites();
			for(auto& selectionSprite : sprites)
			{
				oldNumElements = newNumElements;
				newNumElements += selectionSprite->GetNumRenderElements();

				if(renderElemIdx < newNumElements)
					return GGUIManager().GetInputSelectionTool()->GetSelectionSpriteOffset(spriteIdx);

				spriteIdx++;
			}
		}

		return Vector2I();
	}

	Rect2I GUIInputBox::RenderElemToClipRect(UINT32 renderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->GetNumRenderElements();
		if(renderElemIdx < newNumElements)
			return GetTextClipRect();

		oldNumElements = newNumElements;
		newNumElements += mImageSprite->GetNumRenderElements();

		if(renderElemIdx < newNumElements)
			return mLayoutData.GetLocalClipRect();

		if(mCaretShown && gGUIManager().GetCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().GetInputCaretTool()->GetSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
			{
				return GGUIManager().GetInputCaretTool()->GetSpriteClipRect(getTextClipRect());
			}
		}

		if(mSelectionShown)
		{
			UINT32 spriteIdx = 0;
			const Vector<ImageSprite*>& sprites = gGUIManager().GetInputSelectionTool()->GetSprites();
			for(auto& selectionSprite : sprites)
			{
				oldNumElements = newNumElements;
				newNumElements += selectionSprite->GetNumRenderElements();

				if(renderElemIdx < newNumElements)
					return GGUIManager().GetInputSelectionTool()->GetSelectionSpriteClipRect(spriteIdx, getTextClipRect());

				spriteIdx++;
			}
		}

		return Rect2I();
	}

	Vector2I GUIInputBox::_getOptimalSize() const
	{
		UINT32 imageWidth = 0;
		UINT32 imageHeight = 0;

		const HSpriteTexture& activeTex = getActiveTexture();
		if(SpriteTexture::checkIsLoaded(activeTex))
		{
			imageWidth = activeTex->GetWidth();
			imageHeight = activeTex->GetHeight();
		}

		Vector2I contentSize = GUIHelper::calcOptimalContentsSize(mText, *_getStyle(), _getDimensions());
		UINT32 contentWidth = std::max(imageWidth, (UINT32)contentSize.x);
		UINT32 contentHeight = std::max(imageHeight, (UINT32)contentSize.y);

		return Vector2I(contentWidth, contentHeight);
	}

	Vector2I GUIInputBox::_getTextInputOffset() const
	{
		return mTextOffset;	
	}

	Rect2I GUIInputBox::_getTextInputRect() const
	{
		Rect2I textBounds = getCachedContentBounds();
		textBounds.x -= mLayoutData.area.x;
		textBounds.y -= mLayoutData.area.y;

		return textBounds;
	}

	UINT32 GUIInputBox::_getRenderElementDepthRange() const
	{
		return 4;
	}

	bool GUIInputBox::_hasCustomCursor(const Vector2I position, CursorType& type) const
	{
		if(_isInBounds(position) && !_isDisabled())
		{
			type = CursorType::IBeam;
			return true;
		}

		return false;
	}

	void GUIInputBox::_fillBuffer(
		UINT8* vertices,
		UINT32* indices,
		UINT32 vertexOffset,
		UINT32 indexOffset,
		const Vector2I& offset,
		UINT32 maxNumVerts,
		UINT32 maxNumIndices,
		UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		UINT32 localRenderElementIdx;
		Sprite* sprite = renderElemToSprite(renderElementIdx, localRenderElementIdx);
		Vector2I layoutOffset = renderElemToOffset(renderElementIdx) + offset;
		Rect2I clipRect = renderElemToClipRect(renderElementIdx);

		sprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride,
			indexStride, localRenderElementIdx, layoutOffset, clipRect);
	}

	bool GUIInputBox::_mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.GetType() == GUIMouseEventType::MouseOver)
		{
			if (!_isDisabled())
			{
				if (!mHasFocus)
				{
					Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
					mState = State::Hover;
					Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

					if (origSize != newSize)
						_markLayoutAsDirty();
					else
						_markContentAsDirty();
				}

				mIsMouseOver = true;
			}

			return true;
		}
		else If(ev.GetType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				if (!mHasFocus)
				{
					Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
					mState = State::Normal;
					Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

					if (origSize != newSize)
						_markLayoutAsDirty();
					else
						_markContentAsDirty();
				}

				mIsMouseOver = false;
			}

			return true;
		}
		else If(ev.GetType() == GUIMouseEventType::MouseDoubleClick && ev.getButton() == GUIMouseButton::Left)
		{
			if (!_isDisabled())
			{
				showSelection(0);
				gGUIManager().GetInputSelectionTool()->SelectAll();

				_markContentAsDirty();
			}

			return true;
		}
		else If(ev.GetType() == GUIMouseEventType::MouseDown && ev.getButton() == GUIMouseButton::Left)
		{
			if (!_isDisabled())
			{
				if (ev.IsShiftDown())
				{
					if (!mSelectionShown)
						showSelection(gGUIManager().GetInputCaretTool()->GetCaretPos());
				}
				else
				{
					bool focusGainedThisFrame = mHasFocus && mFocusGainedFrame == gTime().GetFrameIdx();

					// We want to select all on focus gain, so don't override that
					if(!focusGainedThisFrame)
						clearSelection();

					showCaret();
				}

				if (mNumChars > 0)
					gGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.getPosition());
				else
					gGUIManager().GetInputCaretTool()->MoveCaretToStart();

				if (ev.IsShiftDown())
					gGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

				scrollTextToCaret();
				_markContentAsDirty();
			}

			return true;
		}
		else If(ev.GetType() == GUIMouseEventType::MouseDragStart)
		{
			if (!_isDisabled())
			{
				if (!ev.IsShiftDown())
				{
					mDragInProgress = true;

					UINT32 caretPos = gGUIManager().GetInputCaretTool()->GetCaretPos();
					showSelection(caretPos);
					gGUIManager().GetInputSelectionTool()->SelectionDragStart(caretPos);
					_markContentAsDirty();

					return true;
				}
			}
		}
		else If(ev.GetType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
			{
				if (!ev.IsShiftDown())
				{
					mDragInProgress = false;

					gGUIManager().GetInputSelectionTool()->SelectionDragEnd();
					_markContentAsDirty();
					return true;
				}
			}
		}
		else If(ev.GetType() == GUIMouseEventType::MouseDrag)
		{
			if (!_isDisabled())
			{
				if (!ev.IsShiftDown())
				{
					if (mNumChars > 0)
						gGUIManager().GetInputCaretTool()->MoveCaretToPos(ev.getPosition());
					else
						gGUIManager().GetInputCaretTool()->MoveCaretToStart();

					gGUIManager().GetInputSelectionTool()->SelectionDragUpdate(gGUIManager().getInputCaretTool()->getCaretPos());

					scrollTextToCaret();
					_markContentAsDirty();
					return true;
				}
			}
		}

		return false;
	}

	bool GUIInputBox::_textInputEvent(const GUITextInputEvent& ev)
	{
		if (_isDisabled())
			return false;

		Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

		if(mSelectionShown)
			deleteSelectedText(true);

		UINT32 charIdx = gGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

		bool filterOkay = true;
		if(mFilter != nullptr)
		{
			String newText = mText;
			UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
			String utf8chars = UTF8::fromUTF32(U32String(1, ev.GetInputChar()));
			newText.Insert(newText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());

			filterOkay = mFilter(newText);
		}

		if(filterOkay)
		{
			insertChar(charIdx, ev.GetInputChar());

			gGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);
			scrollTextToCaret();

			if(!onValueChanged.Empty())
				onValueChanged(mText);
		}

		Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();

		return true;
	}

	bool GUIInputBox::_commandEvent(const GUICommandEvent& ev)
	{
		if (_isDisabled())
			return false;

		bool baseReturn = GUIElement::_commandEvent(ev);

		if(ev.GetType() == GUICommandEventType::Redraw)
		{
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::FocusGained)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			mState = State::Focused;

			showSelection(0);
			gGUIManager().GetInputSelectionTool()->SelectAll();

			mHasFocus = true;
			mFocusGainedFrame = gTime().GetFrameIdx();

			Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			return true;
		}
		
		if(ev.GetType() == GUICommandEventType::FocusLost)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			mState = State::Normal;

			hideCaret();
			clearSelection();

			mHasFocus = false;

			Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			return true;
		}
		
		if(ev.GetType() == GUICommandEventType::Backspace)
		{
			if(mNumChars > 0)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
				if(mSelectionShown)
				{
					deleteSelectedText();
				}
				else
				{
					UINT32 charIdx = gGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos() - 1;

					if(charIdx < mNumChars)
					{
						bool filterOkay = true;
						if(mFilter != nullptr)
						{
							String newText = mText;
							UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
							UINT32 byteCount = UTF8::charByteCount(mText, charIdx);
							newText.Erase(byteIdx, byteCount);

							filterOkay = mFilter(newText);
						}

						if(filterOkay)
						{
							eraseChar(charIdx);

							if (charIdx > 0)
							{
								charIdx--;

								gGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);
							}
							else
								gGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_BEFORE);

							scrollTextToCaret();

							if(!onValueChanged.Empty())
								onValueChanged(mText);
						}
					}
				}

				Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();
			}

			return true;
		}

		if(ev.GetType() == GUICommandEventType::Delete)
		{
			if(mNumChars > 0)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
				if(mSelectionShown)
				{
					deleteSelectedText();
				}
				else
				{
					UINT32 charIdx = gGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();
					if(charIdx < mNumChars)
					{
						bool filterOkay = true;
						if(mFilter != nullptr)
						{
							String newText = mText;
							UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
							UINT32 byteCount = UTF8::charByteCount(mText, charIdx);
							newText.Erase(byteIdx, byteCount);

							filterOkay = mFilter(newText);
						}

						if(filterOkay)
						{
							eraseChar(charIdx);

							if(charIdx > 0)
								charIdx--;

							gGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx, CARET_AFTER);

							scrollTextToCaret();

							if(!onValueChanged.Empty())
								onValueChanged(mText);
						}
					}
				}

				Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();
			}

			return true;
		}

		if(ev.GetType() == GUICommandEventType::MoveLeft)
		{
			if(mSelectionShown)
			{
				UINT32 selStart = gGUIManager().GetInputSelectionTool()->GetSelectionStart();
				clearSelection();

				if (!mCaretShown)
					showCaret();

				if(selStart > 0)
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(selStart - 1, CARET_AFTER);
				else
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
			}
			else
				gGUIManager().GetInputCaretTool()->MoveCaretLeft();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::SelectLeft)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().GetInputCaretTool()->GetCaretPos());

			gGUIManager().GetInputCaretTool()->MoveCaretLeft();
			gGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::MoveRight)
		{
			if(mSelectionShown)
			{
				UINT32 selEnd = gGUIManager().GetInputSelectionTool()->GetSelectionEnd();
				clearSelection();

				if (!mCaretShown)
					showCaret();

				if(selEnd > 0)
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(selEnd - 1, CARET_AFTER);
				else
					gGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
			}
			else
				gGUIManager().GetInputCaretTool()->MoveCaretRight();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::SelectRight)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().GetInputCaretTool()->GetCaretPos());

			gGUIManager().GetInputCaretTool()->MoveCaretRight();
			gGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::MoveUp)
		{
			if (mSelectionShown)
				clearSelection();

			if (!mCaretShown)
				showCaret();

			gGUIManager().GetInputCaretTool()->MoveCaretUp();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::SelectUp)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().GetInputCaretTool()->GetCaretPos());;

			gGUIManager().GetInputCaretTool()->MoveCaretUp();
			gGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::MoveDown)
		{
			if (mSelectionShown)
				clearSelection();

			if (!mCaretShown)
				showCaret();

			gGUIManager().GetInputCaretTool()->MoveCaretDown();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::SelectDown)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().GetInputCaretTool()->GetCaretPos());

			gGUIManager().GetInputCaretTool()->MoveCaretDown();
			gGUIManager().GetInputSelectionTool()->MoveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.GetType() == GUICommandEventType::Return)
		{
			if (mIsMultiline)
			{
				Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

				if (mSelectionShown)
					deleteSelectedText();

				UINT32 charIdx = gGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

				bool filterOkay = true;
				if (mFilter != nullptr)
				{
					String newText = mText;
					UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
					newText.Insert(newText.begin() + byteIdx, '\n');

					filterOkay = mFilter(newText);
				}

				if (filterOkay)
				{
					insertChar(charIdx, '\n');

					gGUIManager().GetInputCaretTool()->MoveCaretRight();
					scrollTextToCaret();

					if (!onValueChanged.Empty())
						onValueChanged(mText);
				}

				Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();

				return true;
			}
		}

		if (ev.GetType() == GUICommandEventType::Confirm)
		{
			onConfirm();
			return true;
		}

		return baseReturn;
	}

	bool GUIInputBox::_virtualButtonEvent(const GUIVirtualButtonEvent& ev)
	{
		if (_isDisabled())
			return false;

		if(ev.GetButton() == mCutVB)
		{
			cutText();

			return true;
		}
		else If(ev.GetButton() == mCopyVB)
		{
			copyText();

			return true;
		}
		else If(ev.GetButton() == mPasteVB)
		{
			pasteText();
			return true;
		}
		else If(ev.GetButton() == mSelectAllVB)
		{
			showSelection(0);
			gGUIManager().GetInputSelectionTool()->SelectAll();
			_markContentAsDirty();

			return true;
		}

		return false;
	}

	void GUIInputBox::ShowCaret()
	{
		mCaretShown = true;

		TEXT_SPRITE_DESC textDesc = getTextDesc();
		gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
	}

	void GUIInputBox::HideCaret()
	{
		mCaretShown = false;
	}

	void GUIInputBox::ShowSelection(UINT32 anchorCaretPos)
	{
		TEXT_SPRITE_DESC textDesc = getTextDesc();
		gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

		gGUIManager().GetInputSelectionTool()->ShowSelection(anchorCaretPos);
		mSelectionShown = true;
	}

	void GUIInputBox::ClearSelection()
	{
		gGUIManager().GetInputSelectionTool()->ClearSelectionVisuals();
		mSelectionShown = false;
	}

	void GUIInputBox::ScrollTextToCaret()
	{
		TEXT_SPRITE_DESC textDesc = getTextDesc();

		Vector2I textOffset = getTextOffset();
		Vector2I caretPos = gGUIManager().GetInputCaretTool()->GetCaretPosition(textOffset);
		UINT32 caretHeight = gGUIManager().GetInputCaretTool()->GetCaretHeight();
		UINT32 caretWidth = 1;

		INT32 left = textOffset.x - mTextOffset.x;
		// Include caret width here because we don't want to scroll if just the caret is outside the bounds
		// (Possible if the text width is exactly the maximum width)
		INT32 right = left + (INT32)textDesc.width + caretWidth;
		INT32 top = textOffset.y - mTextOffset.y;
		INT32 bottom = top + (INT32)textDesc.height;

		// If caret is too high to display we don't want the offset to keep adjusting itself
		caretHeight = std::min(caretHeight, (UINT32)(bottom - top));
		INT32 caretRight = caretPos.x + (INT32)caretWidth;
		INT32 caretBottom = caretPos.y + (INT32)caretHeight;

		Vector2I offset;
		if(caretPos.x < left)
		{
			offset.x = left - caretPos.x;
		}
		else If(caretRight > right)
		{
			offset.x = -(caretRight - right);
		}

		if(caretPos.y < top)
		{
			offset.y = top - caretPos.y;
		}
		else If(caretBottom > bottom)
		{
			offset.y = -(caretBottom - bottom);
		}

		mTextOffset += offset;

		gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
	}

	void GUIInputBox::ClampScrollToBounds(Rect2I unclippedTextBounds)
	{
		TEXT_SPRITE_DESC textDesc = getTextDesc();

		Vector2I newTextOffset;
		INT32 maxScrollableWidth = std::max(0, (INT32)unclippedTextBounds.width - (INT32)textDesc.width);
		INT32 maxScrollableHeight = std::max(0, (INT32)unclippedTextBounds.height - (INT32)textDesc.height);
		newTextOffset.x = Math::clamp(mTextOffset.x, -maxScrollableWidth, 0);
		newTextOffset.y = Math::clamp(mTextOffset.y, -maxScrollableHeight, 0);

		if(newTextOffset != mTextOffset)
		{
			mTextOffset = newTextOffset;

			gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
			gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
		}
	}

	void GUIInputBox::InsertString(UINT32 charIdx, const String& string)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);

		mText.Insert(mText.begin() + byteIdx, string.begin(), string.end());
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
	}

	void GUIInputBox::InsertChar(UINT32 charIdx, UINT32 charCode)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
		String utf8chars = UTF8::fromUTF32(U32String(1, (char32_t)charCode));

		mText.Insert(mText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
	}

	void GUIInputBox::EraseChar(UINT32 charIdx)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
		UINT32 byteCount = UTF8::charByteCount(mText, charIdx);

		mText.Erase(byteIdx, byteCount);
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
		gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);
	}

	void GUIInputBox::DeleteSelectedText(bool internal)
	{
		UINT32 selStart = gGUIManager().GetInputSelectionTool()->GetSelectionStart();
		UINT32 selEnd = gGUIManager().GetInputSelectionTool()->GetSelectionEnd();

		UINT32 byteStart = UTF8::charToByteIndex(mText, selStart);
		UINT32 byteEnd = UTF8::charToByteIndex(mText, selEnd);

		bool filterOkay = true;
		if (!internal && mFilter != nullptr)
		{
			String newText = mText;
			newText.Erase(newText.begin() + byteStart, newText.begin() + byteEnd);

			filterOkay = mFilter(newText);
		}

		if (!mCaretShown)
			showCaret();

		if(filterOkay)
		{
			mText.Erase(mText.begin() + byteStart, mText.begin() + byteEnd);
			mNumChars = UTF8::count(mText);

			TEXT_SPRITE_DESC textDesc = getTextDesc();
			gGUIManager().GetInputCaretTool()->UpdateText(this, textDesc);
			gGUIManager().GetInputSelectionTool()->UpdateText(this, textDesc);

			if(selStart > 0)
			{
				UINT32 newCaretPos = selStart - 1;
				gGUIManager().GetInputCaretTool()->MoveCaretToChar(newCaretPos, CARET_AFTER);
			}
			else
			{
				gGUIManager().GetInputCaretTool()->MoveCaretToChar(0, CARET_BEFORE);
			}

			scrollTextToCaret();

			if (!internal)
				onValueChanged(mText);
		}

		clearSelection();
	}

	String GUIInputBox::GetSelectedText()
	{
		UINT32 selStart = gGUIManager().GetInputSelectionTool()->GetSelectionStart();
		UINT32 selEnd = gGUIManager().GetInputSelectionTool()->GetSelectionEnd();

		UINT32 byteStart = UTF8::charToByteIndex(mText, selStart);
		UINT32 byteEnd = UTF8::charToByteIndex(mText, selEnd);

		return mText.Substr(byteStart, byteEnd - byteStart);
	}

	Vector2I GUIInputBox::GetTextOffset() const
	{
		Rect2I textBounds = getCachedContentBounds();
		return Vector2I(textBounds.x, textBounds.y) + mTextOffset;
	}

	Rect2I GUIInputBox::GetTextClipRect() const
	{
		Rect2I contentClipRect = getCachedContentClipRect();
		return Rect2I(contentClipRect.x - mTextOffset.x, contentClipRect.y - mTextOffset.y, contentClipRect.width, contentClipRect.height);
	}

	TEXT_SPRITE_DESC GUIInputBox::GetTextDesc() const
	{
		TEXT_SPRITE_DESC textDesc;
		textDesc.text = mText;
		textDesc.font = _getStyle()->font;
		textDesc.fontSize = _getStyle()->fontSize;
		textDesc.color = getTint() * getActiveTextColor();

		Rect2I textBounds = getCachedContentBounds();
		textDesc.width = textBounds.width;
		textDesc.height = textBounds.height;
		textDesc.horzAlign = _getStyle()->textHorzAlign;
		textDesc.vertAlign = _getStyle()->textVertAlign;
		textDesc.wordWrap = mIsMultiline;

		return textDesc;
	}

	const HSpriteTexture& GUIInputBox::GetActiveTexture() const
	{
		switch(mState)
		{
		case State::Focused:
			return _getStyle()->focused.texture;
		case State::Hover:
			return _getStyle()->hover.texture;
		case State::Normal:
			return _getStyle()->normal.texture;
		}

		return _getStyle()->normal.texture;
	}

	Color GUIInputBox::GetActiveTextColor() const
	{
		switch (mState)
		{
		case State::Focused:
			return _getStyle()->focused.textColor;
		case State::Hover:
			return _getStyle()->hover.textColor;
		case State::Normal:
			return _getStyle()->normal.textColor;
		}

		return _getStyle()->normal.textColor;
	}

	SPtr<GUIContextMenu> GUIInputBox::_getContextMenu() const
	{
		static SPtr<GUIContextMenu> contextMenu;

		if (contextMenu == nullptr)
		{
			contextMenu = bs_shared_ptr_new<GUIContextMenu>();

			contextMenu->AddMenuItem("Cut", std::bind(&GUIInputBox::cutText, const_cast<GUIInputBox*>(this)), 0);
			contextMenu->AddMenuItem("Copy", std::bind(&GUIInputBox::copyText, const_cast<GUIInputBox*>(this)), 0);
			contextMenu->AddMenuItem("Paste", std::bind(&GUIInputBox::pasteText, const_cast<GUIInputBox*>(this)), 0);

			contextMenu->SetLocalizedName("Cut", HString("Cut"));
			contextMenu->SetLocalizedName("Copy", HString("Copy"));
			contextMenu->SetLocalizedName("Paste", HString("Paste"));
		}

		if (!_isDisabled())
			return contextMenu;

		return nullptr;
	}

	void GUIInputBox::CutText()
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

		copyText();
		deleteSelectedText();

		Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	void GUIInputBox::CopyText()
	{
		Platform::copyToClipboard(getSelectedText());
	}

	void GUIInputBox::PasteText()
	{
		deleteSelectedText(true);

		String textInClipboard = Platform::copyFromClipboard();
		UINT32 charIdx = gGUIManager().GetInputCaretTool()->GetCharIdxAtCaretPos();

		bool filterOkay = true;
		if(mFilter != nullptr)
		{
			String newText = mText;

			UINT32 byteIdx = UTF8::charToByteIndex(newText, charIdx);
			newText.Insert(newText.begin() + byteIdx, textInClipboard.begin(), textInClipboard.end());

			filterOkay = mFilter(newText);
		}

		if(filterOkay)
		{
			Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			insertString(charIdx, textInClipboard);

			UINT32 numChars = UTF8::count(textInClipboard);
			if(numChars > 0)
				gGUIManager().GetInputCaretTool()->MoveCaretToChar(charIdx + (numChars - 1), CARET_AFTER);

			scrollTextToCaret();

			Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			if(!onValueChanged.Empty())
				onValueChanged(mText);
		}
	}
}
