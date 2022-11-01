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

void TextSprite::Update(const TEXT_SPRITE_DESC& desc, u64 groupId)
{
	B3DMarkAllocatorFrame();
	{
		const U32String utf32text = UTF8::ToUtF32(desc.Text);
		TextData<FrameAlloc> textData(utf32text, desc.Font, desc.FontSize, desc.Width, desc.Height, desc.WordWrap, desc.WordBreak);

		u32 numPages = textData.GetNumPages();

		// Free all previous memory
		for(auto& cachedElem : mCachedRenderElements)
		{
			if(cachedElem.Vertices != nullptr) mAlloc.Free(cachedElem.Vertices);
			if(cachedElem.Uvs != nullptr) mAlloc.Free(cachedElem.Uvs);
			if(cachedElem.Indexes != nullptr) mAlloc.Free(cachedElem.Indexes);
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

			cachedElem.Vertices = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
			cachedElem.Uvs = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
			cachedElem.Indexes = (u32*)mAlloc.Alloc(sizeof(u32) * newNumQuads * 6);
			cachedElem.NumQuads = newNumQuads;

			const HTexture& tex = textData.GetTextureForPage(texPage);

			SpriteMaterialInfo& matInfo = cachedElem.MatInfo;
			matInfo.GroupId = groupId;
			matInfo.Texture = tex;
			matInfo.Tint = desc.Color;
			matInfo.AnimationStartTime = 0.0f;

			cachedElem.Material = SpriteManager::Instance().GetTextMaterial();

			texPage++;
		}

		// Calc alignment and anchor offsets and set final line positions
		for(u32 j = 0; j < numPages; j++)
		{
			SpriteRenderElementData& renderElem = mCachedRenderElements[j];

			GenTextQuads(j, textData, desc.Width, desc.Height, desc.HorzAlign, desc.VertAlign, desc.Anchor, renderElem.Vertices, renderElem.Uvs, renderElem.Indexes, renderElem.NumQuads);
		}
	}

	B3DClearAllocatorFrame();

	UpdateBounds();
}

u32 TextSprite::GenTextQuads(u32 page, const TextDataBase& textData, u32 width, u32 height, TextHorzAlign horzAlign, TextVertAlign vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
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

u32 TextSprite::GenTextQuads(const TextDataBase& textData, u32 width, u32 height, TextHorzAlign horzAlign, TextVertAlign vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads)
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

void TextSprite::GetAlignmentOffsets(const TextDataBase& textData, u32 width, u32 height, TextHorzAlign horzAlign, TextVertAlign vertAlign, Vector2I* output)
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
		if(renderElem.Vertices != nullptr)
		{
			mAlloc.Free(renderElem.Vertices);
			renderElem.Vertices = nullptr;
		}

		if(renderElem.Uvs != nullptr)
		{
			mAlloc.Free(renderElem.Uvs);
			renderElem.Uvs = nullptr;
		}

		if(renderElem.Indexes != nullptr)
		{
			mAlloc.Free(renderElem.Indexes);
			renderElem.Indexes = nullptr;
		}
	}

	mCachedRenderElements.clear();
	mAlloc.Clear();

	UpdateBounds();
}
