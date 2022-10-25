//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsImageSprite.h"
#include "2D/BsSpriteManager.h"
#include "Image/BsTexture.h"
#include "Image/BsSpriteTexture.h"

namespace bs
{
	ImageSprite::~ImageSprite()
	{
		ClearMesh();
	}

	void ImageSprite::Update(const IMAGE_SPRITE_DESC& desc, u64 groupId)
	{
		if(!SpriteTexture::CheckIsLoaded(desc.Texture))
		{
			ClearMesh();
			return;
		}

		// Actually generate a mesh
		if(mCachedRenderElements.size() < 1)
			mCachedRenderElements.resize(1);

		bool useScale9Grid = desc.BorderLeft > 0 || desc.BorderRight > 0 ||
			desc.BorderTop > 0 || desc.BorderBottom > 0;

		u32 numQuads = 1;
		if(useScale9Grid)
			numQuads = 9;

		SpriteRenderElementData& renderElem = mCachedRenderElements[0];
		{
			u32 newNumQuads = numQuads;
			if(newNumQuads != renderElem.NumQuads)
			{
				u32 oldVertexCount = renderElem.NumQuads * 4;
				u32 oldIndexCount = renderElem.NumQuads * 6;

				if(renderElem.Vertices != nullptr) bs_deleteN(renderElem.Vertices, oldVertexCount);
				if(renderElem.Uvs != nullptr) bs_deleteN(renderElem.Uvs, oldVertexCount);
				if(renderElem.Indexes != nullptr) bs_deleteN(renderElem.Indexes, oldIndexCount);

				renderElem.Vertices = bs_newN<Vector2>(newNumQuads * 4);
				renderElem.Uvs = bs_newN<Vector2>(newNumQuads * 4);
				renderElem.Indexes = bs_newN<u32>(newNumQuads * 6);
				renderElem.NumQuads = newNumQuads;
			}

			const HTexture& tex = desc.Texture->GetTexture();

			SpriteMaterialInfo& matInfo = renderElem.MatInfo;
			matInfo.GroupId = groupId;
			matInfo.Texture = tex;
			matInfo.Tint = desc.Color;
			matInfo.AnimationStartTime = desc.AnimationStartTime;

			bool animated = desc.Texture->GetAnimation().Count > 1;
			if(animated)
				matInfo.SpriteTexture = desc.Texture;

			renderElem.Material = SpriteManager::Instance().GetImageMaterial(
				desc.Transparent ? SpriteMaterialTransparency::Alpha : SpriteMaterialTransparency::Opaque, animated);
		}

		for(u32 i = 0; i < numQuads; i++)
		{
			renderElem.Indexes[i * 6 + 0] = i * 4 + 0;
			renderElem.Indexes[i * 6 + 1] = i * 4 + 1;
			renderElem.Indexes[i * 6 + 2] = i * 4 + 2;
			renderElem.Indexes[i * 6 + 3] = i * 4 + 1;
			renderElem.Indexes[i * 6 + 4] = i * 4 + 3;
			renderElem.Indexes[i * 6 + 5] = i * 4 + 2;
		}

		Vector2I offset = GetAnchorOffset(desc.Anchor, desc.Width, desc.Height);
		Vector2 uvOffset = desc.UvOffset;
		Vector2 uvScale = desc.UvScale;

		if(useScale9Grid)
		{
			u32 leftBorder = desc.BorderLeft;
			u32 rightBorder = desc.BorderRight;
			u32 topBorder = desc.BorderTop;
			u32 bottomBorder = desc.BorderBottom;

			float centerWidth = (float)std::max((i32)0, (i32)desc.Width - (i32)leftBorder - (i32)rightBorder);
			float centerHeight = (float)std::max((i32)0, (i32)desc.Height - (i32)topBorder - (i32)bottomBorder);

			float topCenterStart = (float)(offset.X + leftBorder);
			float topRightStart = (float)(topCenterStart + centerWidth);

			float middleStart = (float)(offset.Y + topBorder);
			float bottomStart = (float)(middleStart + centerHeight);

			// Top left
			renderElem.Vertices[0] = Vector2((float)offset.X, (float)offset.Y);
			renderElem.Vertices[1] = Vector2((float)offset.X + leftBorder, (float)offset.Y);
			renderElem.Vertices[2] = Vector2((float)offset.X, middleStart);
			renderElem.Vertices[3] = Vector2((float)offset.X + leftBorder, middleStart);

			// Top center
			renderElem.Vertices[4] = Vector2(topCenterStart, (float)offset.Y);
			renderElem.Vertices[5] = Vector2(topCenterStart + centerWidth, (float)offset.Y);
			renderElem.Vertices[6] = Vector2(topCenterStart, middleStart);
			renderElem.Vertices[7] = Vector2(topCenterStart + centerWidth, middleStart);

			// Top right
			renderElem.Vertices[8] = Vector2(topRightStart, (float)offset.Y);
			renderElem.Vertices[9] = Vector2(topRightStart + rightBorder, (float)offset.Y);
			renderElem.Vertices[10] = Vector2(topRightStart, middleStart);
			renderElem.Vertices[11] = Vector2(topRightStart + rightBorder, middleStart);

			// Middle left
			renderElem.Vertices[12] = Vector2((float)offset.X, middleStart);
			renderElem.Vertices[13] = Vector2((float)offset.X + leftBorder, middleStart);
			renderElem.Vertices[14] = Vector2((float)offset.X, bottomStart);
			renderElem.Vertices[15] = Vector2((float)offset.X + leftBorder, bottomStart);

			// Middle center
			renderElem.Vertices[16] = Vector2(topCenterStart, middleStart);
			renderElem.Vertices[17] = Vector2(topCenterStart + centerWidth, middleStart);
			renderElem.Vertices[18] = Vector2(topCenterStart, bottomStart);
			renderElem.Vertices[19] = Vector2(topCenterStart + centerWidth, bottomStart);

			// Middle right
			renderElem.Vertices[20] = Vector2(topRightStart, middleStart);
			renderElem.Vertices[21] = Vector2(topRightStart + rightBorder, middleStart);
			renderElem.Vertices[22] = Vector2(topRightStart, bottomStart);
			renderElem.Vertices[23] = Vector2(topRightStart + rightBorder, bottomStart);

			// Bottom left
			renderElem.Vertices[24] = Vector2((float)offset.X, bottomStart);
			renderElem.Vertices[25] = Vector2((float)offset.X + leftBorder, bottomStart);
			renderElem.Vertices[26] = Vector2((float)offset.X, bottomStart + bottomBorder);
			renderElem.Vertices[27] = Vector2((float)offset.X + leftBorder, bottomStart + bottomBorder);

			// Bottom center
			renderElem.Vertices[28] = Vector2(topCenterStart, bottomStart);
			renderElem.Vertices[29] = Vector2(topCenterStart + centerWidth, bottomStart);
			renderElem.Vertices[30] = Vector2(topCenterStart, bottomStart + bottomBorder);
			renderElem.Vertices[31] = Vector2(topCenterStart + centerWidth, bottomStart + bottomBorder);

			// Bottom right
			renderElem.Vertices[32] = Vector2(topRightStart, bottomStart);
			renderElem.Vertices[33] = Vector2(topRightStart + rightBorder, bottomStart);
			renderElem.Vertices[34] = Vector2(topRightStart, bottomStart + bottomBorder);
			renderElem.Vertices[35] = Vector2(topRightStart + rightBorder, bottomStart + bottomBorder);

			float invWidth = 1.0f / (float)desc.Texture->GetTexture()->GetProperties().GetWidth();
			float invHeight = 1.0f / (float)desc.Texture->GetTexture()->GetProperties().GetHeight();

			float uvLeftBorder = desc.BorderLeft * invWidth;
			float uvRightBorder = desc.BorderRight * invWidth;
			float uvTopBorder = desc.BorderTop * invHeight;
			float uvBottomBorder = desc.BorderBottom * invHeight;

			float uvCenterWidth = std::max(0.0f, uvScale.X - uvLeftBorder - uvRightBorder);
			float uvCenterHeight = std::max(0.0f, uvScale.Y - uvTopBorder - uvBottomBorder);

			float uvTopCenterStart = uvOffset.X + uvLeftBorder;
			float uvTopRightStart = uvTopCenterStart + uvCenterWidth;

			float uvMiddleStart = uvOffset.Y + uvTopBorder;
			float uvBottomStart = uvMiddleStart + uvCenterHeight;

			// UV - Top left
			renderElem.Uvs[0] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvOffset.Y));
			renderElem.Uvs[1] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvOffset.Y));
			renderElem.Uvs[2] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvOffset.Y + uvTopBorder));
			renderElem.Uvs[3] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvOffset.Y + uvTopBorder));

			// UV - Top center
			renderElem.Uvs[4] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvOffset.Y));
			renderElem.Uvs[5] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.Y));
			renderElem.Uvs[6] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvOffset.Y + uvTopBorder));
			renderElem.Uvs[7] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.Y + uvTopBorder));

			// UV - Top right
			renderElem.Uvs[8] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvOffset.Y));
			renderElem.Uvs[9] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvOffset.Y));
			renderElem.Uvs[10] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvOffset.Y + uvTopBorder));
			renderElem.Uvs[11] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvOffset.Y + uvTopBorder));

			// UV - Middle left
			renderElem.Uvs[12] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvMiddleStart));
			renderElem.Uvs[13] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvMiddleStart));
			renderElem.Uvs[14] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvMiddleStart + uvCenterHeight));
			renderElem.Uvs[15] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvMiddleStart + uvCenterHeight));

			// UV - Middle center
			renderElem.Uvs[16] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvMiddleStart));
			renderElem.Uvs[17] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart));
			renderElem.Uvs[18] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvMiddleStart + uvCenterHeight));
			renderElem.Uvs[19] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart + uvCenterHeight));

			// UV - Middle right
			renderElem.Uvs[20] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvMiddleStart));
			renderElem.Uvs[21] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart));
			renderElem.Uvs[22] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvMiddleStart + uvCenterHeight));
			renderElem.Uvs[23] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart + uvCenterHeight));

			// UV - Bottom left
			renderElem.Uvs[24] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvBottomStart));
			renderElem.Uvs[25] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvBottomStart));
			renderElem.Uvs[26] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvBottomStart + uvBottomBorder));
			renderElem.Uvs[27] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvLeftBorder, uvBottomStart + uvBottomBorder));

			// UV - Bottom center
			renderElem.Uvs[28] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvBottomStart));
			renderElem.Uvs[29] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart));
			renderElem.Uvs[30] = desc.Texture->TransformUv(Vector2(uvTopCenterStart, uvBottomStart + uvBottomBorder));
			renderElem.Uvs[31] = desc.Texture->TransformUv(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart + uvBottomBorder));

			// UV - Bottom right
			renderElem.Uvs[32] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvBottomStart));
			renderElem.Uvs[33] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart));
			renderElem.Uvs[34] = desc.Texture->TransformUv(Vector2(uvTopRightStart, uvBottomStart + uvBottomBorder));
			renderElem.Uvs[35] = desc.Texture->TransformUv(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart + uvBottomBorder));
		}
		else
		{
			renderElem.Vertices[0] = Vector2((float)offset.X, (float)offset.Y);
			renderElem.Vertices[1] = Vector2((float)offset.X + desc.Width, (float)offset.Y);
			renderElem.Vertices[2] = Vector2((float)offset.X, (float)offset.Y + desc.Height);
			renderElem.Vertices[3] = Vector2((float)offset.X + desc.Width, (float)offset.Y + desc.Height);

			renderElem.Uvs[0] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvOffset.Y));
			renderElem.Uvs[1] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvScale.X, uvOffset.Y));
			renderElem.Uvs[2] = desc.Texture->TransformUv(Vector2(uvOffset.X, uvOffset.Y + uvScale.Y));
			renderElem.Uvs[3] = desc.Texture->TransformUv(Vector2(uvOffset.X + uvScale.X, uvOffset.Y + uvScale.Y));
		}

		UpdateBounds();
	}

	void ImageSprite::ClearMesh()
	{
		for(auto& renderElem : mCachedRenderElements)
		{
			u32 vertexCount = renderElem.NumQuads * 4;
			u32 indexCount = renderElem.NumQuads * 6;

			if(renderElem.Vertices != nullptr)
			{
				bs_deleteN(renderElem.Vertices, vertexCount);
				renderElem.Vertices = nullptr;
			}

			if(renderElem.Uvs != nullptr)
			{
				bs_deleteN(renderElem.Uvs, vertexCount);
				renderElem.Uvs = nullptr;
			}

			if(renderElem.Indexes != nullptr)
			{
				bs_deleteN(renderElem.Indexes, indexCount);
				renderElem.Indexes = nullptr;
			}
		}

		mCachedRenderElements.clear();
		UpdateBounds();
	}

	Vector2 ImageSprite::GetTextureUvScale(Vector2I sourceSize, Vector2I destSize, TextureScaleMode scaleMode)
	{
		Vector2 uvScale = Vector2(1.0f, 1.0f);

		switch(scaleMode)
		{
		case TextureScaleMode::ScaleToFit:
			uvScale.X = sourceSize.X / (float)destSize.X;
			uvScale.Y = sourceSize.Y / (float)destSize.Y;

			if(uvScale.X > uvScale.Y)
			{
				uvScale.X = 1.0f;
				uvScale.Y = (destSize.Y * (sourceSize.Y / (float)sourceSize.X)) / destSize.X;
			}
			else
			{
				uvScale.X = (destSize.X * (sourceSize.X / (float)sourceSize.Y)) / destSize.Y;
				uvScale.Y = 1.0f;
			}

			break;
		case TextureScaleMode::CropToFit:
			uvScale.X = sourceSize.X / (float)destSize.X;
			uvScale.Y = sourceSize.Y / (float)destSize.Y;

			if(uvScale.X > uvScale.Y)
			{
				uvScale.X = (destSize.X * (sourceSize.X / (float)sourceSize.Y)) / destSize.Y;
				uvScale.Y = 1.0f;
			}
			else
			{
				uvScale.X = 1.0f;
				uvScale.Y = (destSize.Y * (sourceSize.Y / (float)sourceSize.X)) / destSize.X;
			}

			break;
		case TextureScaleMode::RepeatToFit:
			uvScale.X = destSize.X / (float)sourceSize.X;
			uvScale.Y = destSize.Y / (float)sourceSize.Y;
			break;
		case TextureScaleMode::StretchToFit:
			// Do nothing, (1.0f, 1.0f) is the default UV scale
			break;
		default:
			break;
		}

		return uvScale;
	}
} // namespace bs
