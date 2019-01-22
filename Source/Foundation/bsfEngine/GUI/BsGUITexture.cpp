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
		mUsingStyleTexture = false;
		mDesc.animationStartTime = gTime().getTime();

		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	UINT32 GUITexture::_getNumRenderElements() const
	{
		return mImageSprite->getNumRenderElements();
	}

	const SpriteMaterialInfo& GUITexture::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		*material = mImageSprite->getMaterial(renderElementIdx);
		return mImageSprite->getMaterialInfo(renderElementIdx);
	}

	void GUITexture::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		UINT32 numQuads = mImageSprite->getNumQuads(renderElementIdx);
		numVertices = numQuads * 4;
		numIndices = numQuads * 6;
		type = GUIMeshType::Triangle;
	}

	void GUITexture::updateRenderElementsInternal()
	{		
		mDesc.width = mLayoutData.area.width;
		mDesc.height = mLayoutData.area.height;

		mDesc.borderLeft = _getStyle()->border.left;
		mDesc.borderRight = _getStyle()->border.right;
		mDesc.borderTop = _getStyle()->border.top;
		mDesc.borderBottom = _getStyle()->border.bottom;
		mDesc.transparent = mTransparent;
		mDesc.color = getTint();

		Vector2I textureSize;
		if (SpriteTexture::checkIsLoaded(mActiveTexture))
		{
			mDesc.texture = mActiveTexture;
			textureSize.x = mDesc.texture->getWidth();
			textureSize.y = mDesc.texture->getHeight();
		}

		Vector2I destSize(mLayoutData.area.width, mLayoutData.area.height);
		mDesc.uvScale = ImageSprite::getTextureUVScale(textureSize, destSize, mScaleMode);
		
		mImageSprite->update(mDesc, (UINT64)_getParentWidget());
		
		GUIElement::updateRenderElementsInternal();
	}

	void GUITexture::styleUpdated()
	{
		if (mUsingStyleTexture)
		{
			mActiveTexture = _getStyle()->normal.texture;
			mDesc.animationStartTime = gTime().getTime();
		}
	}

	Vector2I GUITexture::_getOptimalSize() const
	{
		// TODO - Accounting for style dimensions might be redundant here, I'm pretty sure we do that on higher level anyway
		Vector2I optimalSize;

		if(_getDimensions().fixedWidth())
			optimalSize.x = _getDimensions().minWidth;
		else
		{
			if (SpriteTexture::checkIsLoaded(mActiveTexture))
				optimalSize.x = mActiveTexture->getWidth();
			else
				optimalSize.x = _getDimensions().maxWidth;
		}

		if(_getDimensions().fixedHeight())
			optimalSize.y = _getDimensions().minHeight;
		else
		{
			if (SpriteTexture::checkIsLoaded(mActiveTexture))
				optimalSize.y = mActiveTexture->getHeight();
			else
				optimalSize.y = _getDimensions().maxHeight;
		}

		return optimalSize;
	}

	void GUITexture::_fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, renderElementIdx, offset, mLayoutData.getLocalClipRect());
	}
}