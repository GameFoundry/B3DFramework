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

		u32 numPages = textData.GetNumPages();

		// Free all previous memory
		for(auto& cachedElem : mCachedRenderElements)
		{
			if(cachedElem.VertexPositions != nullptr) mAlloc.Free(cachedElem.VertexPositions);
			if(cachedElem.VertexUVs != nullptr) mAlloc.Free(cachedElem.VertexUVs);
			if(cachedElem.Indices != nullptr) mAlloc.Free(cachedElem.Indices);
		}

		mAlloc.Clear();

		// Resize cached mesh array to needed size
		if(mCachedRenderElements.size() != numPages)
			mCachedRenderElements.resize(numPages);

		// Actually generate a mesh
		u32 texPage = 0;
		for(auto& cachedElem : mCachedRenderElements)
		{
			u32 newNumQuads = textData.GetNumQuadsForPage(texPage);

			cachedElem.VertexPositions = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
			cachedElem.VertexUVs = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
			cachedElem.Indices = (u32*)mAlloc.Alloc(sizeof(u32) * newNumQuads * 6);
			cachedElem.QuadCount = newNumQuads;

			const HTexture& tex = textData.GetTextureForPage(texPage);

			SpriteMaterialInfo& matInfo = cachedElem.MaterialInformation;
			matInfo.GroupId = groupId;
			matInfo.Texture = tex;
			matInfo.Tint = information.Color;
			matInfo.AnimationStartTime = 0.0f;

			cachedElem.Material = SpriteManager::Instance().GetTextMaterial();

			texPage++;
		}

		// Calc alignment and anchor offsets and set final line positions
		for(u32 j = 0; j < numPages; j++)
		{
			SpriteRenderElementData& renderElem = mCachedRenderElements[j];

			GenTextQuads(j, textData, information.Width, information.Height, information.HorzAlign, information.VertAlign, information.Anchor, renderElem.VertexPositions, renderElem.VertexUVs, renderElem.Indices, renderElem.QuadCount);
		}
	}

	B3DClearAllocatorFrame();

	UpdateBounds();
}

u32 TextSprite::GenTextQuads(u32 page, const TextDataBase& textData, u32 width, u32 height, TextHorizontalAlignment horzAlign, TextVerticalAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
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

u32 TextSprite::GenTextQuads(const TextDataBase& textData, u32 width, u32 height, TextHorizontalAlignment horzAlign, TextVerticalAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
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

void TextSprite::GetAlignmentOffsets(const TextDataBase& textData, u32 width, u32 height, TextHorizontalAlignment horzAlign, TextVerticalAlignment vertAlign, Vector2I* output)
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
	case TVA_Top:
		vertOffset = 0;
		break;
	case TVA_Bottom:
		vertOffset = (u32)std::max(0, (i32)vertDiff);
		break;
	case TVA_Center:
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
		case THA_Left:
			horzOffset = 0;
			break;
		case THA_Right:
			horzOffset = (u32)std::max(0, (i32)(width - line.GetWidth()));
			break;
		case THA_Center:
			horzOffset = (u32)std::max(0, (i32)(width - line.GetWidth())) / 2;
			break;
		}

		output[i] = Vector2I(horzOffset, vertOffset + curY);
		curY += line.GetYOffset();
	}
}

void TextSprite::ClearMesh()
{
	for(auto& renderElem : mCachedRenderElements)
	{
		if(renderElem.VertexPositions != nullptr)
		{
			mAlloc.Free(renderElem.VertexPositions);
			renderElem.VertexPositions = nullptr;
		}

		if(renderElem.VertexUVs != nullptr)
		{
			mAlloc.Free(renderElem.VertexUVs);
			renderElem.VertexUVs = nullptr;
		}

		if(renderElem.Indices != nullptr)
		{
			mAlloc.Free(renderElem.Indices);
			renderElem.Indices = nullptr;
		}
	}

	mCachedRenderElements.clear();
	mAlloc.Clear();

	UpdateBounds();
}
