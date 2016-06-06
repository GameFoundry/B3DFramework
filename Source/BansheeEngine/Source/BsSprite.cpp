//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextSprite.h"
#include "BsVector2.h"
#include "BsTexture.h"
#include "BsPlane.h"
#include "BsMeshUtility.h"

namespace BansheeEngine
{
	Sprite::Sprite()
	{ }

	Sprite::~Sprite()
	{ }

	Rect2I Sprite::getBounds(const Vector2I& offset, const Rect2I& clipRect) const 
	{
		Rect2I bounds = mBounds;

		if(clipRect.width > 0 && clipRect.height > 0)
			bounds.clip(clipRect);

		bounds.x += offset.x;
		bounds.y += offset.y;

		return bounds; 
	}

	UINT32 Sprite::getNumRenderElements() const
	{
		return (UINT32)mCachedRenderElements.size();
	}

	const SpriteMaterialInfo& Sprite::getMaterialInfo(UINT32 renderElementIdx) const
	{
		return mCachedRenderElements.at(renderElementIdx).matInfo;
	}

	UINT32 Sprite::getNumQuads(UINT32 renderElementIdx) const
	{
		return mCachedRenderElements.at(renderElementIdx).numQuads;
	}

	UINT32 Sprite::fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, UINT32 maxNumQuads, 
		UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx, const Vector2I& offset, const Rect2I& clipRect, bool clip) const
	{
		const auto& renderElem = mCachedRenderElements.at(renderElementIdx);

		UINT32 startVert = startingQuad * 4;
		UINT32 startIndex = startingQuad * 6;

		UINT32 maxVertIdx = maxNumQuads * 4;
		UINT32 maxIndexIdx = maxNumQuads * 6;

		UINT32 mNumVertices = renderElem.numQuads * 4;
		UINT32 mNumIndices = renderElem.numQuads * 6;

		assert((startVert + mNumVertices) <= maxVertIdx);
		assert((startIndex + mNumIndices) <= maxIndexIdx);

		UINT8* vertDst = vertices + startVert * vertexStride;
		UINT8* uvDst = uv + startVert * vertexStride;

		// TODO - I'm sure this can be done in a more cache friendly way. Profile it later.
		Vector2 vecOffset((float)offset.x, (float)offset.y);
		if(clip)
		{
			for(UINT32 i = 0; i < renderElem.numQuads; i++)
			{
				UINT8* vecStart = vertDst;
				UINT8* uvStart = uvDst;
				UINT32 vertIdx = i * 4;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 0], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 0], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 1], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 1], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 2], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 2], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 3], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 3], sizeof(Vector2));

				clipQuadsToRect(vecStart, uvStart, 1, vertexStride, clipRect);

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
			for(UINT32 i = 0; i < renderElem.numQuads; i++)
			{
				UINT8* vecStart = vertDst;
				UINT32 vertIdx = i * 4;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 0], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 0], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 1], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 1], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 2], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 2], sizeof(Vector2));

				vertDst += vertexStride;
				uvDst += vertexStride;

				memcpy(vertDst, &renderElem.vertices[vertIdx + 3], sizeof(Vector2));
				memcpy(uvDst, &renderElem.uvs[vertIdx + 3], sizeof(Vector2));

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
			memcpy(&indices[startIndex], renderElem.indexes, mNumIndices * sizeof(UINT32));

		return renderElem.numQuads;
	}

	Vector2I Sprite::getAnchorOffset(SpriteAnchor anchor, UINT32 width, UINT32 height)
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

	void Sprite::updateBounds() const
	{
		Vector2 min;
		Vector2 max;

		// Find starting point
		bool foundStartingPoint = false;
		for(auto& renderElem : mCachedRenderElements)
		{
			if(renderElem.vertices != nullptr && renderElem.numQuads > 0)
			{
				min = renderElem.vertices[0];
				max = renderElem.vertices[0];
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
			if(renderElem.vertices != nullptr && renderElem.numQuads > 0)
			{
				UINT32 vertexCount = renderElem.numQuads * 4;

				for(UINT32 i = 0; i < vertexCount; i++)
				{
					min = Vector2::min(min, renderElem.vertices[i]);
					max = Vector2::max(max, renderElem.vertices[i]);
				}
			}
		}

		mBounds = Rect2I((int)min.x, (int)min.y, (int)(max.x - min.x), (int)(max.y - min.y));
	}

	// This will only properly clip an array of quads
	// Vertices in the quad must be in a specific order: top left, top right, bottom left, bottom right
	// (0, 0) represents top left of the screen
	void Sprite::clipQuadsToRect(UINT8* vertices, UINT8* uv, UINT32 numQuads, UINT32 vertStride, const Rect2I& clipRect)
	{
		float left = (float)clipRect.x;
		float right = (float)clipRect.x + clipRect.width;
		float top = (float)clipRect.y;
		float bottom = (float)clipRect.y + clipRect.height;

		for(UINT32 i = 0; i < numQuads; i++)
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
			if(vecA->x >= left && vecB->x <= right &&
				vecA->y >= top && vecC->y <= bottom)
			{
				continue;
			}

			// TODO - Skip those that are 100% clipped as well

			float du = (uvB->x - uvA->x) / (vecB->x - vecA->x);
			float dv = (uvA->y - uvC->y) / (vecA->y - vecD->y);

			if(right < left)
				std::swap(left, right);

			if(bottom < top)
				std::swap(bottom, top);

			// Clip left
			float newLeft = Math::clamp(vecA->x, left, right);
			float uvLeftOffset = (newLeft - vecA->x) * du;

			// Clip right
			float newRight = Math::clamp(vecB->x, left, right);
			float uvRightOffset = (vecB->x - newRight) * du;

			// Clip top
			float newTop = Math::clamp(vecA->y, top, bottom);
			float uvTopOffset = (newTop - vecA->y) * dv;

			// Clip bottom
			float newBottom = Math::clamp(vecC->y, top, bottom);
			float uvBottomOffset = (vecC->y - newBottom) * dv;

			vecA->x = newLeft;
			vecC->x = newLeft;
			vecB->x = newRight;
			vecD->x = newRight;
			vecA->y = newTop;
			vecB->y = newTop;
			vecC->y = newBottom;
			vecD->y = newBottom;
			
			uvA->x += uvLeftOffset;
			uvC->x += uvLeftOffset;
			uvB->x -= uvRightOffset;
			uvD->x -= uvRightOffset;
			uvA->y += uvTopOffset;
			uvB->y += uvTopOffset;
			uvC->y -= uvBottomOffset;
			uvD->y -= uvBottomOffset;

			vertices += vertStride * 4;
			uv += vertStride * 4;
		}
	}

	void Sprite::clipTrianglesToRect(UINT8* vertices, UINT8* uv, UINT32 numTris, UINT32 vertStride, const Rect2I& clipRect, 
		const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback)
	{
		Vector<Plane> clipPlanes =
		{
			Plane(Vector3(1.0f, 0.0f, 0.0f), (float)clipRect.x),
			Plane(Vector3(-1.0f, 0.0f, 0.0f), (float)-(clipRect.x + clipRect.width)),
			Plane(Vector3(0, 1.0f, 0), (float)(clipRect.y + clipRect.height)),
			Plane(Vector3(0.0f, -1.0f, 0.0f), (float)-clipRect.y)
		};

		MeshUtility::clip2D(vertices, uv, numTris, vertStride, clipPlanes, writeCallback);
	}

	UINT64 SpriteMaterialInfo::generateHash() const
	{
		UINT64 textureId = 0;
		if (texture.isLoaded())
			textureId = texture->getInternalID();

		size_t hash = 0;
		hash_combine(hash, groupId);
		hash_combine(hash, type);
		hash_combine(hash, textureId);
		hash_combine(hash, tint);

		return (UINT64)hash;
	}

	bool operator==(const SpriteMaterialInfo& lhs, const SpriteMaterialInfo& rhs)
	{
		return lhs.groupId == rhs.groupId && lhs.texture == rhs.texture && lhs.type == rhs.type && lhs.tint == rhs.tint;
	}

	bool operator!=(const SpriteMaterialInfo& lhs, const SpriteMaterialInfo& rhs)
	{
		return !(lhs == rhs);
	}
}