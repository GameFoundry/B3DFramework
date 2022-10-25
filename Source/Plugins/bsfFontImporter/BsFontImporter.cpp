//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFontImporter.h"
#include "Text/BsFontImportOptions.h"
#include "Image/BsPixelData.h"
#include "Image/BsTexture.h"
#include "Image/BsTextureAtlasLayout.h"
#include "BsCoreApplication.h"
#include "CoreThread/BsCoreThread.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include FT_FREETYPE_H
#include "FileSystem/BsFileSystem.h"

using namespace std::placeholders;

namespace bs
{
	FontImporter::FontImporter()
		: SpecificImporter()
	{
		mExtensions.push_back(u8"ttf");
		mExtensions.push_back(u8"otf");
	}

	bool FontImporter::IsExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::ToLowerCase(lowerCaseExt);

		return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
	}

	bool FontImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
	{
		// Not used
		return false;
	}

	SPtr<ImportOptions> FontImporter::CreateImportOptions() const
	{
		return bs_shared_ptr_new<FontImportOptions>();
	}

	SPtr<Resource> FontImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		const FontImportOptions* fontImportOptions = static_cast<const FontImportOptions*>(importOptions.get());

		FT_Library library;

		FT_Error error = FT_Init_FreeType(&library);
		if(error)
			BS_EXCEPT(InternalErrorException, "Error occurred during FreeType library initialization.");

		FT_Face face;

		{
			Lock fileLock = FileScheduler::GetLock(filePath);
			error = FT_New_Face(library, filePath.ToString().c_str(), 0, &face);
		}

		if(error == FT_Err_Unknown_File_Format)
		{
			BS_EXCEPT(InternalErrorException, "Failed to load font file: " + filePath.ToString() + ". Unsupported file format.");
		}
		else if(error)
		{
			BS_EXCEPT(InternalErrorException, "Failed to load font file: " + filePath.ToString() + ". Unknown error.");
		}

		Vector<CharRange> charIndexRanges = fontImportOptions->CharIndexRanges;
		Vector<u32> fontSizes = fontImportOptions->FontSizes;
		u32 dpi = fontImportOptions->Dpi;

		FT_Int32 loadFlags;
		switch(fontImportOptions->RenderMode)
		{
		case FontRenderMode::Smooth:
			loadFlags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
			break;
		case FontRenderMode::Raster:
			loadFlags = FT_LOAD_TARGET_MONO | FT_LOAD_NO_HINTING;
			break;
		case FontRenderMode::HintedSmooth:
			loadFlags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_AUTOHINT;
			break;
		case FontRenderMode::HintedRaster:
			loadFlags = FT_LOAD_TARGET_MONO | FT_LOAD_NO_AUTOHINT;
			break;
		default:
			loadFlags = FT_LOAD_TARGET_NORMAL;
			break;
		}

		FT_Render_Mode renderMode = FT_LOAD_TARGET_MODE(loadFlags);

		Vector<SPtr<FontBitmap>> dataPerSize;
		for(size_t i = 0; i < fontSizes.size(); i++)
		{
			// Note: Disabled as its not working and I have bigger issues to handle than to figure this out atm
			// FT_Matrix m;
			// if (fontImportOptions->GetBold())
			//	m.xx = (long)(1.25f * (1 << 16));
			// else
			//	m.xx = (long)(1 * (1 << 16));

			// if (fontImportOptions->GetItalic())
			//	m.xy = (long)(0.25f * (1 << 16));
			// else
			//	m.xy = (long)(0 * (1 << 16));

			// m.yx = (long)(0 * (1 << 16));
			// m.yy = (long)(1 * (1 << 16));

			// FT_Set_Transform(face, &m, nullptr);

			FT_F26Dot6 ftSize = (FT_F26Dot6)(fontSizes[i] * (1 << 6));
			if(FT_Set_Char_Size(face, ftSize, 0, dpi, dpi))
				BS_EXCEPT(InternalErrorException, "Could not set character size.");

			SPtr<FontBitmap> fontData = bs_shared_ptr_new<FontBitmap>();

			// Get all char sizes so we can generate texture layout
			Vector<TextureAtlasUtility::Element> atlasElements;
			Map<u32, u32> seqIdxToCharIdx;
			for(auto iter = charIndexRanges.begin(); iter != charIndexRanges.end(); ++iter)
			{
				for(u32 charIdx = iter->Start; charIdx <= iter->End; charIdx++)
				{
					error = FT_Load_Char(face, (FT_ULong)charIdx, loadFlags);

					if(error)
						BS_EXCEPT(InternalErrorException, "Failed to load a character");

					FT_Render_Glyph(face->glyph, renderMode);

					if(error)
						BS_EXCEPT(InternalErrorException, "Failed to render a character");

					FT_GlyphSlot slot = face->glyph;

					TextureAtlasUtility::Element atlasElement;
					atlasElement.Input.Width = slot->bitmap.width;
					atlasElement.Input.Height = slot->bitmap.rows;

					atlasElements.push_back(atlasElement);
					seqIdxToCharIdx[(u32)atlasElements.size() - 1] = charIdx;
				}
			}

			// Add missing glyph
			{
				error = FT_Load_Glyph(face, (FT_ULong)0, loadFlags);

				if(error)
					BS_EXCEPT(InternalErrorException, "Failed to load a character");

				FT_Render_Glyph(face->glyph, renderMode);

				if(error)
					BS_EXCEPT(InternalErrorException, "Failed to render a character");

				FT_GlyphSlot slot = face->glyph;

				TextureAtlasUtility::Element atlasElement;
				atlasElement.Input.Width = slot->bitmap.width;
				atlasElement.Input.Height = slot->bitmap.rows;

				atlasElements.push_back(atlasElement);
			}

			// Create an optimal layout for character bitmaps
			Vector<TextureAtlasUtility::Page> pages = TextureAtlasUtility::CreateAtlasLayout(atlasElements, 64, 64, MAXIMUM_TEXTURE_SIZE, MAXIMUM_TEXTURE_SIZE, true);

			i32 baselineOffset = 0;
			u32 lineHeight = 0;

			// Create char bitmap atlas textures and load character information
			u32 pageIdx = 0;
			for(auto pageIter = pages.begin(); pageIter != pages.end(); ++pageIter)
			{
				u32 bufferSize = pageIter->Width * pageIter->Height * 2;

				// TODO - I don't actually need a 2 channel texture
				SPtr<PixelData> pixelData = bs_shared_ptr_new<PixelData>(pageIter->Width, pageIter->Height, 1, PF_RG8);

				pixelData->AllocateInternalBuffer();
				u8* pixelBuffer = pixelData->GetData();
				memset(pixelBuffer, 0, bufferSize);

				for(size_t i = 0; i < atlasElements.size(); i++)
				{
					// Copy character bitmap
					if(atlasElements[i].Output.Page != (i32)pageIdx)
						continue;

					TextureAtlasUtility::Element curElement = atlasElements[i];
					u32 elementIdx = curElement.Output.Idx;

					bool isMissingGlypth = elementIdx == (atlasElements.size() - 1); // It's always the last element

					u32 charIdx = 0;
					if(!isMissingGlypth)
					{
						charIdx = seqIdxToCharIdx[(u32)elementIdx];

						error = FT_Load_Char(face, charIdx, loadFlags);
					}
					else
					{
						error = FT_Load_Glyph(face, 0, loadFlags);
					}

					if(error)
						BS_EXCEPT(InternalErrorException, "Failed to load a character");

					FT_Render_Glyph(face->glyph, renderMode);

					if(error)
						BS_EXCEPT(InternalErrorException, "Failed to render a character");

					FT_GlyphSlot slot = face->glyph;

					if(slot->bitmap.buffer == nullptr && slot->bitmap.rows > 0 && slot->bitmap.width > 0)
						BS_EXCEPT(InternalErrorException, "Failed to render glyph bitmap");

					u8* sourceBuffer = slot->bitmap.buffer;
					u8* dstBuffer = pixelBuffer + (curElement.Output.Y * pageIter->Width * 2) + curElement.Output.X * 2;

					if(slot->bitmap.pixel_mode == ft_pixel_mode_grays)
					{
						for(i32 bitmapRow = 0; bitmapRow < slot->bitmap.rows; bitmapRow++)
						{
							for(i32 bitmapColumn = 0; bitmapColumn < slot->bitmap.width; bitmapColumn++)
							{
								dstBuffer[bitmapColumn * 2 + 0] = sourceBuffer[bitmapColumn];
								dstBuffer[bitmapColumn * 2 + 1] = sourceBuffer[bitmapColumn];
							}

							dstBuffer += pageIter->Width * 2;
							sourceBuffer += slot->bitmap.pitch;
						}
					}
					else if(slot->bitmap.pixel_mode == ft_pixel_mode_mono)
					{
						// 8 pixels are packed into a byte, so do some unpacking
						for(i32 bitmapRow = 0; bitmapRow < slot->bitmap.rows; bitmapRow++)
						{
							for(i32 bitmapColumn = 0; bitmapColumn < slot->bitmap.width; bitmapColumn++)
							{
								u8 srcValue = sourceBuffer[bitmapColumn >> 3];
								u8 dstValue = (srcValue & (128 >> (bitmapColumn & 7))) != 0 ? 255 : 0;

								dstBuffer[bitmapColumn * 2 + 0] = dstValue;
								dstBuffer[bitmapColumn * 2 + 1] = dstValue;
							}

							dstBuffer += pageIter->Width * 2;
							sourceBuffer += slot->bitmap.pitch;
						}
					}
					else
						BS_EXCEPT(InternalErrorException, "Unsupported pixel mode for a FreeType bitmap.");

					// Store character information
					CharDesc charDesc;

					float invTexWidth = 1.0f / pageIter->Width;
					float invTexHeight = 1.0f / pageIter->Height;

					charDesc.CharId = charIdx;
					charDesc.Width = curElement.Input.Width;
					charDesc.Height = curElement.Input.Height;
					charDesc.Page = curElement.Output.Page;
					charDesc.UvWidth = invTexWidth * curElement.Input.Width;
					charDesc.UvHeight = invTexHeight * curElement.Input.Height;
					charDesc.UvX = invTexWidth * curElement.Output.X;
					charDesc.UvY = invTexHeight * curElement.Output.Y;
					charDesc.XOffset = slot->bitmap_left;
					charDesc.YOffset = slot->bitmap_top;
					charDesc.XAdvance = slot->advance.x >> 6;
					charDesc.YAdvance = slot->advance.y >> 6;

					baselineOffset = std::max(baselineOffset, (i32)(slot->metrics.horiBearingY >> 6));
					lineHeight = std::max(lineHeight, charDesc.Height);

					// Load kerning and store char
					if(!isMissingGlypth)
					{
						FT_Vector resultKerning;
						for(auto kerningIter = charIndexRanges.begin(); kerningIter != charIndexRanges.end(); ++kerningIter)
						{
							for(u32 kerningCharIdx = kerningIter->Start; kerningCharIdx <= kerningIter->End; kerningCharIdx++)
							{
								if(kerningCharIdx == charIdx)
									continue;

								error = FT_Get_Kerning(face, charIdx, kerningCharIdx, FT_KERNING_DEFAULT, &resultKerning);

								if(error)
									BS_EXCEPT(InternalErrorException, "Failed to get kerning information for character: " + toString(charIdx));

								i32 kerningX = (i32)(resultKerning.x >> 6); // Y kerning is ignored because it is so rare
								if(kerningX == 0) // We don't store 0 kerning, this is assumed default
									continue;

								KerningPair pair;
								pair.Amount = kerningX;
								pair.OtherCharId = kerningCharIdx;

								charDesc.KerningPairs.push_back(pair);
							}
						}

						fontData->Characters[charIdx] = charDesc;
					}
					else
					{
						fontData->MissingGlyph = charDesc;
					}
				}

				TEXTURE_DESC texDesc;
				texDesc.Width = pageIter->Width;
				texDesc.Height = pageIter->Height;
				texDesc.Format = PF_RG8;

				HTexture newTex = Texture::Create(texDesc);

				// It's possible the formats no longer match
				if(newTex->GetProperties().GetFormat() != pixelData->GetFormat())
				{
					SPtr<PixelData> temp = newTex->GetProperties().AllocBuffer(0, 0);
					PixelUtil::BulkPixelConversion(*pixelData, *temp);

					newTex->WriteData(temp);
				}
				else
				{
					newTex->WriteData(pixelData);
				}

				newTex->SetName(u8"FontPage" + toString((u32)fontData->TexturePages.size()));

				fontData->TexturePages.push_back(newTex);
				pageIdx++;
			}

			fontData->Size = fontSizes[i];
			fontData->BaselineOffset = baselineOffset;
			fontData->LineHeight = lineHeight;

			// Get space size
			error = FT_Load_Char(face, 32, loadFlags);

			if(error)
				BS_EXCEPT(InternalErrorException, "Failed to load a character");

			fontData->SpaceWidth = face->glyph->advance.x >> 6;

			dataPerSize.push_back(fontData);
		}

		SPtr<Font> newFont = Font::CreatePtrInternal(dataPerSize);

		FT_Done_FreeType(library);

		const String fileName = filePath.GetFilename(false);
		newFont->SetName(fileName);

		return newFont;
	}
} // namespace bs
