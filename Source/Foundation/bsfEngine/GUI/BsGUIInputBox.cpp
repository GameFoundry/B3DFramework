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

	const String& GUIInputBox::getGUITypeName()
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

	void GUIInputBox::setText(const String& text)
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
			Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

			mText = text;
			mNumChars = UTF8::count(mText);

			if (mHasFocus)
			{
				TEXT_SPRITE_DESC textDesc = getTextDesc();

				gGUIManager().getInputCaretTool()->updateText(this, textDesc);
				gGUIManager().getInputSelectionTool()->updateText(this, textDesc);

				if (mNumChars > 0)
					gGUIManager().getInputCaretTool()->moveCaretToChar(mNumChars - 1, CARET_AFTER);
				else
					gGUIManager().getInputCaretTool()->moveCaretToChar(0, CARET_BEFORE);

				if (mSelectionShown)
					gGUIManager().getInputSelectionTool()->selectAll();

				scrollTextToCaret();
			}

			Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();
		}
	}

	void GUIInputBox::updateRenderElementsInternal()
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

		mImageSprite->update(mImageDesc, (UINT64)_getParentWidget());

		TEXT_SPRITE_DESC textDesc = getTextDesc();
		mTextSprite->update(textDesc, (UINT64)_getParentWidget());

		ImageSprite* caretSprite = nullptr;
		if(mCaretShown && gGUIManager().getCaretBlinkState())
		{
			gGUIManager().getInputCaretTool()->updateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
			gGUIManager().getInputCaretTool()->updateSprite();

			caretSprite = gGUIManager().getInputCaretTool()->getSprite();
		}

		if(mSelectionShown)
		{
			gGUIManager().getInputSelectionTool()->updateText(this, textDesc); // TODO - These shouldn't be here. Only call this when one of these parameters changes.
			gGUIManager().getInputSelectionTool()->updateSprite();
		}

		// When text bounds are reduced the scroll needs to be adjusted so that
		// input box isn't filled with mostly empty space.
		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		clampScrollToBounds(mTextSprite->getBounds(offset, Rect2I()));

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mTextSprite, 1), T::SpriteInfo(mImageSprite, 3), T::SpriteInfo(caretSprite) }, mRenderElements);

			if (mSelectionShown)
			{
				const Vector<ImageSprite*>& sprites = gGUIManager().getInputSelectionTool()->getSprites();
				for (auto& entry : sprites)
				{
					for (UINT32 i = 0; i < entry->getNumRenderElements(); i++)
					{
						mRenderElements.add(GUIRenderElement());
						GUIRenderElement& renderElement = mRenderElements.back();

						entry->getRenderElementInfo(i, renderElement);

						renderElement.depth = 2;
						renderElement.type = GUIMeshType::Triangle;
					}
				}
			}
		}

		GUIElement::updateRenderElementsInternal();
	}

	void GUIInputBox::updateClippedBounds()
	{
		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		mClippedBounds = mImageSprite->getBounds(offset, mLayoutData.getLocalClipRect());
	}

	Sprite* GUIInputBox::renderElemToSprite(UINT32 renderElemIdx, UINT32& localRenderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->getNumRenderElements();
		if(renderElemIdx < newNumElements)
		{
			localRenderElemIdx = renderElemIdx - oldNumElements;
			return mTextSprite;
		}

		oldNumElements = newNumElements;
		newNumElements += mImageSprite->getNumRenderElements();

		if(renderElemIdx < newNumElements)
		{
			localRenderElemIdx = renderElemIdx - oldNumElements;
			return mImageSprite;
		}

		if(mCaretShown && gGUIManager().getCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().getInputCaretTool()->getSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
			{
				localRenderElemIdx = renderElemIdx - oldNumElements;
				return gGUIManager().getInputCaretTool()->getSprite();
			}
		}

		if(mSelectionShown)
		{
			const Vector<ImageSprite*>& sprites = gGUIManager().getInputSelectionTool()->getSprites();
			for(auto& selectionSprite : sprites)
			{
				oldNumElements = newNumElements;
				newNumElements += selectionSprite->getNumRenderElements();

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

	Vector2I GUIInputBox::renderElemToOffset(UINT32 renderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->getNumRenderElements();
		if(renderElemIdx < newNumElements)
			return getTextOffset();

		oldNumElements = newNumElements;
		newNumElements += mImageSprite->getNumRenderElements();

		if(renderElemIdx < newNumElements)
			return Vector2I(mLayoutData.area.x, mLayoutData.area.y);;

		if(mCaretShown && gGUIManager().getCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().getInputCaretTool()->getSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
				return gGUIManager().getInputCaretTool()->getSpriteOffset();
		}

		if(mSelectionShown)
		{
			UINT32 spriteIdx = 0;
			const Vector<ImageSprite*>& sprites = gGUIManager().getInputSelectionTool()->getSprites();
			for(auto& selectionSprite : sprites)
			{
				oldNumElements = newNumElements;
				newNumElements += selectionSprite->getNumRenderElements();

				if(renderElemIdx < newNumElements)
					return gGUIManager().getInputSelectionTool()->getSelectionSpriteOffset(spriteIdx);

				spriteIdx++;
			}
		}

		return Vector2I();
	}

	Rect2I GUIInputBox::renderElemToClipRect(UINT32 renderElemIdx) const
	{
		UINT32 oldNumElements = 0;
		UINT32 newNumElements = oldNumElements + mTextSprite->getNumRenderElements();
		if(renderElemIdx < newNumElements)
			return getTextClipRect();

		oldNumElements = newNumElements;
		newNumElements += mImageSprite->getNumRenderElements();

		if(renderElemIdx < newNumElements)
			return mLayoutData.getLocalClipRect();

		if(mCaretShown && gGUIManager().getCaretBlinkState())
		{
			oldNumElements = newNumElements;
			newNumElements += gGUIManager().getInputCaretTool()->getSprite()->getNumRenderElements();

			if(renderElemIdx < newNumElements)
			{
				return gGUIManager().getInputCaretTool()->getSpriteClipRect(getTextClipRect());
			}
		}

		if(mSelectionShown)
		{
			UINT32 spriteIdx = 0;
			const Vector<ImageSprite*>& sprites = gGUIManager().getInputSelectionTool()->getSprites();
			for(auto& selectionSprite : sprites)
			{
				oldNumElements = newNumElements;
				newNumElements += selectionSprite->getNumRenderElements();

				if(renderElemIdx < newNumElements)
					return gGUIManager().getInputSelectionTool()->getSelectionSpriteClipRect(spriteIdx, getTextClipRect());

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
			imageWidth = activeTex->getWidth();
			imageHeight = activeTex->getHeight();
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
		if(ev.getType() == GUIMouseEventType::MouseOver)
		{
			if (!_isDisabled())
			{
				if (!mHasFocus)
				{
					Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
					mState = State::Hover;
					Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

					if (origSize != newSize)
						_markLayoutAsDirty();
					else
						_markContentAsDirty();
				}

				mIsMouseOver = true;
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				if (!mHasFocus)
				{
					Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
					mState = State::Normal;
					Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

					if (origSize != newSize)
						_markLayoutAsDirty();
					else
						_markContentAsDirty();
				}

				mIsMouseOver = false;
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDoubleClick && ev.getButton() == GUIMouseButton::Left)
		{
			if (!_isDisabled())
			{
				showSelection(0);
				gGUIManager().getInputSelectionTool()->selectAll();

				_markContentAsDirty();
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDown && ev.getButton() == GUIMouseButton::Left)
		{
			if (!_isDisabled())
			{
				if (ev.isShiftDown())
				{
					if (!mSelectionShown)
						showSelection(gGUIManager().getInputCaretTool()->getCaretPos());
				}
				else
				{
					bool focusGainedThisFrame = mHasFocus && mFocusGainedFrame == gTime().getFrameIdx();

					// We want to select all on focus gain, so don't override that
					if(!focusGainedThisFrame)
						clearSelection();

					showCaret();
				}

				if (mNumChars > 0)
					gGUIManager().getInputCaretTool()->moveCaretToPos(ev.getPosition());
				else
					gGUIManager().getInputCaretTool()->moveCaretToStart();

				if (ev.isShiftDown())
					gGUIManager().getInputSelectionTool()->moveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

				scrollTextToCaret();
				_markContentAsDirty();
			}

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragStart)
		{
			if (!_isDisabled())
			{
				if (!ev.isShiftDown())
				{
					mDragInProgress = true;

					UINT32 caretPos = gGUIManager().getInputCaretTool()->getCaretPos();
					showSelection(caretPos);
					gGUIManager().getInputSelectionTool()->selectionDragStart(caretPos);
					_markContentAsDirty();

					return true;
				}
			}
		}
		else if(ev.getType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
			{
				if (!ev.isShiftDown())
				{
					mDragInProgress = false;

					gGUIManager().getInputSelectionTool()->selectionDragEnd();
					_markContentAsDirty();
					return true;
				}
			}
		}
		else if(ev.getType() == GUIMouseEventType::MouseDrag)
		{
			if (!_isDisabled())
			{
				if (!ev.isShiftDown())
				{
					if (mNumChars > 0)
						gGUIManager().getInputCaretTool()->moveCaretToPos(ev.getPosition());
					else
						gGUIManager().getInputCaretTool()->moveCaretToStart();

					gGUIManager().getInputSelectionTool()->selectionDragUpdate(gGUIManager().getInputCaretTool()->getCaretPos());

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

		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if(mSelectionShown)
			deleteSelectedText(true);

		UINT32 charIdx = gGUIManager().getInputCaretTool()->getCharIdxAtCaretPos();

		bool filterOkay = true;
		if(mFilter != nullptr)
		{
			String newText = mText;
			UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
			String utf8chars = UTF8::fromUTF32(U32String(1, ev.getInputChar()));
			newText.insert(newText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());

			filterOkay = mFilter(newText);
		}

		if(filterOkay)
		{
			insertChar(charIdx, ev.getInputChar());

			gGUIManager().getInputCaretTool()->moveCaretToChar(charIdx, CARET_AFTER);
			scrollTextToCaret();

			if(!onValueChanged.empty())
				onValueChanged(mText);
		}

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
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

		if(ev.getType() == GUICommandEventType::Redraw)
		{
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::FocusGained)
		{
			Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			mState = State::Focused;

			showSelection(0);
			gGUIManager().getInputSelectionTool()->selectAll();

			mHasFocus = true;
			mFocusGainedFrame = gTime().getFrameIdx();

			Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			return true;
		}
		
		if(ev.getType() == GUICommandEventType::FocusLost)
		{
			Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			mState = State::Normal;

			hideCaret();
			clearSelection();

			mHasFocus = false;

			Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			return true;
		}
		
		if(ev.getType() == GUICommandEventType::Backspace)
		{
			if(mNumChars > 0)
			{
				Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
				if(mSelectionShown)
				{
					deleteSelectedText();
				}
				else
				{
					UINT32 charIdx = gGUIManager().getInputCaretTool()->getCharIdxAtCaretPos() - 1;

					if(charIdx < mNumChars)
					{
						bool filterOkay = true;
						if(mFilter != nullptr)
						{
							String newText = mText;
							UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
							UINT32 byteCount = UTF8::charByteCount(mText, charIdx);
							newText.erase(byteIdx, byteCount);

							filterOkay = mFilter(newText);
						}

						if(filterOkay)
						{
							eraseChar(charIdx);

							if (charIdx > 0)
							{
								charIdx--;

								gGUIManager().getInputCaretTool()->moveCaretToChar(charIdx, CARET_AFTER);
							}
							else
								gGUIManager().getInputCaretTool()->moveCaretToChar(charIdx, CARET_BEFORE);

							scrollTextToCaret();

							if(!onValueChanged.empty())
								onValueChanged(mText);
						}
					}
				}

				Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();
			}

			return true;
		}

		if(ev.getType() == GUICommandEventType::Delete)
		{
			if(mNumChars > 0)
			{
				Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
				if(mSelectionShown)
				{
					deleteSelectedText();
				}
				else
				{
					UINT32 charIdx = gGUIManager().getInputCaretTool()->getCharIdxAtCaretPos();
					if(charIdx < mNumChars)
					{
						bool filterOkay = true;
						if(mFilter != nullptr)
						{
							String newText = mText;
							UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
							UINT32 byteCount = UTF8::charByteCount(mText, charIdx);
							newText.erase(byteIdx, byteCount);

							filterOkay = mFilter(newText);
						}

						if(filterOkay)
						{
							eraseChar(charIdx);

							if(charIdx > 0)
								charIdx--;

							gGUIManager().getInputCaretTool()->moveCaretToChar(charIdx, CARET_AFTER);

							scrollTextToCaret();

							if(!onValueChanged.empty())
								onValueChanged(mText);
						}
					}
				}

				Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();
			}

			return true;
		}

		if(ev.getType() == GUICommandEventType::MoveLeft)
		{
			if(mSelectionShown)
			{
				UINT32 selStart = gGUIManager().getInputSelectionTool()->getSelectionStart();
				clearSelection();

				if (!mCaretShown)
					showCaret();

				if(selStart > 0)
					gGUIManager().getInputCaretTool()->moveCaretToChar(selStart - 1, CARET_AFTER);
				else
					gGUIManager().getInputCaretTool()->moveCaretToChar(0, CARET_BEFORE);
			}
			else
				gGUIManager().getInputCaretTool()->moveCaretLeft();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::SelectLeft)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().getInputCaretTool()->getCaretPos());

			gGUIManager().getInputCaretTool()->moveCaretLeft();
			gGUIManager().getInputSelectionTool()->moveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::MoveRight)
		{
			if(mSelectionShown)
			{
				UINT32 selEnd = gGUIManager().getInputSelectionTool()->getSelectionEnd();
				clearSelection();

				if (!mCaretShown)
					showCaret();

				if(selEnd > 0)
					gGUIManager().getInputCaretTool()->moveCaretToChar(selEnd - 1, CARET_AFTER);
				else
					gGUIManager().getInputCaretTool()->moveCaretToChar(0, CARET_BEFORE);
			}
			else
				gGUIManager().getInputCaretTool()->moveCaretRight();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::SelectRight)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().getInputCaretTool()->getCaretPos());

			gGUIManager().getInputCaretTool()->moveCaretRight();
			gGUIManager().getInputSelectionTool()->moveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::MoveUp)
		{
			if (mSelectionShown)
				clearSelection();

			if (!mCaretShown)
				showCaret();

			gGUIManager().getInputCaretTool()->moveCaretUp();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::SelectUp)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().getInputCaretTool()->getCaretPos());;

			gGUIManager().getInputCaretTool()->moveCaretUp();
			gGUIManager().getInputSelectionTool()->moveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::MoveDown)
		{
			if (mSelectionShown)
				clearSelection();

			if (!mCaretShown)
				showCaret();

			gGUIManager().getInputCaretTool()->moveCaretDown();

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::SelectDown)
		{
			if(!mSelectionShown)
				showSelection(gGUIManager().getInputCaretTool()->getCaretPos());

			gGUIManager().getInputCaretTool()->moveCaretDown();
			gGUIManager().getInputSelectionTool()->moveSelectionToCaret(gGUIManager().getInputCaretTool()->getCaretPos());

			scrollTextToCaret();
			_markContentAsDirty();
			return true;
		}

		if(ev.getType() == GUICommandEventType::Return)
		{
			if (mIsMultiline)
			{
				Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

				if (mSelectionShown)
					deleteSelectedText();

				UINT32 charIdx = gGUIManager().getInputCaretTool()->getCharIdxAtCaretPos();

				bool filterOkay = true;
				if (mFilter != nullptr)
				{
					String newText = mText;
					UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
					newText.insert(newText.begin() + byteIdx, '\n');

					filterOkay = mFilter(newText);
				}

				if (filterOkay)
				{
					insertChar(charIdx, '\n');

					gGUIManager().getInputCaretTool()->moveCaretRight();
					scrollTextToCaret();

					if (!onValueChanged.empty())
						onValueChanged(mText);
				}

				Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
				if (origSize != newSize)
					_markLayoutAsDirty();
				else
					_markContentAsDirty();

				return true;
			}
		}

		if (ev.getType() == GUICommandEventType::Confirm)
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

		if(ev.getButton() == mCutVB)
		{
			cutText();

			return true;
		}
		else if(ev.getButton() == mCopyVB)
		{
			copyText();

			return true;
		}
		else if(ev.getButton() == mPasteVB)
		{
			pasteText();
			return true;
		}
		else if(ev.getButton() == mSelectAllVB)
		{
			showSelection(0);
			gGUIManager().getInputSelectionTool()->selectAll();
			_markContentAsDirty();

			return true;
		}

		return false;
	}

	void GUIInputBox::showCaret()
	{
		mCaretShown = true;

		TEXT_SPRITE_DESC textDesc = getTextDesc();
		gGUIManager().getInputCaretTool()->updateText(this, textDesc);
	}

	void GUIInputBox::hideCaret()
	{
		mCaretShown = false;
	}

	void GUIInputBox::showSelection(UINT32 anchorCaretPos)
	{
		TEXT_SPRITE_DESC textDesc = getTextDesc();
		gGUIManager().getInputSelectionTool()->updateText(this, textDesc);

		gGUIManager().getInputSelectionTool()->showSelection(anchorCaretPos);
		mSelectionShown = true;
	}

	void GUIInputBox::clearSelection()
	{
		gGUIManager().getInputSelectionTool()->clearSelectionVisuals();
		mSelectionShown = false;
	}

	void GUIInputBox::scrollTextToCaret()
	{
		TEXT_SPRITE_DESC textDesc = getTextDesc();

		Vector2I textOffset = getTextOffset();
		Vector2I caretPos = gGUIManager().getInputCaretTool()->getCaretPosition(textOffset);
		UINT32 caretHeight = gGUIManager().getInputCaretTool()->getCaretHeight();
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
		else if(caretRight > right)
		{
			offset.x = -(caretRight - right);
		}

		if(caretPos.y < top)
		{
			offset.y = top - caretPos.y;
		}
		else if(caretBottom > bottom)
		{
			offset.y = -(caretBottom - bottom);
		}

		mTextOffset += offset;

		gGUIManager().getInputCaretTool()->updateText(this, textDesc);
		gGUIManager().getInputSelectionTool()->updateText(this, textDesc);
	}

	void GUIInputBox::clampScrollToBounds(Rect2I unclippedTextBounds)
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

			gGUIManager().getInputCaretTool()->updateText(this, textDesc);
			gGUIManager().getInputSelectionTool()->updateText(this, textDesc);
		}
	}

	void GUIInputBox::insertString(UINT32 charIdx, const String& string)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);

		mText.insert(mText.begin() + byteIdx, string.begin(), string.end());
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().getInputCaretTool()->updateText(this, textDesc);
		gGUIManager().getInputSelectionTool()->updateText(this, textDesc);
	}

	void GUIInputBox::insertChar(UINT32 charIdx, UINT32 charCode)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
		String utf8chars = UTF8::fromUTF32(U32String(1, (char32_t)charCode));

		mText.insert(mText.begin() + byteIdx, utf8chars.begin(), utf8chars.end());
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().getInputCaretTool()->updateText(this, textDesc);
		gGUIManager().getInputSelectionTool()->updateText(this, textDesc);
	}

	void GUIInputBox::eraseChar(UINT32 charIdx)
	{
		UINT32 byteIdx = UTF8::charToByteIndex(mText, charIdx);
		UINT32 byteCount = UTF8::charByteCount(mText, charIdx);

		mText.erase(byteIdx, byteCount);
		mNumChars = UTF8::count(mText);

		TEXT_SPRITE_DESC textDesc = getTextDesc();

		gGUIManager().getInputCaretTool()->updateText(this, textDesc);
		gGUIManager().getInputSelectionTool()->updateText(this, textDesc);
	}

	void GUIInputBox::deleteSelectedText(bool internal)
	{
		UINT32 selStart = gGUIManager().getInputSelectionTool()->getSelectionStart();
		UINT32 selEnd = gGUIManager().getInputSelectionTool()->getSelectionEnd();

		UINT32 byteStart = UTF8::charToByteIndex(mText, selStart);
		UINT32 byteEnd = UTF8::charToByteIndex(mText, selEnd);

		bool filterOkay = true;
		if (!internal && mFilter != nullptr)
		{
			String newText = mText;
			newText.erase(newText.begin() + byteStart, newText.begin() + byteEnd);

			filterOkay = mFilter(newText);
		}

		if (!mCaretShown)
			showCaret();

		if(filterOkay)
		{
			mText.erase(mText.begin() + byteStart, mText.begin() + byteEnd);
			mNumChars = UTF8::count(mText);

			TEXT_SPRITE_DESC textDesc = getTextDesc();
			gGUIManager().getInputCaretTool()->updateText(this, textDesc);
			gGUIManager().getInputSelectionTool()->updateText(this, textDesc);

			if(selStart > 0)
			{
				UINT32 newCaretPos = selStart - 1;
				gGUIManager().getInputCaretTool()->moveCaretToChar(newCaretPos, CARET_AFTER);
			}
			else
			{
				gGUIManager().getInputCaretTool()->moveCaretToChar(0, CARET_BEFORE);
			}

			scrollTextToCaret();

			if (!internal)
				onValueChanged(mText);
		}

		clearSelection();
	}

	String GUIInputBox::getSelectedText()
	{
		UINT32 selStart = gGUIManager().getInputSelectionTool()->getSelectionStart();
		UINT32 selEnd = gGUIManager().getInputSelectionTool()->getSelectionEnd();

		UINT32 byteStart = UTF8::charToByteIndex(mText, selStart);
		UINT32 byteEnd = UTF8::charToByteIndex(mText, selEnd);

		return mText.substr(byteStart, byteEnd - byteStart);
	}

	Vector2I GUIInputBox::getTextOffset() const
	{
		Rect2I textBounds = getCachedContentBounds();
		return Vector2I(textBounds.x, textBounds.y) + mTextOffset;
	}

	Rect2I GUIInputBox::getTextClipRect() const
	{
		Rect2I contentClipRect = getCachedContentClipRect();
		return Rect2I(contentClipRect.x - mTextOffset.x, contentClipRect.y - mTextOffset.y, contentClipRect.width, contentClipRect.height);
	}

	TEXT_SPRITE_DESC GUIInputBox::getTextDesc() const
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

	const HSpriteTexture& GUIInputBox::getActiveTexture() const
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

	Color GUIInputBox::getActiveTextColor() const
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

			contextMenu->addMenuItem("Cut", std::bind(&GUIInputBox::cutText, const_cast<GUIInputBox*>(this)), 0);
			contextMenu->addMenuItem("Copy", std::bind(&GUIInputBox::copyText, const_cast<GUIInputBox*>(this)), 0);
			contextMenu->addMenuItem("Paste", std::bind(&GUIInputBox::pasteText, const_cast<GUIInputBox*>(this)), 0);

			contextMenu->setLocalizedName("Cut", HString("Cut"));
			contextMenu->setLocalizedName("Copy", HString("Copy"));
			contextMenu->setLocalizedName("Paste", HString("Paste"));
		}

		if (!_isDisabled())
			return contextMenu;

		return nullptr;
	}

	void GUIInputBox::cutText()
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		copyText();
		deleteSelectedText();

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	void GUIInputBox::copyText()
	{
		Platform::copyToClipboard(getSelectedText());
	}

	void GUIInputBox::pasteText()
	{
		deleteSelectedText(true);

		String textInClipboard = Platform::copyFromClipboard();
		UINT32 charIdx = gGUIManager().getInputCaretTool()->getCharIdxAtCaretPos();

		bool filterOkay = true;
		if(mFilter != nullptr)
		{
			String newText = mText;

			UINT32 byteIdx = UTF8::charToByteIndex(newText, charIdx);
			newText.insert(newText.begin() + byteIdx, textInClipboard.begin(), textInClipboard.end());

			filterOkay = mFilter(newText);
		}

		if(filterOkay)
		{
			Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			insertString(charIdx, textInClipboard);

			UINT32 numChars = UTF8::count(textInClipboard);
			if(numChars > 0)
				gGUIManager().getInputCaretTool()->moveCaretToChar(charIdx + (numChars - 1), CARET_AFTER);

			scrollTextToCaret();

			Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
			if (origSize != newSize)
				_markLayoutAsDirty();
			else
				_markContentAsDirty();

			if(!onValueChanged.empty())
				onValueChanged(mText);
		}
	}
}
