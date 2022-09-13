//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIButtonBase.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIHelper.h"

namespace bs
{
	GUIButtonBase::GUIButtonBase(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions,
		GUIElementOptions options)
		: GUIElement(styleName, dimensions, options), mContent(content)
	{
		mImageSprite = bs_new<ImageSprite>();
		mTextSprite = bs_new<TextSprite>();

		mImageDesc.animationStartTime = gTime().GetTime();
		mContentAnimationStartTime = mImageDesc.animationStartTime;

		refreshContentSprite();
	}

	GUIButtonBase::~GUIButtonBase()
	{
		bs_delete(mTextSprite);
		bs_delete(mImageSprite);

		if(mContentImageSprite != nullptr)
			bs_delete(mContentImageSprite);
	}

	void GUIButtonBase::SetContent(const GUIContent& content)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;
		mContent = content;
		mContentAnimationStartTime = gTime().GetTime();

		refreshContentSprite();

		Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	void GUIButtonBase::_setOn(bool on)
	{
		if(on)
			_setState((GUIElementState)((INT32)mActiveState | (INT32)GUIElementState::OnFlag));
		else
			_setState((GUIElementState)((INT32)mActiveState & ~(INT32)GUIElementState::OnFlag));
	}

	bool GUIButtonBase::_isOn() const
	{
		return ((INT32)mActiveState & (INT32)GUIElementState::OnFlag) != 0;
	}

	void GUIButtonBase::UpdateRenderElementsInternal()
	{		
		mImageDesc.width = mLayoutData.area.width;
		mImageDesc.height = mLayoutData.area.height;

		const HSpriteTexture& activeTex = getActiveTexture();
		if (SpriteTexture::checkIsLoaded(activeTex))
			mImageDesc.texture = activeTex;
		else
			mImageDesc.texture = nullptr;

		mImageDesc.borderLeft = _getStyle()->border.left;
		mImageDesc.borderRight = _getStyle()->border.right;
		mImageDesc.borderTop = _getStyle()->border.top;
		mImageDesc.borderBottom = _getStyle()->border.bottom;
		mImageDesc.color = getTint();

		mImageSprite->Update(mImageDesc, (UINT64)_getParentWidget());
		mTextSprite->Update(getTextDesc(), (UINT64)_getParentWidget());

		if(mContentImageSprite != nullptr)
		{
			Rect2I contentBounds = getCachedContentBounds();

			HSpriteTexture image = mContent.GetImage(mActiveState);
			UINT32 contentWidth = image->GetWidth();
			UINT32 contentHeight = image->GetHeight();

			UINT32 contentMaxWidth = std::min((UINT32)contentBounds.width, contentWidth);
			UINT32 contentMaxHeight = std::min((UINT32)contentBounds.height, contentHeight);

			float horzRatio = contentMaxWidth / (float)contentWidth;
			float vertRatio = contentMaxHeight / (float)contentHeight;

			if (horzRatio < vertRatio)
			{
				contentWidth = Math::RoundToInt(contentWidth * horzRatio);
				contentHeight = Math::RoundToInt(contentHeight * horzRatio);
			}
			else
			{
				contentWidth = Math::RoundToInt(contentWidth * vertRatio);
				contentHeight = Math::RoundToInt(contentHeight * vertRatio);
			}

			IMAGE_SPRITE_DESC contentImgDesc;
			contentImgDesc.texture = image;
			contentImgDesc.width = contentWidth;
			contentImgDesc.height = contentHeight;
			contentImgDesc.color = getTint();
			contentImgDesc.animationStartTime = mContentAnimationStartTime;

			mContentImageSprite->Update(contentImgDesc, (UINT64)_getParentWidget());
		}

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mImageSprite, 1), T::SpriteInfo(mTextSprite), T::SpriteInfo(mContentImageSprite) }, mRenderElements);
		}

		GUIElement::updateRenderElementsInternal();
	}

	Vector2I GUIButtonBase::_getOptimalSize() const
	{
		UINT32 imageWidth = 0;
		UINT32 imageHeight = 0;

		const HSpriteTexture& activeTex = getActiveTexture();
		if(SpriteTexture::checkIsLoaded(activeTex))
		{
			imageWidth = activeTex->GetWidth();
			imageHeight = activeTex->GetHeight();
		}

		Vector2I contentSize = GUIHelper::calcOptimalContentsSize(mContent, *_getStyle(), _getDimensions(), mActiveState);
		UINT32 contentWidth = std::max(imageWidth, (UINT32)contentSize.x);
		UINT32 contentHeight = std::max(imageHeight, (UINT32)contentSize.y);

		return Vector2I(contentWidth, contentHeight);
	}

	UINT32 GUIButtonBase::_getRenderElementDepthRange() const
	{
		return 2;
	}

	void GUIButtonBase::_fillBuffer(
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

		UINT32 textSpriteIdx = mImageSprite->GetNumRenderElements();
		UINT32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->GetNumRenderElements();

		if(renderElementIdx < textSpriteIdx)
		{
			Vector2I imageOffset = Vector2I(mLayoutData.area.x, mLayoutData.area.y) + offset;

			mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, imageOffset, mLayoutData.GetLocalClipRect());

			return;
		}

		Rect2I contentBounds = getCachedContentBounds();
		Rect2I contentClipRect = getCachedContentClipRect();
		Rect2I textBounds = mTextSprite->GetBounds(Vector2I(), Rect2I());

		Vector2I textOffset;
		Rect2I textClipRect;

		Vector2I contentOffset;
		Rect2I imageClipRect;
		if(mContentImageSprite != nullptr)
		{
			Rect2I imageBounds = mContentImageSprite->GetBounds(Vector2I(), Rect2I());
			INT32 imageXOffset = 0;
			INT32 textImageSpacing = 0;
			
			if (textBounds.width == 0)
			{
				UINT32 freeWidth = (UINT32)std::max(0, (INT32)contentBounds.width - (INT32)textBounds.width - (INT32)imageBounds.width);
				imageXOffset = (INT32)(freeWidth / 2);
			}
			else
				textImageSpacing = GUIContent::IMAGE_TEXT_SPACING;

			if(_getStyle()->imagePosition == GUIImagePosition::Right)
			{
				INT32 imageReservedWidth = std::max(0, (INT32)contentBounds.width - (INT32)textBounds.width);

				textOffset = Vector2I(contentBounds.x, contentBounds.y);
				textClipRect = contentClipRect;
				textClipRect.width = std::min(contentBounds.width - imageReservedWidth, textClipRect.width);

				contentOffset = Vector2I(contentBounds.x + textBounds.width + imageXOffset + textImageSpacing, contentBounds.y) + offset;
				imageClipRect = contentClipRect;
				imageClipRect.x -= textBounds.width + imageXOffset;
			}
			else
			{
				INT32 imageReservedWidth = imageBounds.width + imageXOffset;

				contentOffset = Vector2I(contentBounds.x + imageXOffset, contentBounds.y) + offset;
				imageClipRect = contentClipRect;
				imageClipRect.x -= imageXOffset;
				imageClipRect.width = std::min(imageReservedWidth, (INT32)imageClipRect.width);

				textOffset = Vector2I(contentBounds.x + imageReservedWidth + textImageSpacing, contentBounds.y);
				textClipRect = contentClipRect;
				textClipRect.x -= imageReservedWidth;
			}

			INT32 imageYOffset = (contentBounds.height - imageBounds.height) / 2;
			imageClipRect.y -= imageYOffset;
			contentOffset.y += imageYOffset;
		}
		else
		{
			textOffset = Vector2I(contentBounds.x, contentBounds.y) + offset;
			textClipRect = contentClipRect;
		}

		if(renderElementIdx >= contentImgSpriteIdx)
		{
			mContentImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, contentImgSpriteIdx - renderElementIdx, contentOffset, imageClipRect);
		}
		else
		{
			mTextSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, textSpriteIdx - renderElementIdx, textOffset, textClipRect);
		}
	}

	bool GUIButtonBase::_mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.GetType() == GUIMouseEventType::MouseOver)
		{
			if (!_isDisabled())
			{
				if(mHasFocus)
					_setState(_isOn() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
				else
					_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);

				onHover();
			}

			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}
		else if(ev.GetType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				if(mHasFocus)
					_setState(_isOn() ? GUIElementState::FocusedOn : GUIElementState::Focused);
				else
					_setState(_isOn() ? GUIElementState::NormalOn : GUIElementState::Normal);

				onOut();
			}

			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}
		else if(ev.GetType() == GUIMouseEventType::MouseDown)
		{
			if (!_isDisabled())
				_setState(_isOn() ? GUIElementState::ActiveOn : GUIElementState::Active);

			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}
		else if(ev.GetType() == GUIMouseEventType::MouseUp)
		{
			if (!_isDisabled())
			{
				if(mHasFocus)
					_setState(_isOn() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
				else
					_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);

				onClick();
			}

			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}
		else if (ev.GetType() == GUIMouseEventType::MouseDoubleClick)
		{
			if (!_isDisabled())
				onDoubleClick();

			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}

		return false;
	}

	bool GUIButtonBase::_commandEvent(const GUICommandEvent& ev)
	{
		const bool baseReturnValue = GUIElement::_commandEvent(ev);

		GUIElementState state = (GUIElementState)((UINT32)mActiveState & (UINT32)GUIElementState::TypeMask);
		if(ev.GetType() == GUICommandEventType::FocusGained)
		{
			mHasFocus = true;

			if(!_isDisabled())
			{
				if(state == GUIElementState::Normal)
					_setState(_isOn() ? GUIElementState::FocusedOn : GUIElementState::Focused);
				else if(state == GUIElementState::Hover)
					_setState(_isOn() ? GUIElementState::FocusedHoverOn : GUIElementState::FocusedHover);
			}

			return true;
		}
		else if(ev.GetType() == GUICommandEventType::FocusLost)
		{
			mHasFocus = false;

			if (state == GUIElementState::Focused)
				_setState(_isOn() ? GUIElementState::NormalOn : GUIElementState::Normal);
			else if (state == GUIElementState::FocusedHover)
				_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);

			return true;
		}

		return baseReturnValue;
	}

	String GUIButtonBase::_getTooltip() const
	{
		return mContent.tooltip;
	}

	void GUIButtonBase::RefreshContentSprite()
	{
		HSpriteTexture contentTex = mContent.GetImage(mActiveState);
		if (SpriteTexture::checkIsLoaded(contentTex))
		{
			if (mContentImageSprite == nullptr)
				mContentImageSprite = bs_new<ImageSprite>();
		}
		else
		{
			if (mContentImageSprite != nullptr)
			{
				bs_delete(mContentImageSprite);
				mContentImageSprite = nullptr;
			}
		}
	}

	TEXT_SPRITE_DESC GUIButtonBase::GetTextDesc() const
	{
		TEXT_SPRITE_DESC textDesc;
		textDesc.text = mContent.text;
		textDesc.font = _getStyle()->font;
		textDesc.fontSize = _getStyle()->fontSize;
		textDesc.color = getTint() * getActiveTextColor();

		Rect2I textBounds = getCachedContentBounds();

		textDesc.width = textBounds.width;
		textDesc.height = textBounds.height;
		textDesc.horzAlign = _getStyle()->textHorzAlign;
		textDesc.vertAlign = _getStyle()->textVertAlign;

		return textDesc;
	}

	void GUIButtonBase::StyleUpdated()
	{
		mImageDesc.animationStartTime = gTime().GetTime();
	}

	void GUIButtonBase::_setState(GUIElementState state)
	{
		Vector2I origSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

		if(mActiveState != state)
			mImageDesc.animationStartTime = gTime().GetTime();

		mActiveState = state;
		refreshContentSprite();
		Vector2I newSize = mDimensions.CalculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	const HSpriteTexture& GUIButtonBase::GetActiveTexture() const
	{
		switch(mActiveState)
		{
		case GUIElementState::Normal:
			return _getStyle()->normal.texture;
		case GUIElementState::Hover:
			return _getStyle()->hover.texture;
		case GUIElementState::Active:
			return _getStyle()->active.texture;
		case GUIElementState::Focused:
			return _getStyle()->focused.texture;
		case GUIElementState::FocusedHover:
			return _getStyle()->focusedHover.texture;
		case GUIElementState::NormalOn:
			return _getStyle()->normalOn.texture;
		case GUIElementState::HoverOn:
			return _getStyle()->hoverOn.texture;
		case GUIElementState::ActiveOn:
			return _getStyle()->activeOn.texture;
		case GUIElementState::FocusedOn:
			return _getStyle()->focusedOn.texture;
		case GUIElementState::FocusedHoverOn:
			return _getStyle()->focusedHoverOn.texture;
		default:
			break;
		}

		return _getStyle()->normal.texture;
	}

	Color GUIButtonBase::GetActiveTextColor() const
	{
		switch (mActiveState)
		{
		case GUIElementState::Normal:
			return _getStyle()->normal.textColor;
		case GUIElementState::Hover:
			return _getStyle()->hover.textColor;
		case GUIElementState::Active:
			return _getStyle()->active.textColor;
		case GUIElementState::Focused:
			return _getStyle()->focused.textColor;
		case GUIElementState::FocusedHover:
			return _getStyle()->focusedHover.textColor;
		case GUIElementState::NormalOn:
			return _getStyle()->normalOn.textColor;
		case GUIElementState::HoverOn:
			return _getStyle()->hoverOn.textColor;
		case GUIElementState::ActiveOn:
			return _getStyle()->activeOn.textColor;
		case GUIElementState::FocusedOn:
			return _getStyle()->focusedOn.textColor;
		case GUIElementState::FocusedHoverOn:
			return _getStyle()->focusedHoverOn.textColor;
		default:
			break;
		}

		return _getStyle()->normal.textColor;
	}
}
