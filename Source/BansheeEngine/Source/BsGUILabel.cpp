//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUILabel.h"
#include "BsGUIElementStyle.h"
#include "BsTextSprite.h"
#include "BsGUIDimensions.h"
#include "BsGUIHelper.h"

namespace BansheeEngine
{
	GUILabel::GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mContent(content)
	{
		mTextSprite = bs_new<TextSprite>();
	}

	GUILabel::~GUILabel()
	{
		bs_delete(mTextSprite);
	}

	UINT32 GUILabel::_getNumRenderElements() const
	{
		return mTextSprite->getNumRenderElements();
	}

	const SpriteMaterialInfo& GUILabel::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		*material = mTextSprite->getMaterial(renderElementIdx);
		return mTextSprite->getMaterialInfo(renderElementIdx);
	}

	void GUILabel::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		UINT32 numQuads = mTextSprite->getNumQuads(renderElementIdx);

		numVertices = numQuads * 4;
		numIndices = numQuads * 6;
		type = GUIMeshType::Triangle;
	}

	void GUILabel::updateRenderElementsInternal()
	{		
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

		mTextSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride,
			indexStride, renderElementIdx, offset, mLayoutData.getLocalClipRect());
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