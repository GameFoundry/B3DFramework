//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsTextSprite.h"
#include "Text/BsTextData.h"
#include "Math/BsVector2.h"
#include "2D/BsSpriteManager.h"
#include "String/BsUnicode.h"

namespace bs
{
	TextSprite::~TextSprite()
	{
		clearMesh();
	}

	void TextSprite::Update(const TEXT_SPRITE_DESC& desc, UINT64 groupId)
	{
		bs_frame_mark();
		{
			const U32String utf32text = UTF8::toUTF32(desc.text);
			TextData<FrameAlloc> textData(utf32text, desc.font, desc.fontSize, desc.width, desc.height, desc.wordWrap,
				desc.wordBreak);

			UINT32 numPages = textData.GetNumPages();

			// Free all previous memory
			for (auto& cachedElem : mCachedRenderElements)
			{
				if (cachedElem.vertices != nullptr) mAlloc.Free(cachedElem.vertices);
				if (cachedElem.uvs != nullptr) mAlloc.Free(cachedElem.uvs);
				if (cachedElem.indexes != nullptr) mAlloc.Free(cachedElem.indexes);
			}

			mAlloc.Clear();

			// Resize cached mesh array to needed size
			if (mCachedRenderElements.size() != numPages)
				mCachedRenderElements.Resize(numPages);

			// Actually generate a mesh
			UINT32 texPage = 0;
			for (auto& cachedElem : mCachedRenderElements)
			{
				UINT32 newNumQuads = textData.GetNumQuadsForPage(texPage);

				cachedElem.vertices = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
				cachedElem.uvs = (Vector2*)mAlloc.Alloc(sizeof(Vector2) * newNumQuads * 4);
				cachedElem.indexes = (UINT32*)mAlloc.Alloc(sizeof(UINT32) * newNumQuads * 6);
				cachedElem.numQuads = newNumQuads;

				const HTexture& tex = textData.GetTextureForPage(texPage);

				SpriteMaterialInfo& matInfo = cachedElem.matInfo;
				matInfo.groupId = groupId;
				matInfo.texture = tex;
				matInfo.tint = desc.color;
				matInfo.animationStartTime = 0.0f;

				cachedElem.material = SpriteManager::instance().GetTextMaterial();

				texPage++;
			}

			// Calc alignment and anchor offsets and set final line positions
			for (UINT32 j = 0; j < numPages; j++)
			{
				SpriteRenderElementData& renderElem = mCachedRenderElements[j];

				genTextQuads(j, textData, desc.width, desc.height, desc.horzAlign, desc.vertAlign, desc.anchor,
					renderElem.vertices, renderElem.uvs, renderElem.indexes, renderElem.numQuads);
			}
		}

		bs_frame_clear();

		updateBounds();
	}

	UINT32 TextSprite::genTextQuads(UINT32 page, const TextDataBase& textData, UINT32 width, UINT32 height,
		TextHorzAlign horzAlign, TextVertAlign vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, UINT32* indices, UINT32 bufferSizeQuads)
	{
		UINT32 numLines = textData.GetNumLines();
		UINT32 newNumQuads = textData.GetNumQuadsForPage(page);

		Vector2I* alignmentOffsets = bs_stack_new<Vector2I>(numLines);
		getAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
		Vector2I offset = getAnchorOffset(anchor, width, height);

		UINT32 quadOffset = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.GetLine(i);
			UINT32 writtenQuads = line.FillBuffer(page, vertices, uv, indices, quadOffset, bufferSizeQuads);

			Vector2I position = offset + alignmentOffsets[i];
			UINT32 numVertices = writtenQuads * 4;
			for(UINT32 j = 0; j < numVertices; j++)
			{
				vertices[quadOffset * 4 + j].x += (float)position.x;
				vertices[quadOffset * 4 + j].y += (float)position.y;
			}

			quadOffset += writtenQuads;
		}

		bs_stack_delete(alignmentOffsets, numLines);
		return newNumQuads;
	}


	UINT32 TextSprite::genTextQuads(const TextDataBase& textData, UINT32 width, UINT32 height,
		TextHorzAlign horzAlign, TextVertAlign vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, UINT32* indices, UINT32 bufferSizeQuads)
	{
		UINT32 numLines = textData.GetNumLines();
		UINT32 numPages = textData.GetNumPages();

		Vector2I* alignmentOffsets = bs_stack_new<Vector2I>(numLines);
		getAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
		Vector2I offset = getAnchorOffset(anchor, width, height);

		UINT32 quadOffset = 0;
		
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.GetLine(i);
			for(UINT32 j = 0; j < numPages; j++)
			{
				UINT32 writtenQuads = line.FillBuffer(j, vertices, uv, indices, quadOffset, bufferSizeQuads);

				Vector2I position = offset + alignmentOffsets[i];

				UINT32 numVertices = writtenQuads * 4;
				for(UINT32 k = 0; k < numVertices; k++)
				{
					vertices[quadOffset * 4 + k].x += (float)position.x;
					vertices[quadOffset * 4 + k].y += (float)position.y;
				}

				quadOffset += writtenQuads;
			}
		}

		bs_stack_delete(alignmentOffsets, numLines);
		return quadOffset;
	}

	void TextSprite::getAlignmentOffsets(const TextDataBase& textData,
		UINT32 width, UINT32 height, TextHorzAlign horzAlign, TextVertAlign vertAlign, Vector2I* output)
	{
		UINT32 numLines = textData.GetNumLines();
		UINT32 curHeight = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.GetLine(i);
			curHeight += line.GetYOffset();
		}

		// Calc vertical alignment offset
		UINT32 vertDiff = (UINT32)std::max(0, (INT32)height - (INT32)curHeight);
		UINT32 vertOffset = 0;
		switch(vertAlign)
		{
		case TVA_Top:
			vertOffset = 0;
			break;
		case TVA_Bottom:
			vertOffset = (UINT32)std::max(0, (INT32)vertDiff);
			break;
		case TVA_Center:
			vertOffset = (UINT32)std::max(0, (INT32)vertDiff) / 2;
			break;
		}

		// Calc horizontal alignment offset
		UINT32 curY = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.GetLine(i);

			UINT32 horzOffset = 0;
			switch(horzAlign)
			{
			case THA_Left:
				horzOffset = 0;
				break;
			case THA_Right:
				horzOffset = (UINT32)std::max(0, (INT32)(width - line.GetWidth()));
				break;
			case THA_Center:
				horzOffset = (UINT32)std::max(0, (INT32)(width - line.GetWidth())) / 2;
				break;
			}

			output[i] = Vector2I(horzOffset, vertOffset + curY);
			curY += line.GetYOffset();
		}
	}

	void TextSprite::ClearMesh()
	{
		for (auto& renderElem : mCachedRenderElements)
		{
			if (renderElem.vertices != nullptr)
			{
				mAlloc.Free(renderElem.vertices);
				renderElem.vertices = nullptr;
			}

			if (renderElem.uvs != nullptr)
			{
				mAlloc.Free(renderElem.uvs);
				renderElem.uvs = nullptr;
			}

			if (renderElem.indexes != nullptr)
			{
				mAlloc.Free(renderElem.indexes);
				renderElem.indexes = nullptr;
			}
		}

		mCachedRenderElements.Clear();
		mAlloc.Clear();

		updateBounds();
	}
}
