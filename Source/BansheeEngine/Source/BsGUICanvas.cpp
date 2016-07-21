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
#include "BsMeshUtility.h"
#include "BsException.h"

namespace BansheeEngine
{
	const float GUICanvas::LINE_SMOOTH_BORDER_WIDTH = 3.0f;

	const String& GUICanvas::getGUITypeName()
	{
		static String name = "Canvas";
		return name;
	}

	GUICanvas::GUICanvas(const String& styleName, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mNumRenderElements(0), mDepthRange(1), mForceTriangleBuild(false)
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

	void GUICanvas::drawLine(const Vector2I& a, const Vector2I& b, const Color& color, UINT8 depth)
	{
		drawPolyLine({ a, b }, color, depth);
	}

	void GUICanvas::drawPolyLine(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
	{
		if(vertices.size() < 2)
			return;

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Line;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.size();
		element.vertexStart = (UINT32)mVertexData.size();
		element.numVertices = (UINT32)vertices.size();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		for (auto& vertex : vertices)
		{
			Vector2 point = Vector2((float)vertex.x, (float)vertex.y);
			mVertexData.push_back(point);
		}

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawTexture(const HSpriteTexture& texture, const Rect2I& area, TextureScaleMode scaleMode, 
		const Color& color, UINT8 depth)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Image;
		element.color = color;
		element.dataId = (UINT32)mImageData.size();
		element.scaleMode = scaleMode;
		element.imageSprite = bs_new<ImageSprite>();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mImageData.push_back({ texture, area });
		_markContentAsDirty();
	}

	void GUICanvas::drawTriangleStrip(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
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
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		// Convert strip to list
		for(UINT32 i = 2; i < (UINT32)vertices.size(); i++)
		{
			if (i % 2 == 0)
			{
				mVertexData.push_back(Vector2((float)vertices[i - 2].x, (float)vertices[i - 2].y));
				mVertexData.push_back(Vector2((float)vertices[i - 1].x, (float)vertices[i - 1].y));
				mVertexData.push_back(Vector2((float)vertices[i - 0].x, (float)vertices[i - 0].y));
			}
			else
			{
				mVertexData.push_back(Vector2((float)vertices[i - 0].x, (float)vertices[i - 0].y));
				mVertexData.push_back(Vector2((float)vertices[i - 1].x, (float)vertices[i - 1].y));
				mVertexData.push_back(Vector2((float)vertices[i - 2].x, (float)vertices[i - 2].y));
			}
		}

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawTriangleList(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
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
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

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
		const Color& color, UINT8 depth)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.back();

		element.type = CanvasElementType::Text;
		element.color = color;
		element.dataId = (UINT32)mTextData.size();
		element.size = size;
		element.textSprite = bs_new<TextSprite>();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mTextData.push_back({ text, font, position });
		_markContentAsDirty();
	}

	void GUICanvas::clear()
	{
		for (auto& element : mElements)
		{
			if(element.type == CanvasElementType::Image && element.imageSprite != nullptr)
				bs_delete(element.imageSprite);

			if (element.type == CanvasElementType::Text && element.textSprite != nullptr)
				bs_delete(element.textSprite);
		}

		mElements.clear();
		mNumRenderElements = 0;
		mDepthRange = 1;

		mVertexData.clear();
		mImageData.clear();
		mTextData.clear();
		mTriangleElementData.clear();
		mClippedVertices.clear();
		mClippedLineVertices.clear();
		mForceTriangleBuild = false;
	}

	UINT32 GUICanvas::_getNumRenderElements() const
	{
		return mNumRenderElements;
	}

	UINT32 GUICanvas::_getRenderElementDepth(UINT32 renderElementIdx) const
	{
		const CanvasElement& element = findElement(renderElementIdx);
		return _getDepth() + element.depth;
	}

	const SpriteMaterialInfo& GUICanvas::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		static const SpriteMaterialInfo defaultMatInfo;

		Vector2 offset((float)mLayoutData.area.x, (float)mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.getLocalClipRect();
		buildAllTriangleElementsIfDirty(offset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		renderElementIdx -= element.renderElemStart;

		switch (element.type)
		{
		case CanvasElementType::Line:
			*material = SpriteManager::instance().getLineMaterial();
			return mTriangleElementData[element.dataId].matInfo;
		case CanvasElementType::Image:
			*material = element.imageSprite->getMaterial(0);
			return element.imageSprite->getMaterialInfo(0);
		case CanvasElementType::Text:
			*material = element.imageSprite->getMaterial(renderElementIdx);
			return element.textSprite->getMaterialInfo(renderElementIdx);
		case CanvasElementType::Triangle:
			*material = SpriteManager::instance().getImageTransparentMaterial();
			return mTriangleElementData[element.dataId].matInfo;
		default:
			*material = nullptr;
			return defaultMatInfo;
		}
	}

	void GUICanvas::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		Vector2 offset((float)mLayoutData.area.x, (float)mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.getLocalClipRect();
		buildAllTriangleElementsIfDirty(offset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		renderElementIdx -= element.renderElemStart;

		switch (element.type)
		{
		case CanvasElementType::Image:
		{
			UINT32 numQuads = element.imageSprite->getNumQuads(renderElementIdx);
			numVertices = numQuads * 4;
			numIndices = numQuads * 6;
			type = GUIMeshType::Triangle;
			break;
		}
		case CanvasElementType::Text:
		{
			UINT32 numQuads = element.textSprite->getNumQuads(renderElementIdx);
			numVertices = numQuads * 4;
			numIndices = numQuads * 6;
			type = GUIMeshType::Triangle;
			break;
		}
		case CanvasElementType::Line:
			numVertices = element.clippedNumVertices;
			numIndices = element.clippedNumVertices;
			type = GUIMeshType::Line;
			break;
		case CanvasElementType::Triangle:
			numVertices = element.clippedNumVertices;
			numIndices = element.clippedNumVertices;
			type = GUIMeshType::Triangle;
			break;
		default:
			numVertices = 0;
			numIndices = 0;
			type = GUIMeshType::Triangle;
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

	void GUICanvas::_fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 indexStride = sizeof(UINT32);

		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.getLocalClipRect();

		Vector2 floatOffset((float)offset.x, (float)offset.y);
		buildAllTriangleElementsIfDirty(floatOffset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		renderElementIdx -= element.renderElemStart;

		switch(element.type)
		{
		case CanvasElementType::Image:
		{
			UINT32 vertexStride = sizeof(Vector2) * 2;
			const Rect2I& area = mImageData[element.dataId].area;

			offset.x += area.x;
			offset.y += area.y;
			clipRect.x -= area.x;
			clipRect.y -= area.y;

			element.imageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, offset, clipRect);
		}
			break;
		case CanvasElementType::Text:
		{
			UINT32 vertexStride = sizeof(Vector2) * 2;
			const Vector2I& position = mTextData[element.dataId].position;
			offset += position;
			clipRect.x -= position.x;
			clipRect.y -= position.y;

			element.textSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, offset, clipRect);
		}
			break;
		case CanvasElementType::Triangle:
		{
			UINT32 vertexStride = sizeof(Vector2) * 2;

			UINT32 startVert = vertexOffset;
			UINT32 startIndex = indexOffset;

			UINT32 maxVertIdx = maxNumVerts;
			UINT32 maxIndexIdx = maxNumIndices;

			UINT32 numVertices = element.clippedNumVertices;
			UINT32 numIndices = numVertices;

			assert((startVert + numVertices) <= maxVertIdx);
			assert((startIndex + numIndices) <= maxIndexIdx);

			UINT8* vertDst = vertices + startVert * vertexStride;
			UINT8* uvDst = uvs + startVert * vertexStride;
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
		case CanvasElementType::Line:
		{
			UINT32 vertexStride = sizeof(Vector2);

			UINT32 startVert = vertexOffset;
			UINT32 startIndex = indexOffset;

			UINT32 maxVertIdx = maxNumVerts;
			UINT32 maxIndexIdx = maxNumIndices;

			UINT32 numVertices = element.clippedNumVertices;
			UINT32 numIndices = numVertices;

			assert((startVert + numVertices) <= maxVertIdx);
			assert((startIndex + numIndices) <= maxIndexIdx);

			UINT8* vertDst = vertices + startVert * vertexStride;
			UINT32* indexDst = indices + startIndex;

			for (UINT32 i = 0; i < element.clippedNumVertices; i++)
			{
				const Vector2& point = mClippedLineVertices[element.clippedVertexStart + i];

				memcpy(vertDst, &point, sizeof(Vector2));

				vertDst += vertexStride;
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

		if (element.type == CanvasElementType::Triangle)
		{
			UINT8* verticesToClip = (UINT8*)&mVertexData[element.vertexStart];
			UINT32 trianglesToClip = element.numVertices / 3;

			auto writeCallback = [&](Vector2* vertices, Vector2* uvs, UINT32 count)
			{
				for (UINT32 i = 0; i < count; i++)
					mClippedVertices.push_back(vertices[i] + offset);

				element.clippedNumVertices += count;
			};

			element.clippedVertexStart = (UINT32)mClippedVertices.size();
			element.clippedNumVertices = 0;

			ImageSprite::clipTrianglesToRect(verticesToClip, nullptr, trianglesToClip, sizeof(Vector2), clipRect, 
				writeCallback);
		}
		else
		{
			UINT32 numLines = element.numVertices - 1;
			const Vector2* linePoints = &mVertexData[element.vertexStart];

			struct Plane2D
			{
				Plane2D(const Vector2& normal, float d)
					:normal(normal), d(d)
				{ }

				Vector2 normal;
				float d;
			};

			std::array<Plane2D, 4> clipPlanes =
			{
				Plane2D(Vector2(1.0f, 0.0f), (float)clipRect.x),
				Plane2D(Vector2(-1.0f, 0.0f), (float)-(clipRect.x + clipRect.width)),
				Plane2D(Vector2(0.0f, 1.0f), (float)clipRect.y),
				Plane2D(Vector2(0.0f, -1.0f), (float)-(clipRect.y + clipRect.height))
			};

			element.clippedVertexStart = (UINT32)mClippedLineVertices.size();
			element.clippedNumVertices = 0;

			for (UINT32 i = 0; i < numLines; i++)
			{
				Vector2 a = linePoints[i];
				Vector2 b = linePoints[i + 1];

				bool isVisible = true;
				for(UINT32 j = 0; j < (UINT32)clipPlanes.size(); j++)
				{
					const Plane2D& plane = clipPlanes[j];
					float d0 = plane.normal.dot(a) - plane.d;
					float d1 = plane.normal.dot(b) - plane.d;

					// Line not visible
					if (d0 <= 0 && d1 <= 0)
					{
						isVisible = false;
						break;
					}

					// Line visible completely
					if (d0 >= 0 && d1 >= 0)
						continue;

					// The line is split by the plane, compute the point of intersection.
					float t = d0 / (d0 - d1);
					Vector2 intersectPt = (1 - t)*a + t*b;

					if (d0 > 0)
						b = intersectPt;
					else
						a = intersectPt;
				}

				if (!isVisible)
					continue;

				mClippedLineVertices.push_back(a + offset);
				mClippedLineVertices.push_back(b + offset);

				element.clippedNumVertices += 2;
			}
		}
	}

	void GUICanvas::buildAllTriangleElementsIfDirty(const Vector2& offset, const Rect2I& clipRect) const
	{
		// We need to rebuild if new triangle element(s) were added, or if offset or clip rectangle changed
		bool isDirty = mForceTriangleBuild || (mLastOffset != offset) || (mLastClipRect != clipRect);
		if (!isDirty)
			return;

		mClippedVertices.clear();
		mClippedLineVertices.clear();
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
		INT32 end = (INT32)(mElements.size() - 1);

		while (start <= end)
		{
			INT32 middle = (start + end) / 2;
			const CanvasElement& current = mElements[middle];

			if (renderElementIdx >= current.renderElemStart && renderElementIdx < current.renderElemEnd)
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