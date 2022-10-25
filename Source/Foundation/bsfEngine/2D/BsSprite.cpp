//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsTextSprite.h"
#include "Math/BsVector2.h"
#include "Math/BsPlane.h"
#include "Mesh/BsMeshUtility.h"

namespace bs
{
	Rect2I Sprite::GetBounds(const Vector2I& offset, const Rect2I& clipRect) const
	{
		Rect2I bounds = mBounds;

		if(clipRect.Width > 0 && clipRect.Height > 0)
			bounds.Clip(clipRect);

		bounds.X += offset.X;
		bounds.Y += offset.Y;

		return bounds;
	}

	u32 Sprite::FillBuffer(u8* vertices, u8* uv, u32* indices, u32 vertexOffset, u32 indexOffset, u32 maxNumVerts, u32 maxNumIndices, u32 vertexStride, u32 indexStride, u32 renderElementIdx, const Vector2I& offset, const Rect2I& clipRect, bool clip) const
	{
		const auto& renderElem = mCachedRenderElements.at(renderElementIdx);

		u32 startVert = vertexOffset;
		u32 startIndex = indexOffset;

		u32 maxVertIdx = maxNumVerts;
		u32 maxIndexIdx = maxNumIndices;

		u32 numVertices = renderElem.NumQuads * 4;
		u32 numIndices = renderElem.NumQuads * 6;

		assert((startVert + numVertices) <= maxVertIdx);
		assert((startIndex + numIndices) <= maxIndexIdx);

		u8* vertDst = vertices + startVert * vertexStride;
		u8* uvDst = uv + startVert * vertexStride;

		// TODO - I'm sure this can be done in a more cache friendly way. Profile it later.
		Vector2 vecOffset((float)offset.X, (float)offset.Y);
		if(clip)
		{
			for(u32 i = 0; i < renderElem.NumQuads; i++)
			{
				u8* vecStart = vertDst;
				u8* uvStart = uvDst;
				u32 vertIdx = i * 4;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 0], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 0], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 1], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 1], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 2], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 2], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 3], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 3], sizeof(Vector2));

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
			for(u32 i = 0; i < renderElem.NumQuads; i++)
			{
				u8* vecStart = vertDst;
				u32 vertIdx = i * 4;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 0], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 0], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 1], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 1], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 2], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 2], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.Vertices[vertIdx + 3], sizeof(Vector2));
				memcpy(uvDst, &renderElem.Uvs[vertIdx + 3], sizeof(Vector2));

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
			memcpy(&indices[startIndex], renderElem.Indexes, numIndices * sizeof(u32));

		return renderElem.NumQuads;
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

		return Vector2I();
	}

	void Sprite::UpdateBounds() const
	{
		Vector2 min;
		Vector2 max;

		// Find starting point
		bool foundStartingPoint = false;
		for(auto& renderElem : mCachedRenderElements)
		{
			if(renderElem.Vertices != nullptr && renderElem.NumQuads > 0)
			{
				min = renderElem.Vertices[0];
				max = renderElem.Vertices[0];
				foundStartingPoint = true;
				break;
			}
		}

		if(!foundStartingPoint)
		{
			mBounds = Rect2I(0, 0, 0, 0);
			return;
		}

		// Calculate bounds
		for(auto& renderElem : mCachedRenderElements)
		{
			if(renderElem.Vertices != nullptr && renderElem.NumQuads > 0)
			{
				u32 vertexCount = renderElem.NumQuads * 4;

				for(u32 i = 0; i < vertexCount; i++)
				{
					min = Vector2::Min(min, renderElem.Vertices[i]);
					max = Vector2::Max(max, renderElem.Vertices[i]);
				}
			}
		}

		mBounds = Rect2I((int)min.X, (int)min.Y, (int)(max.X - min.X), (int)(max.Y - min.Y));
	}

	// This will only properly clip an array of quads
	// Vertices in the quad must be in a specific order: top left, top right, bottom left, bottom right
	// (0, 0) represents top left of the screen
	void Sprite::ClipQuadsToRect(u8* vertices, u8* uv, u32 numQuads, u32 vertStride, const Rect2I& clipRect)
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

	void Sprite::ClipTrianglesToRect(u8* vertices, u8* uv, u32 numTris, u32 vertStride, const Rect2I& clipRect, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback)
	{
		Vector<Plane> clipPlanes = {
			Plane(Vector3(1.0f, 0.0f, 0.0f), (float)clipRect.X),
			Plane(Vector3(-1.0f, 0.0f, 0.0f), (float)-(clipRect.X + (i32)clipRect.Width)),
			Plane(Vector3(0.0f, 1.0f, 0.0f), (float)clipRect.Y),
			Plane(Vector3(0.0f, -1.0f, 0.0f), (float)-(clipRect.Y + (i32)clipRect.Height))
		};

		MeshUtility::Clip2D(vertices, uv, numTris, vertStride, clipPlanes, writeCallback);
	}
} // namespace bs
