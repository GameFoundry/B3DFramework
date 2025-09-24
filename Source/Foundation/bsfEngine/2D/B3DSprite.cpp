//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsTextSprite.h"
#include "Math/BsVector2.h"
#include "Math/BsPlane.h"
#include "Mesh/BsMeshUtility.h"

using namespace b3d;

u32 SpriteRenderElement::GetVertexAndIndexData(u32 vertexOffset, u32 indexOffset, const Vector2& offset, const Area2& clipRectangle, bool performClipping, DataRange& outPositions, DataRange& outUVs, DataRange& outIndices) const
{
	const u32 startVertex = vertexOffset;
	const u32 startIndex = indexOffset;

	const u32 quadCount = VertexCount / 4;

	B3D_ASSERT((startVertex + VertexCount) <= outPositions.ElementCount);
	B3D_ASSERT((startVertex + VertexCount) <= outUVs.ElementCount);
	B3D_ASSERT((startIndex + IndexCount) <= outIndices.ElementCount);

	if(performClipping)
	{
		for(u32 quadIndex = 0; quadIndex < quadCount; quadIndex++)
		{
			const u32 localFirstVertexIndex = quadIndex * 4;
			const u32 globalFirstVertexIndex = startVertex + localFirstVertexIndex;

			for(u32 vertexIndex = 0; vertexIndex < 4; vertexIndex++)
			{
				outPositions.Set(globalFirstVertexIndex + vertexIndex, VertexPositions[localFirstVertexIndex + vertexIndex]);
				outUVs.Set(globalFirstVertexIndex + vertexIndex, VertexUVs[localFirstVertexIndex + vertexIndex]);
			}

			Sprite::ClipQuadsToRectangle(outPositions, outUVs, 1, globalFirstVertexIndex, clipRectangle);

			for(u32 vertexIndex = 0; vertexIndex < 4; vertexIndex++)
				outPositions.At<Vector2>(globalFirstVertexIndex + vertexIndex) += offset;
		}
	}
	else
	{
		for(u32 quadIndex = 0; quadIndex < quadCount; quadIndex++)
		{
			const u32 localFirstVertexIndex = quadIndex * 4;
			const u32 globalFirstVertexIndex = startVertex + localFirstVertexIndex;

			for(u32 vertexIndex = 0; vertexIndex < 4; vertexIndex++)
			{
				outPositions.Set(globalFirstVertexIndex + vertexIndex, VertexPositions[localFirstVertexIndex + vertexIndex] + offset);
				outUVs.Set(globalFirstVertexIndex + vertexIndex, VertexUVs[localFirstVertexIndex + vertexIndex]);
			}
		}
	}

	if(outIndices.Data != nullptr)
	{
		for(u32 index = 0; index < IndexCount; ++index)
			outIndices.Set(startIndex + index, Indices[index]);
		
	}

	return quadCount;
}

Area2I Sprite::GetBounds(const Vector2I& offset, const Area2I& clipRect) const
{
	Area2I bounds = mBounds;

	if(clipRect.Width > 0 && clipRect.Height > 0)
		bounds.Clip(clipRect);

	bounds.X += offset.X;
	bounds.Y += offset.Y;

	return bounds;
}

u32 Sprite::FillBuffer(u8* vertices, u8* uv, u32* indices, u32 vertexOffset, u32 indexOffset, u32 maxNumVerts, u32 maxNumIndices, u32 vertexStride, u32 indexStride, u32 renderElementIdx, const Vector2I& offset, const Area2I& clipRect, bool clip) const
{
	const RenderElementData& renderElementData = mCachedRenderElements[renderElementIdx];
	const SpriteRenderElement& renderElement = renderElementData.RenderElement;

	u32 startVert = vertexOffset;
	u32 startIndex = indexOffset;

	u32 maxVertIdx = maxNumVerts;
	u32 maxIndexIdx = maxNumIndices;

	u32 numVertices = renderElement.VertexCount;
	u32 numIndices = renderElement.IndexCount;
	const u32 quadCount = numVertices / 4;

	B3D_ASSERT((startVert + numVertices) <= maxVertIdx);
	B3D_ASSERT((startIndex + numIndices) <= maxIndexIdx);

	u8* vertDst = vertices + startVert * vertexStride;
	u8* uvDst = uv + startVert * vertexStride;

	// TODO - I'm sure this can be done in a more cache friendly way. Profile it later.
	Vector2 vecOffset((float)offset.X, (float)offset.Y);
	if(clip)
	{
		for(u32 i = 0; i < quadCount; i++)
		{
			u8* vecStart = vertDst;
			u8* uvStart = uvDst;
			u32 vertIdx = i * 4;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 0], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 0], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 1], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 1], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 2], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 2], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 3], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 3], sizeof(Vector2));

			ClipQuadsToRect(vecStart, uvStart, 1, vertexStride, clipRect);

			vertDst = vecStart;
			Vector2* curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			uvDst += vertexStride;
		}
	}
	else
	{
		for(u32 i = 0; i < quadCount; i++)
		{
			u8* vecStart = vertDst;
			u32 vertIdx = i * 4;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 0], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 0], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 1], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 1], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 2], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 2], sizeof(Vector2));

			vertDst += vertexStride;
			uvDst += vertexStride;

			memcpy(vertDst, &renderElement.VertexPositions[vertIdx + 3], sizeof(Vector2));
			memcpy(uvDst, &renderElement.VertexUVs[vertIdx + 3], sizeof(Vector2));

			vertDst = vecStart;
			Vector2* curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			curVec = (Vector2*)vertDst;
			*curVec += vecOffset;

			vertDst += vertexStride;
			uvDst += vertexStride;
		}
	}

	if(indices != nullptr)
		memcpy(&indices[startIndex], renderElement.Indices, numIndices * sizeof(u32));

	return quadCount;
}

Vector2I Sprite::GetAnchorOffset(SpriteAnchor anchor, u32 width, u32 height)
{
	switch(anchor)
	{
	case SA_TopLeft:
		return -Vector2I(0, 0);
	case SA_TopCenter:
		return -Vector2I(width / 2, 0);
	case SA_TopRight:
		return -Vector2I(width, 0);
	case SA_MiddleLeft:
		return -Vector2I(0, height / 2);
	case SA_MiddleCenter:
		return -Vector2I(width / 2, height / 2);
	case SA_MiddleRight:
		return -Vector2I(width, height / 2);
	case SA_BottomLeft:
		return -Vector2I(0, height);
	case SA_BottomCenter:
		return -Vector2I(width / 2, height);
	case SA_BottomRight:
		return -Vector2I(width, height);
	}

	return Vector2I(0, 0);
}

void Sprite::UpdateBounds() const
{
	Vector2 min;
	Vector2 max;

	// Find starting point
	bool foundStartingPoint = false;
	for(auto& entry : mCachedRenderElements)
	{
		const SpriteRenderElement& renderElement = entry.RenderElement;
		if(renderElement.VertexPositions != nullptr && renderElement.VertexCount > 0)
		{
			min = renderElement.VertexPositions[0];
			max = renderElement.VertexPositions[0];
			foundStartingPoint = true;
			break;
		}
	}

	if(!foundStartingPoint)
	{
		mBounds = Area2I(0, 0, 0, 0);
		return;
	}

	// Calculate bounds
	for(auto& entry : mCachedRenderElements)
	{
		const SpriteRenderElement& renderElement = entry.RenderElement;
		if(renderElement.VertexPositions != nullptr && renderElement.VertexCount > 0)
		{
			const u32 vertexCount = renderElement.VertexCount;

			for(u32 i = 0; i < vertexCount; i++)
			{
				min = Vector2::Min(min, renderElement.VertexPositions[i]);
				max = Vector2::Max(max, renderElement.VertexPositions[i]);
			}
		}
	}

	mBounds = Area2I((int)min.X, (int)min.Y, (int)(max.X - min.X), (int)(max.Y - min.Y));
}

// This will only properly clip an array of quads
// Vertices in the quad must be in a specific order: top left, top right, bottom left, bottom right
// (0, 0) represents top left of the screen
void Sprite::ClipQuadsToRect(u8* vertices, u8* uv, u32 numQuads, u32 vertStride, const Area2I& clipRect)
{
	float left = (float)clipRect.X;
	float right = (float)clipRect.X + clipRect.Width;
	float top = (float)clipRect.Y;
	float bottom = (float)clipRect.Y + clipRect.Height;

	for(u32 i = 0; i < numQuads; i++)
	{
		Vector2* vecA = (Vector2*)(vertices);
		Vector2* vecB = (Vector2*)(vertices + vertStride);
		Vector2* vecC = (Vector2*)(vertices + vertStride * 2);
		Vector2* vecD = (Vector2*)(vertices + vertStride * 3);

		Vector2* uvA = (Vector2*)(uv);
		Vector2* uvB = (Vector2*)(uv + vertStride);
		Vector2* uvC = (Vector2*)(uv + vertStride * 2);
		Vector2* uvD = (Vector2*)(uv + vertStride * 3);

		// Attempt to skip those that are definitely not clipped
		if(vecA->X >= left && vecB->X <= right &&
		   vecA->Y >= top && vecC->Y <= bottom)
		{
			continue;
		}

		// TODO - Skip those that are 100% clipped as well

		float du = (uvB->X - uvA->X) / (vecB->X - vecA->X);
		float dv = (uvA->Y - uvC->Y) / (vecA->Y - vecD->Y);

		if(right < left)
			std::swap(left, right);

		if(bottom < top)
			std::swap(bottom, top);

		// Clip left
		float newLeft = Math::Clamp(vecA->X, left, right);
		float uvLeftOffset = (newLeft - vecA->X) * du;

		// Clip right
		float newRight = Math::Clamp(vecB->X, left, right);
		float uvRightOffset = (vecB->X - newRight) * du;

		// Clip top
		float newTop = Math::Clamp(vecA->Y, top, bottom);
		float uvTopOffset = (newTop - vecA->Y) * dv;

		// Clip bottom
		float newBottom = Math::Clamp(vecC->Y, top, bottom);
		float uvBottomOffset = (vecC->Y - newBottom) * dv;

		vecA->X = newLeft;
		vecC->X = newLeft;
		vecB->X = newRight;
		vecD->X = newRight;
		vecA->Y = newTop;
		vecB->Y = newTop;
		vecC->Y = newBottom;
		vecD->Y = newBottom;

		uvA->X += uvLeftOffset;
		uvC->X += uvLeftOffset;
		uvB->X -= uvRightOffset;
		uvD->X -= uvRightOffset;
		uvA->Y += uvTopOffset;
		uvB->Y += uvTopOffset;
		uvC->Y -= uvBottomOffset;
		uvD->Y -= uvBottomOffset;

		vertices += vertStride * 4;
		uv += vertStride * 4;
	}
}

void Sprite::ClipQuadsToRectangle(DataRange& vertices, DataRange& uv, u32 quadCount, u32 startVertexIndex, const Area2& clipRectangle)
{
	float left = clipRectangle.X;
	float right = clipRectangle.X + clipRectangle.Width;
	float top = clipRectangle.Y;
	float bottom = clipRectangle.Y + clipRectangle.Height;

	if(right < left)
		std::swap(left, right);

	if(bottom < top)
		std::swap(bottom, top);

	for(u32 quadIndex = 0; quadIndex < quadCount; quadIndex++)
	{
		Vector2& vecA = vertices.At<Vector2>(startVertexIndex + quadIndex * 4 + 0);
		Vector2& vecB = vertices.At<Vector2>(startVertexIndex + quadIndex * 4 + 1);
		Vector2& vecC = vertices.At<Vector2>(startVertexIndex + quadIndex * 4 + 2);
		Vector2& vecD = vertices.At<Vector2>(startVertexIndex + quadIndex * 4 + 3);

		Vector2& uvA = uv.At<Vector2>(startVertexIndex + quadIndex * 4 + 0);
		Vector2& uvB = uv.At<Vector2>(startVertexIndex + quadIndex * 4 + 1);
		Vector2& uvC = uv.At<Vector2>(startVertexIndex + quadIndex * 4 + 2);
		Vector2& uvD = uv.At<Vector2>(startVertexIndex + quadIndex * 4 + 3);

		// Attempt to skip those that are definitely not clipped
		if(vecA.X >= left && vecB.X <= right &&
		   vecA.Y >= top && vecC.Y <= bottom)
		{
			continue;
		}

		// TODO - Skip those that are 100% clipped as well

		float du = (uvB.X - uvA.X) / (vecB.X - vecA.X);
		float dv = (uvA.Y - uvC.Y) / (vecA.Y - vecD.Y);

		// Clip left
		float newLeft = Math::Clamp(vecA.X, left, right);
		float uvLeftOffset = (newLeft - vecA.X) * du;

		// Clip right
		float newRight = Math::Clamp(vecB.X, left, right);
		float uvRightOffset = (vecB.X - newRight) * du;

		// Clip top
		float newTop = Math::Clamp(vecA.Y, top, bottom);
		float uvTopOffset = (newTop - vecA.Y) * dv;

		// Clip bottom
		float newBottom = Math::Clamp(vecC.Y, top, bottom);
		float uvBottomOffset = (vecC.Y - newBottom) * dv;

		vecA.X = newLeft;
		vecC.X = newLeft;
		vecB.X = newRight;
		vecD.X = newRight;
		vecA.Y = newTop;
		vecB.Y = newTop;
		vecC.Y = newBottom;
		vecD.Y = newBottom;

		uvA.X += uvLeftOffset;
		uvC.X += uvLeftOffset;
		uvB.X -= uvRightOffset;
		uvD.X -= uvRightOffset;
		uvA.Y += uvTopOffset;
		uvB.Y += uvTopOffset;
		uvC.Y -= uvBottomOffset;
		uvD.Y -= uvBottomOffset;
	}
}

void Sprite::ClipTrianglesToRect(u8* vertices, u8* uv, u32 numTris, u32 vertStride, const Area2I& clipRect, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback)
{
	Vector<Plane> clipPlanes = {
		Plane(Vector3(1.0f, 0.0f, 0.0f), (float)clipRect.X),
		Plane(Vector3(-1.0f, 0.0f, 0.0f), (float)-(clipRect.X + (i32)clipRect.Width)),
		Plane(Vector3(0.0f, 1.0f, 0.0f), (float)clipRect.Y),
		Plane(Vector3(0.0f, -1.0f, 0.0f), (float)-(clipRect.Y + (i32)clipRect.Height))
	};

	MeshUtility::Clip2D(vertices, uv, numTris, vertStride, clipPlanes, writeCallback);
}
