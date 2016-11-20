//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIButtonBase.h"
#include "BsImageSprite.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsTextSprite.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"
#include "BsGUIHelper.h"

namespace bs
{
	GUIButtonBase::GUIButtonBase(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions), mContentImageSprite(nullptr), mActiveState(GUIElementState::Normal)
		, mContent(content)
	{
		mImageSprite = bs_new<ImageSprite>();
		mTextSprite = bs_new<TextSprite>();

		refreshContentSprite();
	}

	GUIButtonBase::~GUIButtonBase()
	{
		bs_delete(mTextSprite);
		bs_delete(mImageSprite);

		if(mContentImageSprite != nullptr)
			bs_delete(mContentImageSprite);
	}

	void GUIButtonBase::setContent(const GUIContent& content)
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		mContent = content;

		refreshContentSprite();

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	void GUIButtonBase::_setOn(bool on) 
	{ 
		if(on)
			_setState((GUIElementState)((INT32)mActiveState | 0x10)); 
		else
			_setState((GUIElementState)((INT32)mActiveState & (~0x10))); 
	}

	bool GUIButtonBase::_isOn() const
	{
		return ((INT32)mActiveState & 0x10) != 0;
	}

	UINT32 GUIButtonBase::_getNumRenderElements() const
	{
		UINT32 numElements = mImageSprite->getNumRenderElements();
		numElements += mTextSprite->getNumRenderElements();

		if(mContentImageSprite != nullptr)
			numElements += mContentImageSprite->getNumRenderElements();

		return numElements;
	}

	const SpriteMaterialInfo& GUIButtonBase::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		UINT32 textSpriteIdx = mImageSprite->getNumRenderElements();
		UINT32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->getNumRenderElements();

		if (renderElementIdx >= contentImgSpriteIdx)
		{
			*material = mContentImageSprite->getMaterial(contentImgSpriteIdx - renderElementIdx);
			return mContentImageSprite->getMaterialInfo(contentImgSpriteIdx - renderElementIdx);
		}
		else if (renderElementIdx >= textSpriteIdx)
		{
			*material = mTextSprite->getMaterial(textSpriteIdx - renderElementIdx);
			return mTextSprite->getMaterialInfo(textSpriteIdx - renderElementIdx);
		}
		else
		{
			*material = mImageSprite->getMaterial(renderElementIdx);
			return mImageSprite->getMaterialInfo(renderElementIdx);
		}
	}

	void GUIButtonBase::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		UINT32 textSpriteIdx = mImageSprite->getNumRenderElements();
		UINT32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->getNumRenderElements();

		UINT32 numQuads = 0;
		if(renderElementIdx >= contentImgSpriteIdx)
			numQuads = mContentImageSprite->getNumQuads(contentImgSpriteIdx - renderElementIdx);
		else if(renderElementIdx >= textSpriteIdx)
			numQuads = mTextSprite->getNumQuads(textSpriteIdx - renderElementIdx);
		else
			numQuads = mImageSprite->getNumQuads(renderElementIdx);

		numVertices = numQuads * 4;
		numIndices = numQuads * 6;
		type = GUIMeshType::Triangle;
	}

	void GUIButtonBase::updateRenderElementsInternal()
	{		
		mImageDesc.width = mLayoutData.area.width;
		mImageDesc.height = mLayoutData.area.height;

		const HSpriteTexture& activeTex = getActiveTexture();
		if (SpriteTexture::checkIsLoaded(activeTex))
			mImageDesc.texture = activeTex.getInternalPtr();
		else
			mImageDesc.texture = nullptr;

		mImageDesc.borderLeft = _getStyle()->border.left;
		mImageDesc.borderRight = _getStyle()->border.right;
		mImageDesc.borderTop = _getStyle()->border.top;
		mImageDesc.borderBottom = _getStyle()->border.bottom;
		mImageDesc.color = getTint();

		mImageSprite->update(mImageDesc, (UINT64)_getParentWidget());

		mTextSprite->update(getTextDesc(), (UINT64)_getParentWidget());

		if(mContentImageSprite != nullptr)
		{
			Rect2I contentBounds = getCachedContentBounds();

			HSpriteTexture image = mContent.getImage(mActiveState);
			UINT32 contentWidth = image->getWidth();
			UINT32 contentHeight = image->getHeight();

			UINT32 contentMaxWidth = std::min((UINT32)contentBounds.width, contentWidth);
			UINT32 contentMaxHeight = std::min((UINT32)contentBounds.height, contentHeight);

			float horzRatio = contentMaxWidth / (float)contentWidth;
			float vertRatio = contentMaxHeight / (float)contentHeight;

			if (horzRatio < vertRatio)
			{
				contentWidth = Math::roundToInt(contentWidth * horzRatio);
				contentHeight = Math::roundToInt(contentHeight * horzRatio);
			}
			else
			{
				contentWidth = Math::roundToInt(contentWidth * vertRatio);
				contentHeight = Math::roundToInt(contentHeight * vertRatio);
			}

			IMAGE_SPRITE_DESC contentImgDesc;
			contentImgDesc.texture = image.getInternalPtr();
			contentImgDesc.width = contentWidth;
			contentImgDesc.height = contentHeight;
			contentImgDesc.color = getTint();

			mContentImageSprite->update(contentImgDesc, (UINT64)_getParentWidget());
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
			imageWidth = activeTex->getWidth();
			imageHeight = activeTex->getHeight();
		}

		Vector2I contentSize = GUIHelper::calcOptimalContentsSize(mContent, *_getStyle(), _getDimensions(), mActiveState);
		UINT32 contentWidth = std::max(imageWidth, (UINT32)contentSize.x);
		UINT32 contentHeight = std::max(imageHeight, (UINT32)contentSize.y);

		return Vector2I(contentWidth, contentHeight);
	}

	UINT32 GUIButtonBase::_getRenderElementDepth(UINT32 renderElementIdx) const
	{
		UINT32 textSpriteIdx = mImageSprite->getNumRenderElements();
		UINT32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->getNumRenderElements();

		if(renderElementIdx >= contentImgSpriteIdx)
			return _getDepth();
		else if(renderElementIdx >= textSpriteIdx)
			return _getDepth();
		else
			return _getDepth() + 1;
	}

	UINT32 GUIButtonBase::_getRenderElementDepthRange() const
	{
		return 2;
	}

	void GUIButtonBase::_fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		UINT32 textSpriteIdx = mImageSprite->getNumRenderElements();
		UINT32 contentImgSpriteIdx = textSpriteIdx + mTextSprite->getNumRenderElements();

		if(renderElementIdx < textSpriteIdx)
		{
			Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);

			mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, offset, mLayoutData.getLocalClipRect());

			return;
		}

		Rect2I contentBounds = getCachedContentBounds();
		Rect2I contentClipRect = getCachedContentClipRect();
		Rect2I textBounds = mTextSprite->getBounds(Vector2I(), Rect2I());

		Vector2I textOffset;
		Rect2I textClipRect;

		Vector2I imageOffset;
		Rect2I imageClipRect;
		if(mContentImageSprite != nullptr)
		{
			Rect2I imageBounds = mContentImageSprite->getBounds(Vector2I(), Rect2I());
			INT32 imageXOffset = 0;
			INT32 textImageSpacing = 0;
			
			if (textBounds.width == 0)
			{
				UINT32 freeWidth = (UINT32)std::max(0, contentBounds.width - textBounds.width - imageBounds.width);
				imageXOffset = (INT32)(freeWidth / 2);
			}
			else
				textImageSpacing = GUIContent::IMAGE_TEXT_SPACING;

			if(_getStyle()->imagePosition == GUIImagePosition::Right)
			{
				INT32 imageReservedWidth = std::max(0, contentBounds.width - textBounds.width);

				textOffset = Vector2I(contentBounds.x, contentBounds.y);
				textClipRect = contentClipRect;
				textClipRect.width = std::min(contentBounds.width - imageReservedWidth, textClipRect.width);

				imageOffset = Vector2I(contentBounds.x + textBounds.width + imageXOffset + textImageSpacing, contentBounds.y);
				imageClipRect = contentClipRect;
				imageClipRect.x -= textBounds.width + imageXOffset;
			}
			else
			{
				INT32 imageReservedWidth = imageBounds.width + imageXOffset;

				imageOffset = Vector2I(contentBounds.x + imageXOffset, contentBounds.y);
				imageClipRect = contentClipRect;
				imageClipRect.x -= imageXOffset;
				imageClipRect.width = std::min(imageReservedWidth, imageClipRect.width);

				textOffset = Vector2I(contentBounds.x + imageReservedWidth + textImageSpacing, contentBounds.y);
				textClipRect = contentClipRect;
				textClipRect.x -= imageReservedWidth;
			}

			INT32 imageYOffset = (contentBounds.height - imageBounds.height) / 2;
			imageClipRect.y -= imageYOffset;
			imageOffset.y += imageYOffset;
		}
		else
		{
			textOffset = Vector2I(contentBounds.x, contentBounds.y);
			textClipRect = contentClipRect;
		}

		if(renderElementIdx >= contentImgSpriteIdx)
		{
			mContentImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, contentImgSpriteIdx - renderElementIdx, imageOffset, imageClipRect);
		}
		else
		{
			mTextSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, textSpriteIdx - renderElementIdx, textOffset, textClipRect);
		}
	}

	bool GUIButtonBase::_mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.getType() == GUIMouseEventType::MouseOver)
		{
			if (!_isDisabled())
			{
				_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);
				onHover();
			}

			return mBlockPointerEvents;
		}
		else if(ev.getType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				_setState(_isOn() ? GUIElementState::NormalOn : GUIElementState::Normal);
				onOut();
			}

			return mBlockPointerEvents;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDown)
		{
			if (!_isDisabled())
				_setState(_isOn() ? GUIElementState::ActiveOn : GUIElementState::Active);

			return mBlockPointerEvents;
		}
		else if(ev.getType() == GUIMouseEventType::MouseUp)
		{
			if (!_isDisabled())
			{
				_setState(_isOn() ? GUIElementState::HoverOn : GUIElementState::Hover);
				onClick();
			}

			return mBlockPointerEvents;
		}
		else if (ev.getType() == GUIMouseEventType::MouseDoubleClick)
		{
			if (!_isDisabled())
				onDoubleClick();

			return mBlockPointerEvents;
		}

		return false;
	}

	WString GUIButtonBase::_getTooltip() const
	{
		return mContent.getTooltip();
	}

	void GUIButtonBase::refreshContentSprite()
	{
		HSpriteTexture contentTex = mContent.getImage(mActiveState);
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

	TEXT_SPRITE_DESC GUIButtonBase::getTextDesc() const
	{
		TEXT_SPRITE_DESC textDesc;
		textDesc.text = mContent.getText();
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

	void GUIButtonBase::_setState(GUIElementState state)
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		mActiveState = state;
		refreshContentSprite();
		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	const HSpriteTexture& GUIButtonBase::getActiveTexture() const
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
		case GUIElementState::NormalOn:
			return _getStyle()->normalOn.texture;
		case GUIElementState::HoverOn:
			return _getStyle()->hoverOn.texture;
		case GUIElementState::ActiveOn:
			return _getStyle()->activeOn.texture;
		case GUIElementState::FocusedOn:
			return _getStyle()->focusedOn.texture;
		}

		return _getStyle()->normal.texture;
	}

	Color GUIButtonBase::getActiveTextColor() const
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
		case GUIElementState::NormalOn:
			return _getStyle()->normalOn.textColor;
		case GUIElementState::HoverOn:
			return _getStyle()->hoverOn.textColor;
		case GUIElementState::ActiveOn:
			return _getStyle()->activeOn.textColor;
		case GUIElementState::FocusedOn:
			return _getStyle()->focusedOn.textColor;
		}

		return _getStyle()->normal.textColor;
	}
}