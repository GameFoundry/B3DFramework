//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUICanvas.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsGUIDimensions.h"
#include "BsGUITexture.h"
#include "BsShapeMeshes2D.h"
#include "BsSpriteManager.h"
#include "BsSpriteMaterials.h"
#include "BsException.h"

namespace BansheeEngine
{
	const String& GUICanvas::getGUITypeName()
	{
		static String name = "Canvas";
		return name;
	}

	GUICanvas::GUICanvas(const String& styleName, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mNumRenderElements(0), mForceTriangleBuild(false)
	{

	}

	GUICanvas::~GUICanvas()
	{
		clear();
	}

	GUICanvas* GUICanvas::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUICanvas>()) GUICanvas(getStyleName<GUICanvas>(styleName), GUIDimensions::create(options));
	}

	GUICanvas* GUICanvas::create(const String& styleName)
	{
		return new (bs_alloc<GUICanvas>()) GUICanvas(getStyleName<GUICanvas>(styleName), GUIDimensions::create());
	}

	void GUICanvas::drawLine(const Vector2I& a, const Vector2I& b, float width, const Color& color)
	{
		drawPolyLine({ a, b }, width, color);
	}

	void GUICanvas::drawPolyLine(const Vector<Vector2I>& vertices, float width, const Color& color)
	{
		if(vertices.size() < 2)
		{
			LOGWRN("Invalid number of vertices. Ignoring call.");
			return;
		}

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Line;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.size();
		element.vertexStart = (UINT32)mVertexData.size();
		element.numVertices = (UINT32)vertices.size();
		element.lineWidth = width;

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		SPtr<SpriteMaterialLineInfo> lineInfo = bs_shared_ptr_new<SpriteMaterialLineInfo>();
		lineInfo->width = width;

		for (auto& vertex : vertices)
		{
			Vector2 point = Vector2((float)vertex.x, (float)vertex.y);

			mVertexData.push_back(point);
			lineInfo->points.push_back(point);
		}

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawTexture(const HSpriteTexture& texture, const Rect2I& area, TextureScaleMode scaleMode, 
		const Color& color)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Image;
		element.color = color;
		element.dataId = (UINT32)mImageData.size();
		element.scaleMode = scaleMode;
		element.imageSprite = bs_new<ImageSprite>();

		mImageData.push_back({ texture, area });
		_markContentAsDirty();
	}

	void GUICanvas::drawTriangleStrip(const Vector<Vector2I>& vertices, const Color& color)
	{
		if (vertices.size() < 3)
		{
			LOGWRN("Invalid number of vertices. Ignoring call.");
			return;
		}

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Triangle;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.size();
		element.vertexStart = (UINT32)mVertexData.size();
		element.numVertices = (UINT32)(vertices.size() - 2) * 3;
		element.lineWidth = 0.0f; // Not used

		// Convert strip to list
		for(UINT32 i = 2; i < (UINT32)vertices.size(); i++)
		{
			mVertexData.push_back(Vector2((float)vertices[i - 2].x, (float)vertices[i - 2].y));
			mVertexData.push_back(Vector2((float)vertices[i - 1].x, (float)vertices[i - 1].y));
			mVertexData.push_back(Vector2((float)vertices[i - 0].x, (float)vertices[i - 0].y));
		}

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawTriangleList(const Vector<Vector2I>& vertices, const Color& color)
	{
		if (vertices.size() < 3 || vertices.size() % 3 != 0)
		{
			LOGWRN("Invalid number of vertices. Ignoring call.");
			return;
		}

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Triangle;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.size();
		element.vertexStart = (UINT32)mVertexData.size();
		element.numVertices = (UINT32)vertices.size();
		element.lineWidth = 0.0f; // Not used

		for (auto& vertex : vertices)
			mVertexData.push_back(Vector2((float)vertex.x, (float)vertex.y));

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawText(const WString& text, const Vector2I& position, const HFont& font, UINT32 size, 
		const Color& color)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Text;
		element.color = color;
		element.dataId = (UINT32)mTextData.size();
		element.size = size;
		element.textSprite = bs_new<TextSprite>();

		mTextData.push_back({ text, font, position });
		_markContentAsDirty();
	}

	void GUICanvas::clear()
	{
		for (auto& element : mElements)
		{
			if(element.imageSprite != nullptr)
				bs_delete(element.imageSprite);
		}

		mElements.clear();
		mNumRenderElements = 0;

		mVertexData.clear();
		mImageData.clear();
		mTextData.clear();
		mTriangleElementData.clear();
		mClippedVertices.clear();
		mForceTriangleBuild = false;
	}

	UINT32 GUICanvas::_getNumRenderElements() const
	{
		return mNumRenderElements;
	}

	const SpriteMaterialInfo& GUICanvas::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		static const SpriteMaterialInfo defaultMatInfo;

		const CanvasElement& element = findElement(renderElementIdx);
		switch (element.type)
		{
		case CanvasElementType::Line:
			*material = SpriteManager::instance().getLineMaterial();
			return mTriangleElementData[element.dataId].matInfo;
		case CanvasElementType::Image:
			*material = element.imageSprite->getMaterial(0);
			return element.imageSprite->getMaterialInfo(0);
		case CanvasElementType::Text:
			*material = element.imageSprite->getMaterial(renderElementIdx - element.renderElemStart);
			return element.textSprite->getMaterialInfo(renderElementIdx - element.renderElemStart);
		case CanvasElementType::Triangle:
			*material = SpriteManager::instance().getImageTransparentMaterial();
			return mTriangleElementData[element.dataId].matInfo;
		default:
			*material = nullptr;
			return defaultMatInfo;
		}
	}

	void GUICanvas::_getMeshSize(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices) const
	{
		Vector2 offset((float)mLayoutData.area.x, (float)mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.getLocalClipRect();
		buildAllTriangleElementsIfDirty(offset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		switch (element.type)
		{
		case CanvasElementType::Image:
		{
			UINT32 numQuads = element.imageSprite->getNumQuads(renderElementIdx);
			numVertices = numQuads * 4;
			numIndices = numQuads * 6;
		}
		case CanvasElementType::Text:
		{
			UINT32 numQuads = element.textSprite->getNumQuads(renderElementIdx);
			numVertices = numQuads * 4;
			numIndices = numQuads * 6;
		}
		case CanvasElementType::Line:
		case CanvasElementType::Triangle:
			numVertices = element.clippedNumVertices;
			numIndices = element.clippedNumVertices;
			break;
		default:
			numVertices = 0;
			numIndices = 0;
			break;
		}
	}

	void GUICanvas::updateRenderElementsInternal()
	{
		mNumRenderElements = 0;
		for(auto& element : mElements)
		{
			switch(element.type)
			{
			case CanvasElementType::Image:
				buildImageElement(element);
				element.renderElemStart = mNumRenderElements;
				element.renderElemEnd = element.renderElemStart + element.imageSprite->getNumRenderElements();
				break;
			case CanvasElementType::Text:
				buildTextElement(element);
				element.renderElemStart = mNumRenderElements;
				element.renderElemEnd = element.renderElemStart + element.textSprite->getNumRenderElements();
				break;
			case CanvasElementType::Line:
			case CanvasElementType::Triangle:
				element.renderElemStart = mNumRenderElements;
				element.renderElemEnd = element.renderElemStart + 1;

				mTriangleElementData[element.dataId].matInfo.groupId = (UINT64)_getParentWidget();

				// Actual mesh build happens when reading from it, because the mesh size varies due to clipping rectangle/offset
				break;
			}

			mNumRenderElements = element.renderElemEnd;
		}

		GUIElement::updateRenderElementsInternal();
	}

	Vector2I GUICanvas::_getOptimalSize() const
	{
		return Vector2I(10, 10);
	}

	void GUICanvas::_fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const
	{
		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.getLocalClipRect();

		Vector2 floatOffset((float)offset.x, (float)offset.y);
		buildAllTriangleElementsIfDirty(floatOffset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		switch(element.type)
		{
		case CanvasElementType::Image:
		{
			const Rect2I& area = mImageData[element.dataId].area;

			offset.x += area.x;
			offset.y += area.y;
			clipRect.x -= area.x;
			clipRect.y -= area.y;

			element.imageSprite->fillBuffer(vertices, uv, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, offset, clipRect);
		}
			break;
		case CanvasElementType::Text:
		{
			const Vector2I& position = mTextData[element.dataId].position;
			offset += position;
			clipRect.x += position.x;
			clipRect.y += position.y;

			element.textSprite->fillBuffer(vertices, uv, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, offset, mLayoutData.getLocalClipRect());
		}
			break;
		case CanvasElementType::Triangle:
		case CanvasElementType::Line:
		{
			UINT32 startVert = vertexOffset;
			UINT32 startIndex = indexOffset;

			UINT32 maxVertIdx = maxNumVerts;
			UINT32 maxIndexIdx = maxNumIndices;

			UINT32 numVertices = element.clippedNumVertices;
			UINT32 numIndices = numVertices;

			assert((startVert + numVertices) <= maxVertIdx);
			assert((startIndex + numIndices) <= maxIndexIdx);

			UINT8* vertDst = vertices + startVert * vertexStride;
			UINT8* uvDst = uv + startVert * vertexStride;
			UINT32* indexDst = indices + startIndex;

			Vector2 zeroUV;
			for(UINT32 i = 0; i < element.clippedNumVertices; i++)
			{
				memcpy(vertDst, &mClippedVertices[element.clippedVertexStart + i], sizeof(Vector2));
				memcpy(uvDst, &zeroUV, sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;
				indexDst[i] = i;
			}
		}
			break;
		}
	}

	void GUICanvas::buildImageElement(const CanvasElement& element)
	{
		assert(element.type == CanvasElementType::Image);

		const ImageElementData& imageData = mImageData[element.dataId];

		IMAGE_SPRITE_DESC desc;
		desc.width = imageData.area.width;
		desc.height = imageData.area.height;

		desc.transparent = true;
		desc.color = element.color;

		Vector2I textureSize;
		if (SpriteTexture::checkIsLoaded(imageData.texture))
		{
			desc.texture = imageData.texture.getInternalPtr();
			textureSize.x = desc.texture->getWidth();
			textureSize.y = desc.texture->getHeight();
		}

		Vector2I destSize(mLayoutData.area.width, mLayoutData.area.height);
		desc.uvScale = ImageSprite::getTextureUVScale(textureSize, destSize, element.scaleMode);

		element.imageSprite->update(desc, (UINT64)_getParentWidget());
	}

	void GUICanvas::buildTextElement(const CanvasElement& element)
	{
		assert(element.type == CanvasElementType::Text);

		const TextElementData& textData = mTextData[element.dataId];

		TEXT_SPRITE_DESC desc;
		desc.font = textData.font;
		desc.fontSize = element.size;
		desc.text = textData.string;
		desc.color = element.color;

		element.textSprite->update(desc, (UINT64)_getParentWidget());
	}

	void GUICanvas::buildTriangleElement(const CanvasElement& element, const Vector2& offset, const Rect2I& clipRect) const
	{
		assert(element.type == CanvasElementType::Triangle || element.type == CanvasElementType::Line);

		UINT8* verticesToClip;
		UINT32 trianglesToClip;
		bool freeVertices;

		if (element.type == CanvasElementType::Triangle)
		{
			verticesToClip = (UINT8*)&mVertexData[element.vertexStart];
			trianglesToClip = element.numVertices / 3;
			freeVertices = false;
		}
		else
		{
			UINT32 numLines = element.numVertices - 1;

			verticesToClip = (UINT8*)bs_stack_alloc(sizeof(Vector2) * (numLines * 6));
			trianglesToClip = numLines * 2;
			freeVertices = true;

			const Vector2* linePoints = &mVertexData[element.vertexStart];
			ShapeMeshes2D::quadLineList(linePoints, element.numVertices, element.lineWidth, verticesToClip,
				sizeof(Vector2), false);
		}

		element.clippedVertexStart = (UINT32)mClippedVertices.size();
		element.clippedNumVertices = 0;
		auto writeCallback = [&](Vector2* vertices, Vector2* uvs, UINT32 count)
		{
			for (UINT32 i = 0; i < count; i++)
				mClippedVertices.push_back(vertices[i] + offset);

			element.clippedNumVertices += count;
		};

		ImageSprite::clipTrianglesToRect(verticesToClip, nullptr, trianglesToClip, sizeof(Vector2), clipRect, writeCallback);

		if(freeVertices)
			bs_stack_free(verticesToClip);
	}

	void GUICanvas::buildAllTriangleElementsIfDirty(const Vector2& offset, const Rect2I& clipRect) const
	{
		// We need to rebuild if new triangle element(s) were added, or if offset or clip rectangle changed
		bool isDirty = mForceTriangleBuild || (mLastOffset != offset) || (mLastClipRect != clipRect);
		if (!isDirty)
			return;

		mClippedVertices.clear();
		for(auto& element : mElements)
		{
			if (element.type != CanvasElementType::Triangle && element.type != CanvasElementType::Line)
				continue;

			buildTriangleElement(element, offset, clipRect);
		}

		mLastOffset = offset;
		mLastClipRect = clipRect;
		mForceTriangleBuild = false;
	}

	const GUICanvas::CanvasElement& GUICanvas::findElement(UINT32 renderElementIdx) const
	{
		INT32 start = 0;
		INT32 end = (INT32)mElements.size();

		while (start < end)
		{
			INT32 middle = (start + end) / 2;
			const CanvasElement& current = mElements[middle];

			if (current.renderElemStart >= renderElementIdx && renderElementIdx < current.renderElemEnd)
				return current;

			if (renderElementIdx < current.renderElemStart)
				end = middle - 1;
			else
				start = middle + 1;
		}

		BS_EXCEPT(InvalidParametersException, "Cannot find requested GUI render element.");

		static CanvasElement dummyElement;
		return dummyElement;
	}
}