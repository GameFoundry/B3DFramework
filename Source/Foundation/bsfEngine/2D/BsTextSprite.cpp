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

	void TextSprite::update(const TEXT_SPRITE_DESC& desc, UINT64 groupId)
	{
		bs_frame_mark();
		{
			const U32String utf32text = UTF8::toUTF32(desc.text);
			TextData<FrameAlloc> textData(utf32text, desc.font, desc.fontSize, desc.width, desc.height, desc.wordWrap, 
				desc.wordBreak);

			UINT32 numPages = textData.getNumPages();

			// Free all previous memory
			for (auto& cachedElem : mCachedRenderElements)
			{
				if (cachedElem.vertices != nullptr) mAlloc.free(cachedElem.vertices);
				if (cachedElem.uvs != nullptr) mAlloc.free(cachedElem.uvs);
				if (cachedElem.indexes != nullptr) mAlloc.free(cachedElem.indexes);
			}

			mAlloc.clear();

			// Resize cached mesh array to needed size
			if (mCachedRenderElements.size() != numPages)
				mCachedRenderElements.resize(numPages);

			// Actually generate a mesh
			UINT32 texPage = 0;
			for (auto& cachedElem : mCachedRenderElements)
			{
				UINT32 newNumQuads = textData.getNumQuadsForPage(texPage);

				cachedElem.vertices = (Vector2*)mAlloc.alloc(sizeof(Vector2) * newNumQuads * 4);
				cachedElem.uvs = (Vector2*)mAlloc.alloc(sizeof(Vector2) * newNumQuads * 4);
				cachedElem.indexes = (UINT32*)mAlloc.alloc(sizeof(UINT32) * newNumQuads * 6);
				cachedElem.numQuads = newNumQuads;

				const HTexture& tex = textData.getTextureForPage(texPage);

				SpriteMaterialInfo& matInfo = cachedElem.matInfo;
				matInfo.groupId = groupId;
				matInfo.texture = tex;
				matInfo.tint = desc.color;
				matInfo.animationStartTime = 0.0f;

				cachedElem.material = SpriteManager::instance().getTextMaterial();

				texPage++;
			}

			// Calc alignment and anchor offsets and set final line positions
			for (UINT32 j = 0; j < numPages; j++)
			{
				SpriteRenderElement& renderElem = mCachedRenderElements[j];

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
		UINT32 numLines = textData.getNumLines();
		UINT32 newNumQuads = textData.getNumQuadsForPage(page);

		Vector2I* alignmentOffsets = bs_stack_new<Vector2I>(numLines);
		getAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
		Vector2I offset = getAnchorOffset(anchor, width, height);

		UINT32 quadOffset = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.getLine(i);
			UINT32 writtenQuads = line.fillBuffer(page, vertices, uv, indices, quadOffset, bufferSizeQuads);

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
		UINT32 numLines = textData.getNumLines();
		UINT32 numPages = textData.getNumPages();

		Vector2I* alignmentOffsets = bs_stack_new<Vector2I>(numLines);
		getAlignmentOffsets(textData, width, height, horzAlign, vertAlign, alignmentOffsets);
		Vector2I offset = getAnchorOffset(anchor, width, height);

		UINT32 quadOffset = 0;
		
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.getLine(i);
			for(UINT32 j = 0; j < numPages; j++)
			{
				UINT32 writtenQuads = line.fillBuffer(j, vertices, uv, indices, quadOffset, bufferSizeQuads);

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
		UINT32 numLines = textData.getNumLines();
		UINT32 curHeight = 0;
		for(UINT32 i = 0; i < numLines; i++)
		{
			const TextDataBase::TextLine& line = textData.getLine(i);
			curHeight += line.getYOffset();
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
			const TextDataBase::TextLine& line = textData.getLine(i);

			UINT32 horzOffset = 0;
			switch(horzAlign)
			{
			case THA_Left:
				horzOffset = 0;
				break;
			case THA_Right:
				horzOffset = (UINT32)std::max(0, (INT32)(width - line.getWidth()));
				break;
			case THA_Center:
				horzOffset = (UINT32)std::max(0, (INT32)(width - line.getWidth())) / 2;
				break;
			}

			output[i] = Vector2I(horzOffset, vertOffset + curY);
			curY += line.getYOffset();
		}
	}

	void TextSprite::clearMesh()
	{
		for (auto& renderElem : mCachedRenderElements)
		{
			if (renderElem.vertices != nullptr)
			{
				mAlloc.free(renderElem.vertices);
				renderElem.vertices = nullptr;
			}

			if (renderElem.uvs != nullptr)
			{
				mAlloc.free(renderElem.uvs);
				renderElem.uvs = nullptr;
			}

			if (renderElem.indexes != nullptr)
			{
				mAlloc.free(renderElem.indexes);
				renderElem.indexes = nullptr;
			}
		}

		mCachedRenderElements.clear();
		mAlloc.clear();

		updateBounds();
	}
}
