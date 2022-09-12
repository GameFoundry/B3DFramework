//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUICanvas.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUITexture.h"
#include "Utility/BsShapeMeshes2D.h"
#include "2D/BsSpriteManager.h"
#include "2D/BsSpriteMaterials.h"
#include "Mesh/BsMeshUtility.h"
#include "Error/BsException.h"

namespace bs
{
	const float GUICanvas::LINE_SMOOTH_BORDER_WIDTH = 3.0f;

	const String& GUICanvas::GetGUITypeName()
	{
		static String name = "Canvas";
		return name;
	}

	GUICanvas::GUICanvas(const String& styleName, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions)
	{ }

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

	void GUICanvas::DrawLine(const Vector2I& a, const Vector2I& b, const Color& color, UINT8 depth)
	{
		drawPolyLine({ a, b }, color, depth);
	}

	void GUICanvas::DrawPolyLine(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
	{
		if(vertices.Size() < 2)
			return;

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.Back();

		element.type = CanvasElementType::Line;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.Size();
		element.vertexStart = (UINT32)mVertexData.Size();
		element.numVertices = (UINT32)vertices.Size();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.Back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		for (auto& vertex : vertices)
		{
			Vector2 point = Vector2((float)vertex.x, (float)vertex.y);
			point += Vector2(0.5f, 0.5f); // Offset to the middle of the pixel

			mVertexData.push_back(point);
		}

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawTexture(const HSpriteTexture& texture, const Rect2I& area, TextureScaleMode scaleMode,
		const Color& color, UINT8 depth)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.Back();

		element.type = CanvasElementType::Image;
		element.color = color;
		element.dataId = (UINT32)mImageData.Size();
		element.scaleMode = scaleMode;
		element.imageSprite = bs_new<ImageSprite>();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mImageData.push_back({ texture, area });
		_markContentAsDirty();
	}

	void GUICanvas::DrawTriangleStrip(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
	{
		if (vertices.Size() < 3)
		{
			BS_LOG(Warning, GUI, "Invalid number of vertices. Ignoring call.");
			return;
		}

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.Back();

		element.type = CanvasElementType::Triangle;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.Size();
		element.vertexStart = (UINT32)mVertexData.Size();
		element.numVertices = (UINT32)(vertices.Size() - 2) * 3;
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		// Convert strip to list
		for(UINT32 i = 2; i < (UINT32)vertices.Size(); i++)
		{
			if (i % 2 == 0)
			{
				mVertexData.push_back(Vector2((float)vertices[i - 2].x + 0.5f, (float)vertices[i - 2].y + 0.5f));
				mVertexData.push_back(Vector2((float)vertices[i - 1].x + 0.5f, (float)vertices[i - 1].y + 0.5f));
				mVertexData.push_back(Vector2((float)vertices[i - 0].x + 0.5f, (float)vertices[i - 0].y + 0.5f));
			}
			else
			{
				mVertexData.push_back(Vector2((float)vertices[i - 0].x + 0.5f, (float)vertices[i - 0].y + 0.5f));
				mVertexData.push_back(Vector2((float)vertices[i - 1].x + 0.5f, (float)vertices[i - 1].y + 0.5f));
				mVertexData.push_back(Vector2((float)vertices[i - 2].x + 0.5f, (float)vertices[i - 2].y + 0.5f));
			}
		}

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.Back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::DrawTriangleList(const Vector<Vector2I>& vertices, const Color& color, UINT8 depth)
	{
		if (vertices.Size() < 3 || vertices.size() % 3 != 0)
		{
			BS_LOG(Warning, GUI, "Invalid number of vertices. Ignoring call.");
			return;
		}

		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.Back();

		element.type = CanvasElementType::Triangle;
		element.color = color;
		element.dataId = (UINT32)mTriangleElementData.Size();
		element.vertexStart = (UINT32)mVertexData.Size();
		element.numVertices = (UINT32)vertices.Size();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		for (auto& vertex : vertices)
			mVertexData.push_back(Vector2((float)vertex.x + 0.5f, (float)vertex.y + 0.5f));

		mTriangleElementData.push_back(TriangleElementData());
		TriangleElementData& elemData = mTriangleElementData.Back();
		elemData.matInfo.groupId = 0;
		elemData.matInfo.tint = color;

		mForceTriangleBuild = true;
		_markContentAsDirty();
	}

	void GUICanvas::drawText(const String& text, const Vector2I& position, const HFont& font, UINT32 size,
		const Color& color, UINT8 depth)
	{
		mElements.push_back(CanvasElement());
		CanvasElement& element = mElements.Back();

		element.type = CanvasElementType::Text;
		element.color = color;
		element.dataId = (UINT32)mTextData.Size();
		element.size = size;
		element.textSprite = bs_new<TextSprite>();
		element.depth = depth;

		mDepthRange = std::max(mDepthRange, (UINT8)(depth + 1));

		mTextData.push_back({ text, font, position });
		_markContentAsDirty();
	}

	void GUICanvas::Clear()
	{
		for (auto& element : mElements)
		{
			if(element.type == CanvasElementType::Image && element.imageSprite != nullptr)
				bs_delete(element.imageSprite);

			if (element.type == CanvasElementType::Text && element.textSprite != nullptr)
				bs_delete(element.textSprite);
		}

		mElements.Clear();
		mRenderElements.Clear();
		mDepthRange = 1;

		mVertexData.Clear();
		mImageData.Clear();
		mTextData.Clear();
		mTriangleElementData.Clear();
		mClippedVertices.Clear();
		mClippedLineVertices.Clear();
		mForceTriangleBuild = false;
	}

	void GUICanvas::UpdateRenderElementsInternal()
	{
		Vector2 Offset((float)mLayoutData.area.x, (float)mLayoutData.area.y);
		Rect2I clipRect = mLayoutData.GetLocalClipRect();
		buildAllTriangleElementsIfDirty(offset, clipRect);

		mRenderElements.Clear();
		for(auto& element : mElements)
		{
			element.renderElemStart = mRenderElements.Size();
			
			switch(element.type)
			{
			case CanvasElementType::Image:
				buildImageElement(element);

				for(UINT32 i = 0; i < element.imageSprite->GetNumRenderElements(); i++)
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					element.imageSprite->GetRenderElementInfo(i, renderElement);

					renderElement.depth = element.depth;
					renderElement.type = GUIMeshType::Triangle;
				}
				
				break;
			case CanvasElementType::Text:
				buildTextElement(element);

				for(UINT32 i = 0; i < element.textSprite->GetNumRenderElements(); i++)
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					element.textSprite->GetRenderElementInfo(i, renderElement);

					renderElement.depth = element.depth;
					renderElement.type = GUIMeshType::Triangle;
				}
				break;
			case CanvasElementType::Line:
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					renderElement.numVertices = element.clippedNumVertices;
					renderElement.numIndices = element.clippedNumVertices;

					renderElement.material = SpriteManager::instance().GetLineMaterial();
					renderElement.matInfo = &mTriangleElementData[element.dataId].matInfo;
					
					renderElement.depth = element.depth;
					renderElement.type = GUIMeshType::Line;

					mTriangleElementData[element.dataId].matInfo.groupId = (UINT64)_getParentWidget();

					// Actual mesh build happens when reading from it, because the mesh size varies due to clipping rectangle/offset
					break;
				}

			case CanvasElementType::Triangle:
				{
					mRenderElements.Add(GUIRenderElement());
					GUIRenderElement& renderElement = mRenderElements.Back();

					renderElement.numVertices = element.clippedNumVertices;
					renderElement.numIndices = element.clippedNumVertices;

					renderElement.material = SpriteManager::instance().GetImageMaterial(SpriteMaterialTransparency::Alpha);
					renderElement.matInfo = &mTriangleElementData[element.dataId].matInfo;

					renderElement.depth = element.depth;
					renderElement.type = GUIMeshType::Triangle;

					mTriangleElementData[element.dataId].matInfo.groupId = (UINT64)_getParentWidget();

					// Actual mesh build happens when reading from it, because the mesh size varies due to clipping rectangle/offset
					break;
				}
			}

			element.renderElemEnd = mRenderElements.Size();
		}

		GUIElement::updateRenderElementsInternal();
	}

	Vector2I GUICanvas::_getOptimalSize() const
	{
		return Vector2I(10, 10);
	}

	void GUICanvas::_fillBuffer(
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
		UINT32 indexStride = sizeof(UINT32);

		Vector2I layoutOffset = Vector2I(mLayoutData.area.x, mLayoutData.area.y) + offset;
		Rect2I clipRect = mLayoutData.GetLocalClipRect();

		Vector2 FloatOffset((float)layoutOffset.x, (float)layoutOffset.y);
		buildAllTriangleElementsIfDirty(floatOffset, clipRect);

		const CanvasElement& element = findElement(renderElementIdx);
		renderElementIdx -= element.renderElemStart;

		switch(element.type)
		{
		case CanvasElementType::Image:
		{
			UINT32 vertexStride = sizeof(Vector2) * 2;
			const Rect2I& area = mImageData[element.dataId].area;

			layoutOffset.x += area.x;
			layoutOffset.y += area.y;
			clipRect.x -= area.x;
			clipRect.y -= area.y;

			element.imageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
		}
			break;
		case CanvasElementType::Text:
		{
			UINT32 vertexStride = sizeof(Vector2) * 2;
			const Vector2I& position = mTextData[element.dataId].position;
			layoutOffset += position;
			clipRect.x -= position.x;
			clipRect.y -= position.y;

			element.textSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
				vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
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

			Vector2 ZeroUV(BsZero);
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

	void GUICanvas::BuildImageElement(const CanvasElement& element)
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
			desc.texture = imageData.texture;
			textureSize.x = desc.texture->GetWidth();
			textureSize.y = desc.texture->GetHeight();
		}

		Vector2I DestSize(mLayoutData.area.width, mLayoutData.area.height);
		desc.uvScale = ImageSprite::getTextureUVScale(textureSize, destSize, element.scaleMode);

		element.imageSprite->Update(desc, (UINT64)_getParentWidget());
	}

	void GUICanvas::BuildTextElement(const CanvasElement& element)
	{
		assert(element.type == CanvasElementType::Text);

		const TextElementData& textData = mTextData[element.dataId];

		TEXT_SPRITE_DESC desc;
		desc.font = textData.font;
		desc.fontSize = element.size;
		desc.text = textData.string;
		desc.color = element.color;

		element.textSprite->Update(desc, (UINT64)_getParentWidget());
	}

	void GUICanvas::BuildTriangleElement(const CanvasElement& element, const Vector2& offset, const Rect2I& clipRect) const
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

			element.clippedVertexStart = (UINT32)mClippedVertices.Size();
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
			{{
				Plane2D(Vector2(1.0f, 0.0f), (float)clipRect.x),
				Plane2D(Vector2(-1.0f, 0.0f), (float)-(clipRect.x + (INT32)clipRect.width)),
				Plane2D(Vector2(0.0f, 1.0f), (float)clipRect.y),
				Plane2D(Vector2(0.0f, -1.0f), (float)-(clipRect.y + (INT32)clipRect.height))
			}};

			element.clippedVertexStart = (UINT32)mClippedLineVertices.Size();
			element.clippedNumVertices = 0;

			for (UINT32 i = 0; i < numLines; i++)
			{
				Vector2 a = linePoints[i];
				Vector2 b = linePoints[i + 1];

				bool isVisible = true;
				for(UINT32 j = 0; j < (UINT32)clipPlanes.Size(); j++)
				{
					const Plane2D& plane = clipPlanes[j];
					float d0 = plane.normal.Dot(a) - plane.d;
					float d1 = plane.normal.Dot(b) - plane.d;

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

	void GUICanvas::BuildAllTriangleElementsIfDirty(const Vector2& offset, const Rect2I& clipRect) const
	{
		// We need to rebuild if new triangle element(s) were added, or if offset or clip rectangle changed
		bool isDirty = mForceTriangleBuild || (mLastOffset != offset) || (mLastClipRect != clipRect);
		if (!isDirty)
			return;

		mClippedVertices.Clear();
		mClippedLineVertices.Clear();
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

	const GUICanvas::CanvasElement& GUICanvas::FindElement(UINT32 renderElementIdx) const
	{
		INT32 start = 0;
		INT32 end = (INT32)(mElements.Size() - 1);

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
	}
}
