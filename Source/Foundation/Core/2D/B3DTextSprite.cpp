//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DTextSprite.h"
#include "Math/B3DVector2.h"
#include "2D/B3DSpriteManager.h"
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"
#include "String/B3DUnicode.h"

using namespace b3d;

void TextSpriteInformation::InitializeFromStyleSheetRules(const GUIStyleSheetRules& rules)
{
	Font = rules.Font;
	FontSize = rules.FontSize;
	WordWrap = rules.WordWrap == GUIWordWrapMode::WrapWord;
	HorzAlign = rules.HorizontalTextAlignment;
	VertAlign = rules.VerticalTextAlignment;
	Color = rules.Color;
	Color.A *= rules.Opacity;
}

TextSprite::~TextSprite()
{
	ClearMesh();
}

void TextSprite::Update(const TextSpriteInformation& information, u64 groupId)
{
	B3DMarkAllocatorFrame();
	{
		const U32String utf32Text = UTF8::ToUtF32(information.Text);
		TTextGeometry<FrameAllocatorTag> textGeometry(utf32Text, information.Font, information.FontSize, (u32)information.Size.Width, (u32)information.Size.Height, information.WordWrap, information.WordBreak);

		const u32 pageCount = textGeometry.GetPageCount();

		// Free all previous memory
		for(auto& entry : mCachedRenderElements)
		{
			SpriteRenderElement& renderElement = entry.RenderElement;

			if(renderElement.VertexPositions != nullptr) mAlloc.Free(renderElement.VertexPositions);
			if(renderElement.VertexUVs != nullptr) mAlloc.Free(renderElement.VertexUVs);
			if(renderElement.Indices != nullptr) mAlloc.Free(renderElement.Indices);
		}

		mAlloc.Clear();

		// Resize cached mesh array to needed size
		if(mCachedRenderElements.size() != pageCount)
			mCachedRenderElements.resize(pageCount);

		// Actually generate a mesh
		u32 pageIndex = 0;
		for(auto& renderElementData : mCachedRenderElements)
		{
			const u32 newQuadCount = textGeometry.GetQuadCount(pageIndex);

			SpriteRenderElement& renderElement = renderElementData.RenderElement;
			renderElement.VertexCount = newQuadCount * 4;
			renderElement.IndexCount = newQuadCount * 6;
			renderElement.VertexPositions = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * renderElement.VertexCount);
			renderElement.VertexUVs = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * renderElement.VertexCount);
			renderElement.Indices = (u32*)mAlloc.Alloc(sizeof(u32) * renderElement.IndexCount);

			const HTexture& texture = textGeometry.GetTextureForPage(pageIndex);

			SpriteMaterialInfo& materialInformation = renderElementData.MaterialInformation;
			materialInformation.GroupId = groupId;
			materialInformation.Texture = texture;
			materialInformation.Tint = information.Color;
			materialInformation.AnimationStartTime = 0.0f;

			renderElement.Material = SpriteManager::Instance().GetTextMaterial();
			renderElement.MaterialInformation = &renderElementData.MaterialInformation;

			pageIndex++;
		}

		// Calc alignment and anchor offsets and set final line positions
		pageIndex = 0;
		for(; pageIndex < pageCount; pageIndex++)
		{
			SpriteRenderElement& renderElement = mCachedRenderElements[pageIndex].RenderElement;

			const u32 quadCount = renderElement.VertexCount / 4;
			BuildTextQuads(pageIndex, textGeometry, (u32)information.Size.Width, (u32)information.Size.Height, information.HorzAlign, information.VertAlign, information.Anchor, renderElement.VertexPositions, renderElement.VertexUVs, renderElement.Indices, quadCount);
		}
	}

	B3DClearAllocatorFrame();

	UpdateBounds();
}

u32 TextSprite::BuildTextQuads(u32 page, const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
{
	const u32 lineCount = textGeometry.GetLineCount();
	const u32 quadCount = textGeometry.GetQuadCount(page);

	Vector2I* const alignmentOffsets = B3DStackNew<Vector2I>(lineCount);
	GetAlignmentOffsets(textGeometry, width, height, horzAlign, vertAlign, alignmentOffsets);
	const Vector2I offset = GetAnchorOffset(anchor, width, height);

	u32 quadOffset = 0;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const TextGeometry::Line& line = textGeometry.GetLine(lineIndex);
		const u32 writtenQuadCount = line.FillBuffer(page, vertices, uv, indices, quadOffset, bufferSizeQuads);

		const Vector2I position = offset + alignmentOffsets[lineIndex];
		const u32 vertexCount = writtenQuadCount * 4;
		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			vertices[quadOffset * 4 + vertexIndex].X += (float)position.X;
			vertices[quadOffset * 4 + vertexIndex].Y += (float)position.Y;
		}

		quadOffset += writtenQuadCount;
	}

	B3DStackDelete(alignmentOffsets, lineCount);
	return quadCount;
}

u32 TextSprite::BuildTextQuads(const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
{
	const u32 lineCount = textGeometry.GetLineCount();
	const u32 pageCount = textGeometry.GetPageCount();

	Vector2I* const alignmentOffsets = B3DStackNew<Vector2I>(lineCount);
	GetAlignmentOffsets(textGeometry, width, height, horzAlign, vertAlign, alignmentOffsets);
	const Vector2I offset = GetAnchorOffset(anchor, width, height);

	u32 quadOffset = 0;

	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const TextGeometry::Line& line = textGeometry.GetLine(lineIndex);
		for(u32 pageIndex = 0; pageIndex < pageCount; pageIndex++)
		{
			const u32 writtenQuadCount = line.FillBuffer(pageIndex, vertices, uv, indices, quadOffset, bufferSizeQuads);
			const Vector2I position = offset + alignmentOffsets[lineIndex];

			const u32 vertexCount = writtenQuadCount * 4;
			for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
			{
				vertices[quadOffset * 4 + vertexIndex].X += (float)position.X;
				vertices[quadOffset * 4 + vertexIndex].Y += (float)position.Y;
			}

			quadOffset += writtenQuadCount;
		}
	}

	B3DStackDelete(alignmentOffsets, lineCount);
	return quadOffset;
}

void TextSprite::GetAlignmentOffsets(const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, Vector2I* output)
{
	const u32 lineCount = textGeometry.GetLineCount();
	float currentHeightTotal = 0.0f;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const TextGeometry::Line& line = textGeometry.GetLine(lineIndex);
		currentHeightTotal += line.GetYOffset();
	}

	// Calc vertical alignment offset
	const float verticalFreeSpace = Math::Max(0.0f, (float)height - currentHeightTotal);
	float verticalOffset = 0.0f;
	switch(vertAlign)
	{
	case GUIVerticalTextAlignment::Top:
		verticalOffset = 0;
		break;
	case GUIVerticalTextAlignment::Bottom:
		verticalOffset = Math::Max(0.0f, verticalFreeSpace);
		break;
	case GUIVerticalTextAlignment::Middle:
		verticalOffset = Math::Max(0.0f, verticalFreeSpace) / 2.0f;
		break;
	}

	// Calc horizontal alignment offset
	float currentY = 0.0f;
	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const TextGeometry::Line& line = textGeometry.GetLine(lineIndex);

		float horizontalOffset = 0.0f;
		switch(horzAlign)
		{
		case GUIHorizontalTextAlignment::Left:
			horizontalOffset = 0;
			break;
		case GUIHorizontalTextAlignment::Right:
			horizontalOffset = Math::Max(0.0f, (float)width - line.GetWidth());
			break;
		case GUIHorizontalTextAlignment::Center:
			horizontalOffset = Math::Max(0.0f, (float)width - line.GetWidth()) / 2.0f;
			break;
		}

		output[lineIndex] = Vector2I(Math::RoundToI32(horizontalOffset), Math::RoundToI32(verticalOffset + currentY));
		currentY += line.GetYOffset();
	}
}

void TextSprite::ClearMesh()
{
	for(auto& entry : mCachedRenderElements)
	{
		SpriteRenderElement& renderElement = entry.RenderElement;
		if(renderElement.VertexPositions != nullptr)
		{
			mAlloc.Free(renderElement.VertexPositions);
			renderElement.VertexPositions = nullptr;
		}

		if(renderElement.VertexUVs != nullptr)
		{
			mAlloc.Free(renderElement.VertexUVs);
			renderElement.VertexUVs = nullptr;
		}

		if(renderElement.Indices != nullptr)
		{
			mAlloc.Free(renderElement.Indices);
			renderElement.Indices = nullptr;
		}
	}

	mCachedRenderElements.clear();
	mAlloc.Clear();

	UpdateBounds();
}
