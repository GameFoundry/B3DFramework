//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DImageSprite.h"
#include "2D/B3DSpriteManager.h"
#include "GUI/B3DGUIInteractable.h"
#include "Image/B3DTexture.h"
#include "Image/B3DSpriteTexture.h"
#include "Image/B3DSpriteGlyph.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

ImageSprite::~ImageSprite()
{
	ClearMesh();
}

void ImageSprite::Update(const ImageSpriteInformation& information, u64 groupId)
{
	if(!information.Image.IsLoaded())
	{
		ClearMesh();
		return;
	}

	// Actually generate a mesh
	if(mCachedRenderElements.size() < 1)
		mCachedRenderElements.resize(1);

	const bool useScale9Grid = information.BorderLeft > 0 || information.BorderRight > 0 ||
		information.BorderTop > 0 || information.BorderBottom > 0;

	u32 quadCount = 1;
	if(useScale9Grid)
		quadCount = 9;

	// If not using scale9grid attempt to allocate a sprite image of the exact size
	const SPtr<SpriteImageAllocation> spriteAllocation = useScale9Grid
		? information.Image->GetDefaultAllocatedImageAsShared()
		: information.Image->FindOrAllocateImageToFitArea(information.Size);

	if(spriteAllocation == nullptr)
	{
		ClearMesh();
		return;
	}

	mSpriteImageAllocations = spriteAllocation;

	RenderElementData& renderElementData = mCachedRenderElements[0];
	SpriteRenderElement& renderElement = renderElementData.RenderElement;
	{
		const u32 existingQuadCount = renderElement.VertexCount / 4;
		const u32 newQuadCount = quadCount;
		if(newQuadCount != existingQuadCount)
		{
			if(renderElement.VertexPositions != nullptr) B3DDeleteMultiple(renderElement.VertexPositions, renderElement.VertexCount);
			if(renderElement.VertexUVs != nullptr) B3DDeleteMultiple(renderElement.VertexUVs, renderElement.VertexCount);
			if(renderElement.Indices != nullptr) B3DDeleteMultiple(renderElement.Indices, renderElement.IndexCount);

			renderElement.VertexCount = newQuadCount * 4;
			renderElement.IndexCount = newQuadCount * 6;
			renderElement.VertexPositions = B3DNewMultiple<Vector2>(renderElement.VertexCount);
			renderElement.VertexUVs = B3DNewMultiple<Vector2>(renderElement.VertexCount);
			renderElement.Indices = B3DNewMultiple<u32>(renderElement.IndexCount);
		}

		const HTexture& texture = spriteAllocation->GetTexture();

		SpriteMaterialInfo& materialInformation = renderElementData.MaterialInformation;
		materialInformation.GroupId = groupId;
		materialInformation.Texture = texture;
		materialInformation.SpriteImageAllocation = B3DGetRenderProxy(spriteAllocation);
		materialInformation.Tint = information.Color;
		materialInformation.AnimationStartTime = information.AnimationStartTime;

		const bool animated = information.Image->GetAnimation().FrameCount > 1;
		if(animated)
			materialInformation.SpriteImage = information.Image;

		if(B3DRTTIIsOfType<SpriteGlyph>(information.Image.Get()))
		{
			renderElement.Material = SpriteManager::Instance().GetTextMaterial();
		}
		else
		{
			renderElement.Material = SpriteManager::Instance().GetImageMaterial(
				information.Transparent ? SpriteMaterialTransparency::Alpha : SpriteMaterialTransparency::Opaque, animated);
		}

		renderElement.MaterialInformation = &renderElementData.MaterialInformation;
	}

	for(u32 quadIndex = 0; quadIndex < quadCount; quadIndex++)
	{
		renderElement.Indices[quadIndex * 6 + 0] = quadIndex * 4 + 0;
		renderElement.Indices[quadIndex * 6 + 1] = quadIndex * 4 + 1;
		renderElement.Indices[quadIndex * 6 + 2] = quadIndex * 4 + 2;
		renderElement.Indices[quadIndex * 6 + 3] = quadIndex * 4 + 1;
		renderElement.Indices[quadIndex * 6 + 4] = quadIndex * 4 + 3;
		renderElement.Indices[quadIndex * 6 + 5] = quadIndex * 4 + 2;
	}

	Vector2I offset = GetAnchorOffset(information.Anchor, information.Size.Width, information.Size.Height);
	Vector2 uvOffset = information.UvOffset;
	Vector2 uvScale = information.UvScale;

	if(useScale9Grid)
	{
		const u32 leftBorder = information.BorderLeft;
		const u32 rightBorder = information.BorderRight;
		const u32 topBorder = information.BorderTop;
		const u32 bottomBorder = information.BorderBottom;

		const float centerWidth = (float)std::max(0, information.Size.Width - (i32)leftBorder - (i32)rightBorder);
		const float centerHeight = (float)std::max(0, information.Size.Height - (i32)topBorder - (i32)bottomBorder);

		const float topCenterStart = (float)(offset.X + leftBorder);
		const float topRightStart = (float)(topCenterStart + centerWidth);

		const float middleStart = (float)(offset.Y + topBorder);
		const float bottomStart = (float)(middleStart + centerHeight);

		// Top left
		renderElement.VertexPositions[0] = Vector2((float)offset.X, (float)offset.Y);
		renderElement.VertexPositions[1] = Vector2((float)offset.X + leftBorder, (float)offset.Y);
		renderElement.VertexPositions[2] = Vector2((float)offset.X, middleStart);
		renderElement.VertexPositions[3] = Vector2((float)offset.X + leftBorder, middleStart);

		// Top center
		renderElement.VertexPositions[4] = Vector2(topCenterStart, (float)offset.Y);
		renderElement.VertexPositions[5] = Vector2(topCenterStart + centerWidth, (float)offset.Y);
		renderElement.VertexPositions[6] = Vector2(topCenterStart, middleStart);
		renderElement.VertexPositions[7] = Vector2(topCenterStart + centerWidth, middleStart);

		// Top right
		renderElement.VertexPositions[8] = Vector2(topRightStart, (float)offset.Y);
		renderElement.VertexPositions[9] = Vector2(topRightStart + rightBorder, (float)offset.Y);
		renderElement.VertexPositions[10] = Vector2(topRightStart, middleStart);
		renderElement.VertexPositions[11] = Vector2(topRightStart + rightBorder, middleStart);

		// Middle left
		renderElement.VertexPositions[12] = Vector2((float)offset.X, middleStart);
		renderElement.VertexPositions[13] = Vector2((float)offset.X + leftBorder, middleStart);
		renderElement.VertexPositions[14] = Vector2((float)offset.X, bottomStart);
		renderElement.VertexPositions[15] = Vector2((float)offset.X + leftBorder, bottomStart);

		// Middle center
		renderElement.VertexPositions[16] = Vector2(topCenterStart, middleStart);
		renderElement.VertexPositions[17] = Vector2(topCenterStart + centerWidth, middleStart);
		renderElement.VertexPositions[18] = Vector2(topCenterStart, bottomStart);
		renderElement.VertexPositions[19] = Vector2(topCenterStart + centerWidth, bottomStart);

		// Middle right
		renderElement.VertexPositions[20] = Vector2(topRightStart, middleStart);
		renderElement.VertexPositions[21] = Vector2(topRightStart + rightBorder, middleStart);
		renderElement.VertexPositions[22] = Vector2(topRightStart, bottomStart);
		renderElement.VertexPositions[23] = Vector2(topRightStart + rightBorder, bottomStart);

		// Bottom left
		renderElement.VertexPositions[24] = Vector2((float)offset.X, bottomStart);
		renderElement.VertexPositions[25] = Vector2((float)offset.X + leftBorder, bottomStart);
		renderElement.VertexPositions[26] = Vector2((float)offset.X, bottomStart + bottomBorder);
		renderElement.VertexPositions[27] = Vector2((float)offset.X + leftBorder, bottomStart + bottomBorder);

		// Bottom center
		renderElement.VertexPositions[28] = Vector2(topCenterStart, bottomStart);
		renderElement.VertexPositions[29] = Vector2(topCenterStart + centerWidth, bottomStart);
		renderElement.VertexPositions[30] = Vector2(topCenterStart, bottomStart + bottomBorder);
		renderElement.VertexPositions[31] = Vector2(topCenterStart + centerWidth, bottomStart + bottomBorder);

		// Bottom right
		renderElement.VertexPositions[32] = Vector2(topRightStart, bottomStart);
		renderElement.VertexPositions[33] = Vector2(topRightStart + rightBorder, bottomStart);
		renderElement.VertexPositions[34] = Vector2(topRightStart, bottomStart + bottomBorder);
		renderElement.VertexPositions[35] = Vector2(topRightStart + rightBorder, bottomStart + bottomBorder);

		const Size2I& imageSize = spriteAllocation->GetSize();
		const float inverseWidth = 1.0f / (float)imageSize.Width;
		const float inverseHeight = 1.0f / (float)imageSize.Height;

		const float uvLeftBorder = information.BorderLeft * inverseWidth;
		const float uvRightBorder = information.BorderRight * inverseWidth;
		const float uvTopBorder = information.BorderTop * inverseHeight;
		const float uvBottomBorder = information.BorderBottom * inverseHeight;

		const float uvCenterWidth = std::max(0.0f, uvScale.X - uvLeftBorder - uvRightBorder);
		const float uvCenterHeight = std::max(0.0f, uvScale.Y - uvTopBorder - uvBottomBorder);

		const float uvTopCenterStart = uvOffset.X + uvLeftBorder;
		const float uvTopRightStart = uvTopCenterStart + uvCenterWidth;

		const float uvMiddleStart = uvOffset.Y + uvTopBorder;
		const float uvBottomStart = uvMiddleStart + uvCenterHeight;

		// UV - Top left
		renderElement.VertexUVs[0] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvOffset.Y));
		renderElement.VertexUVs[1] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvOffset.Y));
		renderElement.VertexUVs[2] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvOffset.Y + uvTopBorder));
		renderElement.VertexUVs[3] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvOffset.Y + uvTopBorder));

		// UV - Top center
		renderElement.VertexUVs[4] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvOffset.Y));
		renderElement.VertexUVs[5] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.Y));
		renderElement.VertexUVs[6] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvOffset.Y + uvTopBorder));
		renderElement.VertexUVs[7] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.Y + uvTopBorder));

		// UV - Top right
		renderElement.VertexUVs[8] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvOffset.Y));
		renderElement.VertexUVs[9] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvOffset.Y));
		renderElement.VertexUVs[10] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvOffset.Y + uvTopBorder));
		renderElement.VertexUVs[11] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvOffset.Y + uvTopBorder));

		// UV - Middle left
		renderElement.VertexUVs[12] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvMiddleStart));
		renderElement.VertexUVs[13] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvMiddleStart));
		renderElement.VertexUVs[14] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvMiddleStart + uvCenterHeight));
		renderElement.VertexUVs[15] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvMiddleStart + uvCenterHeight));

		// UV - Middle center
		renderElement.VertexUVs[16] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvMiddleStart));
		renderElement.VertexUVs[17] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart));
		renderElement.VertexUVs[18] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvMiddleStart + uvCenterHeight));
		renderElement.VertexUVs[19] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart + uvCenterHeight));

		// UV - Middle right
		renderElement.VertexUVs[20] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvMiddleStart));
		renderElement.VertexUVs[21] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart));
		renderElement.VertexUVs[22] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvMiddleStart + uvCenterHeight));
		renderElement.VertexUVs[23] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart + uvCenterHeight));

		// UV - Bottom left
		renderElement.VertexUVs[24] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvBottomStart));
		renderElement.VertexUVs[25] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvBottomStart));
		renderElement.VertexUVs[26] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvBottomStart + uvBottomBorder));
		renderElement.VertexUVs[27] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvLeftBorder, uvBottomStart + uvBottomBorder));

		// UV - Bottom center
		renderElement.VertexUVs[28] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvBottomStart));
		renderElement.VertexUVs[29] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart));
		renderElement.VertexUVs[30] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart, uvBottomStart + uvBottomBorder));
		renderElement.VertexUVs[31] = spriteAllocation->TransformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart + uvBottomBorder));

		// UV - Bottom right
		renderElement.VertexUVs[32] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvBottomStart));
		renderElement.VertexUVs[33] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart));
		renderElement.VertexUVs[34] = spriteAllocation->TransformUV(Vector2(uvTopRightStart, uvBottomStart + uvBottomBorder));
		renderElement.VertexUVs[35] = spriteAllocation->TransformUV(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart + uvBottomBorder));
	}
	else
	{
		renderElement.VertexPositions[0] = Vector2((float)offset.X, (float)offset.Y);
		renderElement.VertexPositions[1] = Vector2((float)offset.X + (float)information.Size.Width, (float)offset.Y);
		renderElement.VertexPositions[2] = Vector2((float)offset.X, (float)offset.Y + (float)information.Size.Height);
		renderElement.VertexPositions[3] = Vector2((float)offset.X + (float)information.Size.Width, (float)offset.Y + (float)information.Size.Height);

		renderElement.VertexUVs[0] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvOffset.Y));
		renderElement.VertexUVs[1] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvScale.X, uvOffset.Y));
		renderElement.VertexUVs[2] = spriteAllocation->TransformUV(Vector2(uvOffset.X, uvOffset.Y + uvScale.Y));
		renderElement.VertexUVs[3] = spriteAllocation->TransformUV(Vector2(uvOffset.X + uvScale.X, uvOffset.Y + uvScale.Y));
	}

	UpdateBounds();
}

void ImageSprite::ClearMesh()
{
	for(auto& entry: mCachedRenderElements)
	{
		SpriteRenderElement& renderElement = entry.RenderElement;
		if(renderElement.VertexPositions != nullptr)
		{
			B3DDeleteMultiple(renderElement.VertexPositions, renderElement.VertexCount);
			renderElement.VertexPositions = nullptr;
		}

		if(renderElement.VertexUVs != nullptr)
		{
			B3DDeleteMultiple(renderElement.VertexUVs, renderElement.VertexCount);
			renderElement.VertexUVs = nullptr;
		}

		if(renderElement.Indices != nullptr)
		{
			B3DDeleteMultiple(renderElement.Indices, renderElement.IndexCount);
			renderElement.Indices = nullptr;
		}
	}

	mCachedRenderElements.clear();
	mSpriteImageAllocations = nullptr;
	UpdateBounds();
}

Vector2 ImageSprite::GetTextureUvScale(Size2I sourceSize, Size2I destinationSize, TextureScaleMode scaleMode)
{
	Vector2 uvScale = Vector2(1.0f, 1.0f);

	switch(scaleMode)
	{
	case TextureScaleMode::ScaleToFit:
		uvScale.X = (float)sourceSize.Width / (float)destinationSize.Width;
		uvScale.Y = (float)sourceSize.Height / (float)destinationSize.Height;

		if(uvScale.X > uvScale.Y)
		{
			uvScale.X = 1.0f;
			uvScale.Y = ((float)destinationSize.Height * ((float)sourceSize.Height / (float)sourceSize.Width)) / (float)destinationSize.Width;
		}
		else
		{
			uvScale.X = ((float)destinationSize.Width * ((float)sourceSize.Width / (float)sourceSize.Height)) / (float)destinationSize.Height;
			uvScale.Y = 1.0f;
		}

		break;
	case TextureScaleMode::CropToFit:
		uvScale.X = (float)sourceSize.Width / (float)destinationSize.Width;
		uvScale.Y = (float)sourceSize.Height / (float)destinationSize.Height;

		if(uvScale.X > uvScale.Y)
		{
			uvScale.X = ((float)destinationSize.Width * ((float)sourceSize.Width / (float)sourceSize.Height)) / (float)destinationSize.Height;
			uvScale.Y = 1.0f;
		}
		else
		{
			uvScale.X = 1.0f;
			uvScale.Y = ((float)destinationSize.Height * ((float)sourceSize.Height / (float)sourceSize.Width)) / (float)destinationSize.Width;
		}

		break;
	case TextureScaleMode::RepeatToFit:
		uvScale.X = (float)destinationSize.Width / (float)sourceSize.Width;
		uvScale.Y = (float)destinationSize.Height / (float)sourceSize.Height;
		break;
	case TextureScaleMode::StretchToFit:
		// Do nothing, (1.0f, 1.0f) is the default UV scale
		break;
	default:
		break;
	}

	return uvScale;
}
