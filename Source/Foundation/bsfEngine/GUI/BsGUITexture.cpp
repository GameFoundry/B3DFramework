//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITexture.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"

namespace bs
{
	const String& GUITexture::getGUITypeName()
	{
		static String name = "Texture";
		return name;
	}

	GUITexture::GUITexture(const String& styleName, const HSpriteTexture& texture,
		TextureScaleMode scale, bool transparent, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mScaleMode(scale), mTransparent(transparent), mUsingStyleTexture(false)
	{
		mImageSprite = bs_new<ImageSprite>();
		mDesc.animationStartTime = gTime().getTime();

		if(texture != nullptr)
		{
			mActiveTexture = texture;
			mUsingStyleTexture = false;
		}
		else
		{
			mActiveTexture = _getStyle()->normal.texture;
			mUsingStyleTexture = true;
		}

		bool isTexLoaded = SpriteTexture::checkIsLoaded(mActiveTexture);
		mActiveTextureWidth = isTexLoaded ? mActiveTexture->getFrameWidth() : 0;
		mActiveTextureHeight = isTexLoaded ? mActiveTexture->getFrameHeight() : 0;
	}

	GUITexture::~GUITexture()
	{
		bs_delete(mImageSprite);
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent,
		const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, scale, transparent, GUIDimensions::create(options));
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent,
		const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, scale, transparent, GUIDimensions::create());
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture, TextureScaleMode scale,
		const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, scale, true, GUIDimensions::create(options));
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture, TextureScaleMode scale,
		const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, scale, true, GUIDimensions::create());
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture,
		const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, TextureScaleMode::StretchToFit, true, GUIDimensions::create(options));
	}

	GUITexture* GUITexture::create(const HSpriteTexture& texture, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			texture, TextureScaleMode::StretchToFit, true, GUIDimensions::create());
	}

	GUITexture* GUITexture::create(TextureScaleMode scale, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			HSpriteTexture(), scale, true, GUIDimensions::create(options));
	}

	GUITexture* GUITexture::create(TextureScaleMode scale, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			HSpriteTexture(), scale, true, GUIDimensions::create());
	}

	GUITexture* GUITexture::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::create(options));
	}

	GUITexture* GUITexture::create(const String& styleName)
	{
		return new (bs_alloc<GUITexture>()) GUITexture(getStyleName<GUITexture>(styleName),
			HSpriteTexture(), TextureScaleMode::StretchToFit, true, GUIDimensions::create());
	}

	void GUITexture::setTexture(const HSpriteTexture& texture)
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		mActiveTexture = texture;

		bool isTexLoaded = SpriteTexture::checkIsLoaded(mActiveTexture);
		mActiveTextureWidth = isTexLoaded ? mActiveTexture->getFrameWidth() : 0;
		mActiveTextureHeight = isTexLoaded ? mActiveTexture->getFrameHeight() : 0;

		mUsingStyleTexture = false;
		mDesc.animationStartTime = gTime().getTime();

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	void GUITexture::updateRenderElementsInternal()
	{
		Vector2I textureSize;
		if (SpriteTexture::checkIsLoaded(mActiveTexture))
		{
			mDesc.texture = mActiveTexture;
			textureSize.x = mDesc.texture->getFrameWidth();
			textureSize.y = mDesc.texture->getFrameHeight();
		}
		Vector2I destSize(mLayoutData.area.width, mLayoutData.area.height);

		// ScaleToFit is the only scaling mode that might result in the GUITexture area not being completely covered by
		// the sprite. We need the actual sprite size and offsets to center it.
		if(mScaleMode == TextureScaleMode::ScaleToFit)
		{
			if(destSize.x != 0 && destSize.y != 0)
			{
				float aspectX = textureSize.x / (float)destSize.x;
				float aspectY = textureSize.y / (float)destSize.y;

				if (aspectY > aspectX)
				{
					destSize.x = Math::roundToPosInt(textureSize.x / aspectY);
					destSize.y = Math::roundToPosInt(textureSize.y / aspectY);
				}
				else
				{
					destSize.x = Math::roundToPosInt(textureSize.x / aspectX);
					destSize.y = Math::roundToPosInt(textureSize.y / aspectX);
				}
			}

			mImageSpriteOffset = Vector2I(
				((INT32)mLayoutData.area.width - destSize.x) / 2,
				((INT32)mLayoutData.area.height - destSize.y) / 2
			);
		}
		else
			mImageSpriteOffset = Vector2I();

		mDesc.width = (UINT32)destSize.x;
		mDesc.height = (UINT32)destSize.y;

		mDesc.borderLeft = _getStyle()->border.left;
		mDesc.borderRight = _getStyle()->border.right;
		mDesc.borderTop = _getStyle()->border.top;
		mDesc.borderBottom = _getStyle()->border.bottom;
		mDesc.transparent = mTransparent;
		mDesc.color = getTint();

		if(mScaleMode != TextureScaleMode::ScaleToFit)
			mDesc.uvScale = ImageSprite::getTextureUVScale(textureSize, destSize, mScaleMode);
		else
			mDesc.uvScale = Vector2::ONE;
		
		mImageSprite->update(mDesc, (UINT64)_getParentWidget());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
		}
		
		GUIElement::updateRenderElementsInternal();
	}

	void GUITexture::styleUpdated()
	{
		if (mUsingStyleTexture)
		{
			mActiveTexture = _getStyle()->normal.texture;
			mDesc.animationStartTime = gTime().getTime();

			bool isTexLoaded = SpriteTexture::checkIsLoaded(mActiveTexture);
			mActiveTextureWidth = isTexLoaded ? mActiveTexture->getFrameWidth() : 0;
			mActiveTextureHeight = isTexLoaded ? mActiveTexture->getFrameHeight() : 0;
		}
	}

	Vector2I GUITexture::_getOptimalSize() const
	{
		// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
		Vector2I optimalSize;

		// Note: We use cached texture size here. This is because we use this method for checking we a layout update is
		// needed (size change is detected). Sprite texture could change without us knowing and by storing the size we can
		// safely detect this. (In short, don't do mActiveTexture->getFrameWidth/Height() here)
		
		if(_getDimensions().fixedWidth())
			optimalSize.x = _getDimensions().minWidth;
		else
		{
			if (SpriteTexture::checkIsLoaded(mActiveTexture))
				optimalSize.x = mActiveTextureWidth;
			else
				optimalSize.x = _getDimensions().maxWidth;
		}

		if(_getDimensions().fixedHeight())
			optimalSize.y = _getDimensions().minHeight;
		else
		{
			if (SpriteTexture::checkIsLoaded(mActiveTexture))
				optimalSize.y = mActiveTextureHeight;
			else
				optimalSize.y = _getDimensions().maxHeight;
		}

		return optimalSize;
	}

	void GUITexture::_fillBuffer(
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

		Vector2I layoutOffset = Vector2I(mLayoutData.area.x, mLayoutData.area.y) + mImageSpriteOffset + offset;
		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.getLocalClipRect());
	}
}
