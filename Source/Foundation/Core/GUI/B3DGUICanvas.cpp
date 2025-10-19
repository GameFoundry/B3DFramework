//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUICanvas.h"
#include "Image/B3DSpriteTexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUITexture.h"
#include "Utility/B3DShapeMeshes2D.h"
#include "2D/B3DSpriteManager.h"
#include "Mesh/B3DMeshUtility.h"
#include "Error/B3DException.h"

using namespace b3d;

const float GUICanvas::kLineSmoothBorderWidth = 3.0f;

const String& GUICanvas::GetGuiTypeName()
{
	static String name = "Canvas";
	return name;
}

GUICanvas::GUICanvas(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIInteractable(styleName, sizeConstraints)
{}

GUICanvas::~GUICanvas()
{
	Clear();
}

void GUICanvas::DrawLine(const GUILogicalPoint& a, const GUILogicalPoint& b, const Color& color, u8 depth)
{
	DrawPolyLine({ a, b }, color, depth);
}

void GUICanvas::DrawPolyLine(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth)
{
	if(vertices.size() < 2)
		return;

	mElements.push_back(CanvasElement());
	CanvasElement& element = mElements.back();

	element.Type = CanvasElementType::Line;
	element.Color = color;
	element.DataId = (u32)mTriangleElementData.size();
	element.VertexStart = (u32)mVertexData.size();
	element.VertexCount = (u32)vertices.size();
	element.Depth = depth;

	mDepthRange = std::max(mDepthRange, (u8)(depth + 1));

	mTriangleElementData.push_back(TriangleElementData());
	TriangleElementData& elemData = mTriangleElementData.back();
	elemData.MaterialInfo.GroupId = 0;
	elemData.MaterialInfo.Tint = color;

	for(auto& vertex : vertices)
	{
		GUILogicalPointF point = GUILogicalPointF((float)vertex.X, (float)vertex.Y);
		point += GUILogicalPointF(0.5f, 0.5f); // Offset to the middle of the pixel

		mVertexData.push_back(point);
	}

	mForceTriangleBuild = true;
	MarkContentAsDirty();
}

void GUICanvas::DrawImage(const HSpriteImage& image, const GUILogicalArea& area, const Color& color, TextureScaleMode scaleMode, u8 depth)
{
	mElements.push_back(CanvasElement());
	CanvasElement& element = mElements.back();

	element.Type = CanvasElementType::Image;
	element.Color = color;
	element.DataId = (u32)mImageData.size();
	element.ScaleMode = scaleMode;
	element.ImageSprite = B3DNew<ImageSprite>();
	element.Depth = depth;

	mDepthRange = std::max(mDepthRange, (u8)(depth + 1));

	mImageData.push_back({ image, area });
	MarkContentAsDirty();
}

void GUICanvas::DrawTriangleStrip(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth)
{
	if(vertices.size() < 3)
	{
		B3D_LOG(Warning, GUI, "Invalid number of vertices. Ignoring call.");
		return;
	}

	mElements.push_back(CanvasElement());
	CanvasElement& element = mElements.back();

	element.Type = CanvasElementType::Triangle;
	element.Color = color;
	element.DataId = (u32)mTriangleElementData.size();
	element.VertexStart = (u32)mVertexData.size();
	element.VertexCount = (u32)(vertices.size() - 2) * 3;
	element.Depth = depth;

	mDepthRange = std::max(mDepthRange, (u8)(depth + 1));

	// Convert strip to list
	for(u32 vertexIndex = 2; vertexIndex < (u32)vertices.size(); vertexIndex++)
	{
		if(vertexIndex % 2 == 0)
		{
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 2].X + 0.5f, (float)vertices[vertexIndex - 2].Y + 0.5f));
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 1].X + 0.5f, (float)vertices[vertexIndex - 1].Y + 0.5f));
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 0].X + 0.5f, (float)vertices[vertexIndex - 0].Y + 0.5f));
		}
		else
		{
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 0].X + 0.5f, (float)vertices[vertexIndex - 0].Y + 0.5f));
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 1].X + 0.5f, (float)vertices[vertexIndex - 1].Y + 0.5f));
			mVertexData.push_back(GUILogicalPointF((float)vertices[vertexIndex - 2].X + 0.5f, (float)vertices[vertexIndex - 2].Y + 0.5f));
		}
	}

	mTriangleElementData.push_back(TriangleElementData());
	TriangleElementData& elemData = mTriangleElementData.back();
	elemData.MaterialInfo.GroupId = 0;
	elemData.MaterialInfo.Tint = color;

	mForceTriangleBuild = true;
	MarkContentAsDirty();
}

void GUICanvas::DrawTriangleList(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth)
{
	if(vertices.size() < 3 || vertices.size() % 3 != 0)
	{
		B3D_LOG(Warning, GUI, "Invalid number of vertices. Ignoring call.");
		return;
	}

	mElements.push_back(CanvasElement());
	CanvasElement& element = mElements.back();

	element.Type = CanvasElementType::Triangle;
	element.Color = color;
	element.DataId = (u32)mTriangleElementData.size();
	element.VertexStart = (u32)mVertexData.size();
	element.VertexCount = (u32)vertices.size();
	element.Depth = depth;

	mDepthRange = std::max(mDepthRange, (u8)(depth + 1));

	for(auto& vertex : vertices)
		mVertexData.push_back(GUILogicalPointF((float)vertex.X + 0.5f, (float)vertex.Y + 0.5f));

	mTriangleElementData.push_back(TriangleElementData());
	TriangleElementData& elemData = mTriangleElementData.back();
	elemData.MaterialInfo.GroupId = 0;
	elemData.MaterialInfo.Tint = color;

	mForceTriangleBuild = true;
	MarkContentAsDirty();
}

void GUICanvas::DrawText(const String& text, const GUILogicalPoint& position, const HFont& font, float size, const Color& color, u8 depth)
{
	mElements.push_back(CanvasElement());
	CanvasElement& element = mElements.back();

	element.Type = CanvasElementType::Text;
	element.Color = color;
	element.DataId = (u32)mTextData.size();
	element.Size = size;
	element.TextSprite = B3DNew<TextSprite>();
	element.Depth = depth;

	mDepthRange = std::max(mDepthRange, (u8)(depth + 1));

	mTextData.push_back({ text, font, position });
	MarkContentAsDirty();
}

void GUICanvas::Clear()
{
	for(auto& element : mElements)
	{
		if(element.Type == CanvasElementType::Image && element.ImageSprite != nullptr)
			B3DDelete(element.ImageSprite);

		if(element.Type == CanvasElementType::Text && element.TextSprite != nullptr)
			B3DDelete(element.TextSprite);
	}

	mElements.clear();
	mRenderElements.Clear();
	mDepthRange = 1;

	mVertexData.clear();
	mImageData.clear();
	mTextData.clear();
	mTriangleElementData.clear();
	mClippedVertices.clear();
	mClippedLineVertices.clear();
	mForceTriangleBuild = false;
}

void GUICanvas::UpdateRenderElements()
{
	Vector2 offset = mAbsolutePosition.To<float>();
	Area2I clipRect = GetClippedAreaTranslatedRelativeToParent().To<i32, u32>();
	BuildAllTriangleElementsIfDirty(offset, mAbsoluteScale, clipRect);

	mRenderElements.Clear();
	for(auto& element : mElements)
	{
		element.RenderElemStart = (u32)mRenderElements.Size();

		switch(element.Type)
		{
		case CanvasElementType::Image:
			BuildImageElement(element);

			for(u32 elementIndex = 0; elementIndex < element.ImageSprite->GetRenderElementCount(); elementIndex++)
			{
				mRenderElements.Add(GUIRenderElement());
				GUIRenderElement& renderElement = mRenderElements.Back();

				element.ImageSprite->GetRenderElement(elementIndex, renderElement);

				renderElement.Depth = element.Depth;
				renderElement.Type = GUIMeshType::Triangle;
			}

			break;
		case CanvasElementType::Text:
			BuildTextElement(element);

			for(u32 elementIndex = 0; elementIndex < element.TextSprite->GetRenderElementCount(); elementIndex++)
			{
				mRenderElements.Add(GUIRenderElement());
				GUIRenderElement& renderElement = mRenderElements.Back();

				element.TextSprite->GetRenderElement(elementIndex, renderElement);

				renderElement.Depth = element.Depth;
				renderElement.Type = GUIMeshType::Triangle;
			}
			break;
		case CanvasElementType::Line:
			{
				mRenderElements.Add(GUIRenderElement());
				GUIRenderElement& renderElement = mRenderElements.Back();

				renderElement.VertexCount = element.ClippedVertexCount;
				renderElement.IndexCount = element.ClippedVertexCount;

				renderElement.Material = SpriteManager::Instance().GetLineMaterial();
				renderElement.MaterialInformation = &mTriangleElementData[element.DataId].MaterialInfo;

				renderElement.Depth = element.Depth;
				renderElement.Type = GUIMeshType::Line;

				mTriangleElementData[element.DataId].MaterialInfo.GroupId = (u64)GetParentWidget();

				// Actual mesh build happens when reading from it, because the mesh size varies due to clipping rectangle/offset
				break;
			}

		case CanvasElementType::Triangle:
			{
				mRenderElements.Add(GUIRenderElement());
				GUIRenderElement& renderElement = mRenderElements.Back();

				renderElement.VertexCount = element.ClippedVertexCount;
				renderElement.IndexCount = element.ClippedVertexCount;

				renderElement.Material = SpriteManager::Instance().GetImageMaterial(SpriteMaterialTransparency::Alpha);
				renderElement.MaterialInformation = &mTriangleElementData[element.DataId].MaterialInfo;

				renderElement.Depth = element.Depth;
				renderElement.Type = GUIMeshType::Triangle;

				mTriangleElementData[element.DataId].MaterialInfo.GroupId = (u64)GetParentWidget();

				// Actual mesh build happens when reading from it, because the mesh size varies due to clipping rectangle/offset
				break;
			}
		}

		element.RenderElemEnd = (u32)mRenderElements.Size();
	}

	GUIInteractable::UpdateRenderElements();
}

GUILogicalSize GUICanvas::CalculateUnconstrainedOptimalSize() const
{
	return GUILogicalSize(10, 10);
}

void GUICanvas::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxVertexCount,
	u32 maxIndexCount,
	u32 renderElementIdx) const
{
	u8* uvs = vertices + sizeof(Vector2);
	u32 indexStride = sizeof(u32);

	Vector2I layoutOffset = mAbsolutePosition.To<i32>() + offset;
	Area2I clipRect = GetClippedAreaTranslatedRelativeToParent().To<i32, u32>();

	Vector2 floatOffset((float)layoutOffset.X, (float)layoutOffset.Y);
	BuildAllTriangleElementsIfDirty(floatOffset, mAbsoluteScale, clipRect);

	const CanvasElement& element = FindElement(renderElementIdx);
	renderElementIdx -= element.RenderElemStart;

	switch(element.Type)
	{
	case CanvasElementType::Image:
		{
			u32 vertexStride = sizeof(Vector2) * 2;
			const GUILogicalArea& area = mImageData[element.DataId].Area;

			layoutOffset.X += (i32)area.X;
			layoutOffset.Y += (i32)area.Y;
			clipRect.X -= (i32)area.X;
			clipRect.Y -= (i32)area.Y;

			element.ImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxVertexCount, maxIndexCount, vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
		}
		break;
	case CanvasElementType::Text:
		{
			u32 vertexStride = sizeof(Vector2) * 2;
			const GUILogicalPoint& position = mTextData[element.DataId].Position;
			layoutOffset += position.To<i32>();
			clipRect.X -= (i32)position.X;
			clipRect.Y -= (i32)position.Y;

			element.TextSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxVertexCount, maxIndexCount, vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
		}
		break;
	case CanvasElementType::Triangle:
		{
			u32 vertexStride = sizeof(Vector2) * 2;

			u32 startVertex = vertexOffset;
			u32 startIndex = indexOffset;

			u32 maxVertexIndex = maxVertexCount;
			u32 maxIndexIndex = maxIndexCount;

			u32 vertexCount = element.ClippedVertexCount;
			u32 indexCount = vertexCount;

			B3D_ASSERT((startVertex + vertexCount) <= maxVertexIndex);
			B3D_ASSERT((startIndex + indexCount) <= maxIndexIndex);

			u8* vertexDestination = vertices + startVertex * vertexStride;
			u8* uvDestination = uvs + startVertex * vertexStride;
			u32* indexDestination = indices + startIndex;

			Vector2 zeroUV(BsZero);
			for(u32 vertexIndex = 0; vertexIndex < element.ClippedVertexCount; vertexIndex++)
			{
				memcpy(vertexDestination, &mClippedVertices[element.ClippedVertexStart + vertexIndex], sizeof(Vector2));
				memcpy(uvDestination, &zeroUV, sizeof(Vector2));

				vertexDestination += vertexStride;
				uvDestination += vertexStride;
				indexDestination[vertexIndex] = vertexIndex;
			}
		}
		break;
	case CanvasElementType::Line:
		{
			u32 vertexStride = sizeof(Vector2);

			u32 startVertex = vertexOffset;
			u32 startIndex = indexOffset;

			u32 maxVertexIndex = maxVertexCount;
			u32 maxIndexIndex = maxIndexCount;

			u32 vertexCount = element.ClippedVertexCount;
			u32 indexCount = vertexCount;

			B3D_ASSERT((startVertex + vertexCount) <= maxVertexIndex);
			B3D_ASSERT((startIndex + indexCount) <= maxIndexIndex);

			u8* vertexDestination = vertices + startVertex * vertexStride;
			u32* indexDestination = indices + startIndex;

			for(u32 vertexIndex = 0; vertexIndex < element.ClippedVertexCount; vertexIndex++)
			{
				const Vector2& point = mClippedLineVertices[element.ClippedVertexStart + vertexIndex];

				memcpy(vertexDestination, &point, sizeof(Vector2));

				vertexDestination += vertexStride;
				indexDestination[vertexIndex] = vertexIndex;
			}
		}
		break;
	}
}

void GUICanvas::BuildImageElement(const CanvasElement& element)
{
	B3D_ASSERT(element.Type == CanvasElementType::Image);

	const ImageElementData& imageData = mImageData[element.DataId];

	ImageSpriteInformation imageSpriteInformation;
	imageSpriteInformation.Size = imageData.Area.GetSize().To<i32>();

	imageSpriteInformation.Transparent = true;
	imageSpriteInformation.Color = element.Color;

	Size2I textureSize(BsZero);
	if(imageData.Image.IsLoaded())
	{
		imageSpriteInformation.Image = imageData.Image;

		textureSize = imageSpriteInformation.Image->GetDefaultAllocatedImage().GetSize();
	}

	Size2I destSize = mAbsoluteSize.To<i32>();
	imageSpriteInformation.UvScale = ImageSprite::GetTextureUvScale(textureSize, destSize, element.ScaleMode);

	element.ImageSprite->Update(imageSpriteInformation, (u64)GetParentWidget());
}

void GUICanvas::BuildTextElement(const CanvasElement& element)
{
	B3D_ASSERT(element.Type == CanvasElementType::Text);

	const TextElementData& textData = mTextData[element.DataId];

	TextSpriteInformation desc;
	desc.Font = textData.Font;
	desc.FontSize = element.Size * mAbsoluteScale;
	desc.Text = textData.String;
	desc.Color = element.Color;

	element.TextSprite->Update(desc, (u64)GetParentWidget());
}

void GUICanvas::BuildTriangleElement(const CanvasElement& element, const Vector2& offset, float scale, const Area2I& clipRect) const
{
	B3D_ASSERT(element.Type == CanvasElementType::Triangle || element.Type == CanvasElementType::Line);

	if(element.Type == CanvasElementType::Triangle)
	{
		u8* verticesToClip = (u8*)&mVertexData[element.VertexStart];
		u32 triangleCount = element.VertexCount / 3;

		auto writeCallback = [&](Vector2* vertices, Vector2* uvs, u32 count)
		{
			for(u32 vertexIndex = 0; vertexIndex < count; vertexIndex++)
				mClippedVertices.push_back((vertices[vertexIndex] * scale) + offset);

			element.ClippedVertexCount += count;
		};

		element.ClippedVertexStart = (u32)mClippedVertices.size();
		element.ClippedVertexCount = 0;

		ImageSprite::ClipTrianglesToRect(verticesToClip, nullptr, triangleCount, sizeof(Vector2), clipRect, writeCallback);
	}
	else
	{
		u32 lineCount = element.VertexCount - 1;
		const GUILogicalPointF* linePoints = &mVertexData[element.VertexStart];

		struct Plane2D
		{
			Plane2D(const Vector2& normal, float d)
				: Normal(normal), D(d)
			{}

			Vector2 Normal;
			float D;
		};

		std::array<Plane2D, 4> clipPlanes = { { Plane2D(Vector2(1.0f, 0.0f), (float)clipRect.X),
												Plane2D(Vector2(-1.0f, 0.0f), (float)-(clipRect.X + (i32)clipRect.Width)),
												Plane2D(Vector2(0.0f, 1.0f), (float)clipRect.Y),
												Plane2D(Vector2(0.0f, -1.0f), (float)-(clipRect.Y + (i32)clipRect.Height)) } };

		element.ClippedVertexStart = (u32)mClippedLineVertices.size();
		element.ClippedVertexCount = 0;

		for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
		{
			GUILogicalPointF a = linePoints[lineIndex];
			GUILogicalPointF b = linePoints[lineIndex + 1];

			bool isVisible = true;
			for(u32 planeIndex = 0; planeIndex < (u32)clipPlanes.size(); planeIndex++)
			{
				const Plane2D& plane = clipPlanes[planeIndex];
				float d0 = plane.Normal.Dot(a.To<float>()) - plane.D;
				float d1 = plane.Normal.Dot(b.To<float>()) - plane.D;

				// Line not visible
				if(d0 <= 0 && d1 <= 0)
				{
					isVisible = false;
					break;
				}

				// Line visible completely
				if(d0 >= 0 && d1 >= 0)
					continue;

				// The line is split by the plane, compute the point of intersection.
				float t = d0 / (d0 - d1);
				GUILogicalPointF intersectPt = (1 - t) * a + t * b;

				if(d0 > 0)
					b = intersectPt;
				else
					a = intersectPt;
			}

			if(!isVisible)
				continue;

			mClippedLineVertices.push_back(a.To<float>() * scale + offset);
			mClippedLineVertices.push_back(b.To<float>() * scale + offset);

			element.ClippedVertexCount += 2;
		}
	}
}

void GUICanvas::BuildAllTriangleElementsIfDirty(const Vector2& offset, float scale, const Area2I& clipRect) const
{
	// We need to rebuild if new triangle element(s) were added, or if offset or clip rectangle changed
	bool isDirty = mForceTriangleBuild || (mLastOffset != offset) || (mLastClipRect != clipRect);
	if(!isDirty)
		return;

	mClippedVertices.clear();
	mClippedLineVertices.clear();
	for(auto& element : mElements)
	{
		if(element.Type != CanvasElementType::Triangle && element.Type != CanvasElementType::Line)
			continue;

		BuildTriangleElement(element, offset, scale, clipRect);
	}

	mLastOffset = offset;
	mLastClipRect = clipRect;
	mForceTriangleBuild = false;
}

const GUICanvas::CanvasElement& GUICanvas::FindElement(u32 renderElementIdx) const
{
	i32 start = 0;
	i32 end = (i32)(mElements.size() - 1);

	while(start <= end)
	{
		i32 middle = (start + end) / 2;
		const CanvasElement& current = mElements[middle];

		if(renderElementIdx >= current.RenderElemStart && renderElementIdx < current.RenderElemEnd)
			return current;

		if(renderElementIdx < current.RenderElemStart)
			end = middle - 1;
		else
			start = middle + 1;
	}

	B3D_EXCEPT(InvalidParametersException, "Cannot find requested GUI render element.");
}
