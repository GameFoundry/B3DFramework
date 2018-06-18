//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIElementStyle.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIHelper.h"

namespace bs
{
	GUILabel::GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mContent(content), mImageSprite(nullptr)
	{
		mTextSprite = bs_new<TextSprite>();
	}

	GUILabel::~GUILabel()
	{
		bs_delete(mTextSprite);

		if (mImageSprite != nullptr)
			bs_delete(mImageSprite);
	}

	UINT32 GUILabel::_getNumRenderElements() const
	{
		UINT32 numElements = mTextSprite->getNumRenderElements();

		if(mImageSprite != nullptr)
			numElements += mImageSprite->getNumRenderElements();

		return numElements;
	}

	const SpriteMaterialInfo& GUILabel::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		UINT32 imageSpriteIdx = mTextSprite->getNumRenderElements();

		if(renderElementIdx >= imageSpriteIdx)
		{
			*material = mImageSprite->getMaterial(imageSpriteIdx - renderElementIdx);
			return mImageSprite->getMaterialInfo(imageSpriteIdx - renderElementIdx);
		}
		else
		{
			*material = mTextSprite->getMaterial(renderElementIdx);
			return mTextSprite->getMaterialInfo(renderElementIdx);
		}
	}

	void GUILabel::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		UINT32 imageSpriteIdx = mTextSprite->getNumRenderElements();

		UINT32 numQuads;
		if (renderElementIdx >= imageSpriteIdx)
			numQuads = mImageSprite->getNumQuads(imageSpriteIdx - renderElementIdx);
		else
			numQuads = mTextSprite->getNumQuads(renderElementIdx);

		numVertices = numQuads * 4;
		numIndices = numQuads * 6;
		type = GUIMeshType::Triangle;
	}

	UINT32 GUILabel::_getRenderElementDepth(UINT32 renderElementIdx) const
	{
		UINT32 imageSpriteIdx = mTextSprite->getNumRenderElements();

		if (renderElementIdx >= imageSpriteIdx)
			return _getDepth() + 1;
		else
			return _getDepth();
	}

	UINT32 GUILabel::_getRenderElementDepthRange() const
	{
		return 2;
	}

	void GUILabel::updateRenderElementsInternal()
	{		
		const HSpriteTexture& activeTex = _getStyle()->normal.texture;
		if (SpriteTexture::checkIsLoaded(activeTex))
		{
			mImageDesc.texture = activeTex.getInternalPtr();

			if (mImageSprite == nullptr)
				mImageSprite = bs_new<ImageSprite>();
		}
		else
		{
			mImageDesc.texture = nullptr;

			if (mImageSprite != nullptr)
			{
				bs_delete(mImageSprite);
				mImageSprite = nullptr;
			}
		}

		if (mImageSprite != nullptr)
		{
			mImageDesc.width = mLayoutData.area.width;
			mImageDesc.height = mLayoutData.area.height;

			mImageDesc.borderLeft = _getStyle()->border.left;
			mImageDesc.borderRight = _getStyle()->border.right;
			mImageDesc.borderTop = _getStyle()->border.top;
			mImageDesc.borderBottom = _getStyle()->border.bottom;
			mImageDesc.color = getTint();

			mImageSprite->update(mImageDesc, (UINT64)_getParentWidget());
		}

		mDesc.font = _getStyle()->font;
		mDesc.fontSize = _getStyle()->fontSize;
		mDesc.wordWrap = _getStyle()->wordWrap;
		mDesc.horzAlign = _getStyle()->textHorzAlign;
		mDesc.vertAlign = _getStyle()->textVertAlign;
		mDesc.width = mLayoutData.area.width;
		mDesc.height = mLayoutData.area.height;
		mDesc.text = mContent.getText();
		mDesc.color = getTint() * _getStyle()->normal.textColor;;

		mTextSprite->update(mDesc, (UINT64)_getParentWidget());

		GUIElement::updateRenderElementsInternal();
	}

	Vector2I GUILabel::_getOptimalSize() const
	{
		return GUIHelper::calcOptimalContentsSize(mContent, *_getStyle(), _getDimensions());
	}

	void GUILabel::_fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);
		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);

		UINT32 imageSpriteIdx = mTextSprite->getNumRenderElements();

		if (renderElementIdx < imageSpriteIdx)
		{
			mTextSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride,
				indexStride, renderElementIdx, offset, mLayoutData.getLocalClipRect());

			return;
		}

		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, imageSpriteIdx - renderElementIdx, offset, mLayoutData.getLocalClipRect());
	}

	void GUILabel::setContent(const GUIContent& content)
	{
		Vector2I origSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;
		mContent = content;
		Vector2I newSize = mDimensions.calculateSizeRange(_getOptimalSize()).optimal;

		if (origSize != newSize)
			_markLayoutAsDirty();
		else
			_markContentAsDirty();
	}

	GUILabel* GUILabel::create(const HString& text, const String& styleName)
	{
		return create(GUIContent(text), styleName);
	}

	GUILabel* GUILabel::create(const HString& text, const GUIOptions& options, const String& styleName)
	{
		return create(GUIContent(text), options, styleName);
	}

	GUILabel* GUILabel::create(const GUIContent& content, const String& styleName)
	{
		return new (bs_alloc<GUILabel>()) GUILabel(getStyleName<GUILabel>(styleName), content, GUIDimensions::create());
	}

	GUILabel* GUILabel::create(const GUIContent& content, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUILabel>()) GUILabel(getStyleName<GUILabel>(styleName), content, GUIDimensions::create(options));
	}

	const String& GUILabel::getGUITypeName()
	{
		static String typeName = "Label";
		return typeName;
	}
}