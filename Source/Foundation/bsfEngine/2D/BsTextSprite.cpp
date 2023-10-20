//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsTextSprite.h"
#include "Text/BsTextData.h"
#include "Math/BsVector2.h"
#include "2D/BsSpriteManager.h"
#include "String/BsUnicode.h"

using namespace bs;

TextSprite::~TextSprite()
{
	ClearMesh();
}

void TextSprite::Update(const TextSpriteInformation& information, u64 groupId)
{
	B3DMarkAllocatorFrame();
	{
		const U32String utf32text = UTF8::ToUtF32(information.Text);
		TextData<FrameAllocatorTag> textData(utf32text, information.Font, information.FontSize, information.Width, information.Height, information.WordWrap, information.WordBreak);

		const u32 pageCount = textData.GetNumPages();

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
			const u32 newQuadCount = textData.GetNumQuadsForPage(pageIndex);

			SpriteRenderElement& renderElement = renderElementData.RenderElement;
			renderElement.VertexCount = newQuadCount * 4;
			renderElement.IndexCount = newQuadCount * 6;
			renderElement.VertexPositions = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * renderElement.VertexCount);
			renderElement.VertexUVs = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * renderElement.VertexCount);
			renderElement.Indices = (u32*)mAlloc.Alloc(sizeof(u32) * renderElement.IndexCount);

			const HTexture& tex = textData.GetTextureForPage(pageIndex);

			SpriteMaterialInfo& materialInformation = renderElementData.MaterialInformation;
			materialInformation.GroupId = groupId;
			materialInformation.Texture = tex;
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
			GenTextQuads(pageIndex, textData, information.Width, information.Height, information.HorzAlign, information.VertAlign, information.Anchor, renderElement.VertexPositions, renderElement.VertexUVs, renderElement.Indices, quadCount);
		}
	}

	B3DClearAllocatorFrame();

	UpdateBounds();
}

u32 TextSprite::GenTextQuads(u32 page, const TextDataBase& textData, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
{
	u32 numLines = textData.GetNumLines();
	u32 newNumQuads = textData.GetNumQuadsForPage(page);

	Vector2I* alignmentOffsets = B3DStackNew<Vector2I>(numLines);
	GetAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
	Vector2I offset = GetAnchorOffset(anchor, width, height);

	u32 quadOffset = 0;
	for(u32 i = 0; i < numLines; i++)
	{
		const TextDataBase::TextLine& line = textData.GetLine(i);
		u32 writtenQuads = line.FillBuffer(page, vertices, uv, indices, quadOffset, bufferSizeQuads);

		Vector2I position = offset + alignmentOffsets[i];
		u32 numVertices = writtenQuads * 4;
		for(u32 j = 0; j < numVertices; j++)
		{
			vertices[quadOffset * 4 + j].X += (float)position.X;
			vertices[quadOffset * 4 + j].Y += (float)position.Y;
		}

		quadOffset += writtenQuads;
	}

	B3DStackDelete(alignmentOffsets, numLines);
	return newNumQuads;
}

u32 TextSprite::GenTextQuads(const TextDataBase& textData, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
{
	u32 numLines = textData.GetNumLines();
	u32 numPages = textData.GetNumPages();

	Vector2I* alignmentOffsets = B3DStackNew<Vector2I>(numLines);
	GetAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
	Vector2I offset = GetAnchorOffset(anchor, width, height);

	u32 quadOffset = 0;

	for(u32 i = 0; i < numLines; i++)
	{
		const TextDataBase::TextLine& line = textData.GetLine(i);
		for(u32 j = 0; j < numPages; j++)
		{
			u32 writtenQuads = line.FillBuffer(j, vertices, uv, indices, quadOffset, bufferSizeQuads);

			Vector2I position = offset + alignmentOffsets[i];

			u32 numVertices = writtenQuads * 4;
			for(u32 k = 0; k < numVertices; k++)
			{
				vertices[quadOffset * 4 + k].X += (float)position.X;
				vertices[quadOffset * 4 + k].Y += (float)position.Y;
			}

			quadOffset += writtenQuads;
		}
	}

	B3DStackDelete(alignmentOffsets, numLines);
	return quadOffset;
}

void TextSprite::GetAlignmentOffsets(const TextDataBase& textData, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, Vector2I* output)
{
	u32 numLines = textData.GetNumLines();
	u32 curHeight = 0;
	for(u32 i = 0; i < numLines; i++)
	{
		const TextDataBase::TextLine& line = textData.GetLine(i);
		curHeight += line.GetYOffset();
	}

	// Calc vertical alignment offset
	u32 vertDiff = (u32)std::max(0, (i32)height - (i32)curHeight);
	u32 vertOffset = 0;
	switch(vertAlign)
	{
	case GUIVerticalTextAlignment::Top:
		vertOffset = 0;
		break;
	case GUIVerticalTextAlignment::Bottom:
		vertOffset = (u32)std::max(0, (i32)vertDiff);
		break;
	case GUIVerticalTextAlignment::Middle:
		vertOffset = (u32)std::max(0, (i32)vertDiff) / 2;
		break;
	}

	// Calc horizontal alignment offset
	u32 curY = 0;
	for(u32 i = 0; i < numLines; i++)
	{
		const TextDataBase::TextLine& line = textData.GetLine(i);

		u32 horzOffset = 0;
		switch(horzAlign)
		{
		case GUIHorizontalTextAlignment::Left:
			horzOffset = 0;
			break;
		case GUIHorizontalTextAlignment::Right:
			horzOffset = (u32)std::max(0, (i32)(width - line.GetWidth()));
			break;
		case GUIHorizontalTextAlignment::Center:
			horzOffset = (u32)std::max(0, (i32)(width - line.GetWidth())) / 2;
			break;
		}

		output[i] = Vector2I(horzOffset, vertOffset + curY);
		curY += line.GetYOffset();
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
