//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/B3DFont.h"
#include "Resources/B3DResources.h"
#include "Image/B3DTextureAtlasLayout.h"
#include "RTTI/B3DFontRTTI.h"
#include "B3DApplication.h"
#include "FileSystem/B3DDataStream.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererManager.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

#define USE_FREETYPE2_STATIC
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/tttables.h>
#include <freetype/ftoutln.h>

#include FT_FREETYPE_H

using namespace b3d;

namespace b3d
{
	B3D_LOG_CATEGORY(LogFont)
}

/** Converts a 26.6 fixed point format to float. */
static float ConvertFixed26Dot6ToFloat(i32 value)
{
	return (float)value / 64.0f;
}

/** Converts a float to 26.6 fixed point format. */
static FT_Pos ConvertFloatToFixed26Dot6(float value)
{
	return FT_Pos(value * 64.0f);
}

/** Converts a 16.16 fixed point format to float. */
static float ConvertFixed16Dot16ToFloat(FT_Fixed value)
{
	return (float)value / 65536.0f;
}

/** Converts font render mode into FreeType load flags. */
static FT_Int32 ConvertFontRenderModeToLoadFlags(FontRenderMode renderMode)
{
	switch(renderMode)
	{
	case FontRenderMode::Smooth: return FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
	case FontRenderMode::Raster: return FT_LOAD_TARGET_MONO | FT_LOAD_NO_HINTING;
	case FontRenderMode::HintedSmooth: return FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_AUTOHINT;
	case FontRenderMode::HintedRaster: return FT_LOAD_TARGET_MONO | FT_LOAD_NO_AUTOHINT;
	default: return FT_LOAD_TARGET_NORMAL;
	}
}

/**
 * Checks if glyphs rendered in the provided mode are positioned using fractional (unhinted) advances and kerning. Only
 * hinted monochrome glyphs are fitted to the pixel grid horizontally, and their whole-pixel advances keep them evenly
 * spaced.
 */
static bool UsesFractionalMetrics(FontRenderMode renderMode)
{
	return renderMode != FontRenderMode::HintedRaster;
}

/** Coverage of a glyph rendered at a single horizontal subpixel position. */
struct GlyphPositionBitmap
{
	Vector<u8> Pixels;
	i32 Left = 0; /**< Offset from the pen position to the left edge of the bitmap, in pixels. */
	i32 Top = 0; /**< Offset from the baseline to the top edge of the bitmap, in pixels (up is positive). */
	u32 Width = 0;
	u32 Height = 0;
};

/** Copies the bitmap of the rendered glyph in @p glyph into @p output, expanding it to one byte per pixel. */
static bool ReadGlyphBitmap(const FT_GlyphSlot& glyph, u32 characterId, GlyphPositionBitmap& output)
{
	const FT_Bitmap& bitmap = glyph->bitmap;

	output.Left = glyph->bitmap_left;
	output.Top = glyph->bitmap_top;
	output.Width = (u32)bitmap.width;
	output.Height = (u32)bitmap.rows;
	output.Pixels.resize((size_t)output.Width * output.Height);

	if(output.Pixels.empty())
		return true;

	if(bitmap.buffer == nullptr)
	{
		B3D_LOG(Error, LogFont, "Failed to render glyph '{0}. Bitmap is empty.", characterId);
		return false;
	}

	const u8* sourceBuffer = bitmap.buffer;
	u8* destinationBuffer = output.Pixels.data();

	if(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
	{
		for(u32 bitmapY = 0; bitmapY < output.Height; bitmapY++)
		{
			memcpy(destinationBuffer, sourceBuffer, output.Width);

			destinationBuffer += output.Width;
			sourceBuffer += bitmap.pitch;
		}
	}
	else if(bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
	{
		// 8 pixels are packed into a byte, so do some unpacking
		for(u32 bitmapY = 0; bitmapY < output.Height; bitmapY++)
		{
			for(u32 bitmapX = 0; bitmapX < output.Width; bitmapX++)
			{
				const u8 sourceValue = sourceBuffer[bitmapX >> 3];
				destinationBuffer[bitmapX] = (sourceValue & (128 >> (bitmapX & 7))) != 0 ? 255 : 0;
			}

			destinationBuffer += output.Width;
			sourceBuffer += bitmap.pitch;
		}
	}
	else
	{
		B3D_LOG(Error, LogFont, "Failed to render glyph '{0}'. Unsupported pixel mode.", characterId);
		return false;
	}

	return true;
}

const CharacterInformation& FontBitmapInformation::GetCharacterInformation(u32 characterId) const
{
	auto iterFind = Characters.find(characterId);
	if(iterFind != Characters.end())
	{
		return Characters.at(characterId);
	}

	return MissingGlyph;
}

RTTIType* FontBitmapInformation::GetRttiStatic()
{
	return FontBitmapInformationRTTI::Instance();
}

RTTIType* FontBitmapInformation::GetRtti() const
{
	return FontBitmapInformation::GetRttiStatic();
}

RTTIType* FontBitmapPage::GetRttiStatic()
{
	return FontBitmapPageRTTI::Instance();
}

RTTIType* FontBitmapPage::GetRtti() const
{
	return GetRttiStatic();
}

/** Version reported by an OS/2 table that is present but carries no usable data. */
constexpr u16 kInvalidOS2TableVersion = 0xFFFF;

/** Bit within the OS/2 fsSelection field that marks the face as italic. */
constexpr u16 kOS2SelectionItalic = 1 << 0;

struct Font::Implementation
{
	FT_Face Face = nullptr;
};

Font::Font(const FontCreateInformation& createInformation)
	: Resource(false, createInformation.Name)
	, mInformation(createInformation)
	, mImplementation(B3DNew<Implementation>())
{}

Font::~Font()
{
	if(B3D_ENSURE(mImplementation))
		B3DDelete(mImplementation);
}

void Font::Initialize()
{
	for(const auto& page : mFontPages)
	{
		if(page.Texture != nullptr)
			AddResourceDependency(page.Texture);
	}

	if(mInformation.FontData != nullptr)
		InitializeFontRenderer();

	Resource::Initialize();
}

void Font::Destroy()
{
	DestroyFontRenderer();

	Resource::Destroy();
}

bool Font::InitializeFontRenderer()
{
	FT_Error error = 0;

	if(mImplementation->Face != nullptr)
		return false;

	if(mInformation.FontData == nullptr)
	{
		B3D_LOG(Error, LogFont, "Failed to initialize font renderer. Font data is null.");
		return false;
	}

	static FT_Library library = nullptr;
	if(library == nullptr)
	{
		error = FT_Init_FreeType(&library);
		if(error)
		{
			B3D_LOG(Error, LogFont, "Failed to initialize font renderer. Error occurred during FreeType library initialization.");
			return false;
		}
	}

	mImplementation->Face = nullptr;
	error = FT_New_Memory_Face(library, (FT_Byte*)mInformation.FontData->Data(), (FT_Long)mInformation.FontData->Size(), 0, &mImplementation->Face);

	if(error == FT_Err_Unknown_File_Format)
	{
		B3D_LOG(Error, LogFont, "Failed to initialize font renderer. Unsupported file format.");
	}
	else if(error)
	{
		B3D_LOG(Error, LogFont, "Failed to initialize font renderer. Unknown error.");
	}

	// Record which family the face belongs to and which face of it this is, so it can be found through a family lookup without loading the font.
	if(const FT_Face& face = mImplementation->Face; face != nullptr)
	{
		TShared<FontMetaData> metaData = B3DMakeShared<FontMetaData>();
		metaData->FamilyName = face->family_name != nullptr ? face->family_name : mInformation.Name;

		// The OS/2 table carries the exact weight and slant, but it is optional. When it is missing FreeType still provides
		// a coarse bold/italic classification derived from the face, which is enough to place the face within its family.
		const TT_OS2* os2Table = (const TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
		if(os2Table != nullptr && os2Table->version != kInvalidOS2TableVersion)
		{
			const u16 weight = Math::Clamp(os2Table->usWeightClass, (u16)kMinimumFontWeight, (u16)kMaximumFontWeight);

			metaData->Style.Weight = (FontWeight)weight;
			metaData->Style.Slant = (os2Table->fsSelection & kOS2SelectionItalic) != 0 ? FontSlant::Italic : FontSlant::Normal;
		}
		else
		{
			metaData->Style.Weight = (face->style_flags & FT_STYLE_FLAG_BOLD) != 0 ? FontWeight::Bold : FontWeight::Normal;
			metaData->Style.Slant = (face->style_flags & FT_STYLE_FLAG_ITALIC) != 0 ? FontSlant::Italic : FontSlant::Normal;
		}

		mMetaData = std::move(metaData);
	}

	return true;
}

void Font::DestroyFontRenderer()
{
	if(mImplementation->Face == nullptr)
		return;

	FT_Done_Face(mImplementation->Face);
	mImplementation->Face = nullptr;
}

bool Font::RenderGlyphs(float sizeInPoints, const TArrayView<u32>& characterIds, bool bake)
{
	if(mImplementation->Face == nullptr)
	{
		B3D_LOG(Error, LogFont, "Failed to render font glyphs. Font renderer is not initialized.");
		return false;
	}

	const float quantizedFontSizeInPoints = GetQuantizedFontSize(sizeInPoints);
	Vector<GlyphBitmap> glyphBitmaps;

	const FT_Int32 loadFlags = ConvertFontRenderModeToLoadFlags(mInformation.RenderMode);
	const bool usesFractionalMetrics = UsesFractionalMetrics(mInformation.RenderMode);
	const FT_Face& face = mImplementation->Face;

	if(!B3D_ENSURE(FT_Set_Char_Size(mImplementation->Face, ConvertFloatToFixed26Dot6(quantizedFontSizeInPoints), 0, mInformation.DPI, mInformation.DPI) == 0))
	{
		B3D_LOG(Error, LogFont, "Failed to render font glyphs. Failed to set character size.");
		return false;
	}

	TShared<FontBitmapInformation> bitmapInformation;
	if(auto found = mCharactersByPointSize.find(quantizedFontSizeInPoints); found != mCharactersByPointSize.end())
		bitmapInformation = found->second;

	if(bitmapInformation == nullptr)
	{
		bitmapInformation = B3DMakeShared<FontBitmapInformation>();

		const FT_Size_Metrics& faceMetrics = mImplementation->Face->size->metrics;

		bitmapInformation->Size = quantizedFontSizeInPoints;
		bitmapInformation->LineHeight = ConvertFixed26Dot6ToFloat(faceMetrics.height);
		bitmapInformation->BaselineOffset = ConvertFixed26Dot6ToFloat(faceMetrics.height + faceMetrics.descender);

		constexpr FT_ULong kSpaceCharacterId = 32;
		FT_Load_Char(mImplementation->Face, kSpaceCharacterId, loadFlags);

		const FT_GlyphSlot& glyph = mImplementation->Face->glyph;
		bitmapInformation->SpaceWidth = usesFractionalMetrics ? ConvertFixed16Dot16ToFloat(glyph->linearHoriAdvance) : ConvertFixed26Dot6ToFloat(glyph->advance.x);

		// Monochrome glyphs have no partial coverage to express a fractional position with. Large glyphs gain little from it.
		const bool isAntialiased = mInformation.RenderMode == FontRenderMode::Smooth || mInformation.RenderMode == FontRenderMode::HintedSmooth;
		const float sizeInPixels = quantizedFontSizeInPoints * (float)mInformation.DPI / 72.0f;
		bitmapInformation->SubpixelPositionCount = isAntialiased && sizeInPixels <= kMaximumSubpixelPositionedSize ? kSubpixelPositionCount : 1;

		mCharactersByPointSize[quantizedFontSizeInPoints] = bitmapInformation;

		u32 kMissingGlyphId[] = { 0 };
		RenderGlyphs(sizeInPoints, TArrayView(kMissingGlyphId, 1));

		auto foundCharacter = bitmapInformation->Characters.find(0);
		if(B3D_ENSURE(foundCharacter != bitmapInformation->Characters.end()))
			bitmapInformation->MissingGlyph = foundCharacter->second;
	}

	const u32 subpixelPositionCount = bitmapInformation->SubpixelPositionCount;
	Vector<GlyphPositionBitmap> positionBitmaps(subpixelPositionCount);

	for(const auto& characterId : characterIds)
	{
		if(auto found = bitmapInformation->Characters.find(characterId); found != bitmapInformation->Characters.end())
			continue;

		const FT_UInt glyphIndex = FT_Get_Char_Index(face, (FT_ULong)characterId);
		FT_Error error = FT_Load_Glyph(face, glyphIndex, loadFlags);

		if(error)
		{
			B3D_LOG(Error, LogFont, "Failed to render font glyph '{0}'. Failed to load character.", characterId);
			continue;
		}

		const FT_GlyphSlot& glyph = face->glyph;

		CharacterInformation characterInformation;
		characterInformation.CharId = characterId;
		characterInformation.XAdvance = usesFractionalMetrics ? ConvertFixed16Dot16ToFloat(glyph->linearHoriAdvance) : ConvertFixed26Dot6ToFloat(glyph->metrics.horiAdvance);
		characterInformation.YAdvance = ConvertFixed26Dot6ToFloat(glyph->advance.y);
		characterInformation.PointSize = 0.0f;

		// Render the glyph at every subpixel position, shifting the outline right by a fraction of a pixel for each. TrueType
		// hinting of anti-aliased glyphs only snaps them vertically, so shifting the hinted outline is equivalent to hinting
		// it at the shifted position.
		bool isRendered = true;
		for(u32 positionIndex = 0; positionIndex < subpixelPositionCount; positionIndex++)
		{
			// Rendering replaces the outline with the bitmap, so every position after the first loads the glyph again
			if(positionIndex > 0)
			{
				error = FT_Load_Glyph(face, glyphIndex, loadFlags);
				if(!error && glyph->format == FT_GLYPH_FORMAT_OUTLINE)
					FT_Outline_Translate(&glyph->outline, (FT_Pos)(positionIndex * 64 / subpixelPositionCount), 0);
			}

			if(!error)
				error = FT_Render_Glyph(glyph, FT_LOAD_TARGET_MODE(loadFlags));

			if(error)
			{
				B3D_LOG(Error, LogFont, "Failed to render font glyph '{0}'. Failed to render character.", characterId);
				isRendered = false;
				break;
			}

			if(!ReadGlyphBitmap(glyph, characterId, positionBitmaps[positionIndex]))
			{
				isRendered = false;
				break;
			}
		}

		if(!isRendered)
			continue;

		// Parse kerning, pairing the new character with itself and with every character already rendered at this size
		if(FT_HAS_KERNING(face))
		{
			// Whole-pixel advances need whole-pixel kerning to stay on the pixel grid
			const FT_UInt kerningMode = usesFractionalMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT;

			auto fnAddKerning = [&face, kerningMode](CharacterInformation& leftCharacterInformation, FT_UInt leftGlyphIndex, u32 rightCharacterId, FT_UInt rightGlyphIndex) {
				// Pairs survive when runtime glyphs are cleared, so a re-rendered character may already be paired
				for(const KerningPair& existingPair : leftCharacterInformation.KerningPairs)
				{
					if(existingPair.OtherCharId == rightCharacterId)
						return;
				}

				FT_Vector kerning;

				const FT_Error error = FT_Get_Kerning(face, leftGlyphIndex, rightGlyphIndex, kerningMode, &kerning);
				if(error)
				{
					B3D_LOG(Error, LogFont, "Failed to get kerning information for glyphs '{0}', '{1}'.",  leftCharacterInformation.CharId, rightCharacterId);
					return;
				}

				const float kerningX = ConvertFixed26Dot6ToFloat(kerning.x); // Y kerning is ignored because it is so rare
				if(kerningX != 0.0f) // We don't store 0 kerning, this is assumed default
				{
					KerningPair kerningPair;
					kerningPair.Amount = kerningX;
					kerningPair.OtherCharId = rightCharacterId;

					leftCharacterInformation.KerningPairs.push_back(kerningPair);
				}
			};

			fnAddKerning(characterInformation, glyphIndex, characterId, glyphIndex);

			for(auto& keyValuePair : bitmapInformation->Characters)
			{
				const u32 otherCharacterId = keyValuePair.first;
				CharacterInformation& otherCharacterInformation = keyValuePair.second;
				const FT_UInt otherGlyphIndex = FT_Get_Char_Index(face, (FT_ULong)otherCharacterId);

				fnAddKerning(characterInformation, glyphIndex, otherCharacterId, otherGlyphIndex);
				fnAddKerning(otherCharacterInformation, otherGlyphIndex, characterId, glyphIndex);
			}
		}

		// Find the area covered by the glyph at any subpixel position. Bitmaps for all positions are stored in cells of that
		// size, so a position is selected by offsetting the UV alone.
		i32 cellLeft = std::numeric_limits<i32>::max();
		i32 cellRight = std::numeric_limits<i32>::min();
		i32 cellTop = std::numeric_limits<i32>::min();
		i32 cellBottom = std::numeric_limits<i32>::max();
		for(const GlyphPositionBitmap& positionBitmap : positionBitmaps)
		{
			if(positionBitmap.Pixels.empty())
				continue;

			cellLeft = std::min(cellLeft, positionBitmap.Left);
			cellRight = std::max(cellRight, positionBitmap.Left + (i32)positionBitmap.Width);
			cellTop = std::max(cellTop, positionBitmap.Top);
			cellBottom = std::min(cellBottom, positionBitmap.Top - (i32)positionBitmap.Height);
		}

		if(cellLeft >= cellRight)
			continue;

		const u32 cellWidth = (u32)(cellRight - cellLeft);
		const u32 cellHeight = (u32)(cellTop - cellBottom);
		const Size2UI bitmapSize(cellWidth * subpixelPositionCount, cellHeight);

		// Quads cover the bitmap exactly, so a texel maps to a single pixel
		characterInformation.Width = (float)cellWidth;
		characterInformation.Height = (float)cellHeight;
		characterInformation.XOffset = (float)cellLeft;
		characterInformation.YOffset = (float)cellTop;

		TextureCreateInformation textureCreateInformation;
		textureCreateInformation.Name = "FontGlyph";
		textureCreateInformation.Width = bitmapSize.Width;
		textureCreateInformation.Height = bitmapSize.Height;
		textureCreateInformation.Format = PF_R8;

		// Note: Probably not efficient creating a new texture for each glyph
		HTexture texture = Texture::Create(textureCreateInformation);

		const TShared<PixelData> destinationPixelData = texture->GetProperties().AllocBuffer(0, 0);
		u8* destinationBuffer = destinationPixelData->GetData();
		memset(destinationBuffer, 0, (size_t)bitmapSize.Width * bitmapSize.Height);

		for(u32 positionIndex = 0; positionIndex < subpixelPositionCount; positionIndex++)
		{
			const GlyphPositionBitmap& positionBitmap = positionBitmaps[positionIndex];
			const u32 cellX = positionIndex * cellWidth + (u32)(positionBitmap.Left - cellLeft);
			const u32 cellY = (u32)(cellTop - positionBitmap.Top);

			for(u32 bitmapY = 0; bitmapY < positionBitmap.Height; bitmapY++)
			{
				u8* destinationRow = destinationBuffer + (size_t)(cellY + bitmapY) * bitmapSize.Width + cellX;
				memcpy(destinationRow, &positionBitmap.Pixels[(size_t)bitmapY * positionBitmap.Width], positionBitmap.Width);
			}
		}

		texture->WriteData(destinationPixelData);

		u32 targetPageIndex = ~0u;
		TOptional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
		for(u32 pageIndex = 0; pageIndex < (u32)mFontPages.size(); ++pageIndex)
		{
			FontBitmapPage& page = mFontPages[pageIndex];
			if(bake && page.Type != FontBitmapPageType::Baked)
				continue;

			if(!bake && page.Type != FontBitmapPageType::Runtime)
				continue;

			layoutAllocation = page.Layout.AddElement(bitmapSize);
			if(!layoutAllocation)
				continue;

			targetPageIndex = pageIndex;
			break;
		}

		if(!layoutAllocation.has_value())
		{
			targetPageIndex = (u32)mFontPages.size();

			FontBitmapPage newPage;
			newPage.Type = bake ? FontBitmapPageType::Baked : FontBitmapPageType::Runtime;

			TextureCreateInformation pageTextureCreateInformation;
			pageTextureCreateInformation.Name = StringUtility::Format("Font Page: {0} #{1}", GetName(), targetPageIndex);
			pageTextureCreateInformation.Width = kFontPageSize;
			pageTextureCreateInformation.Height = kFontPageSize;
			pageTextureCreateInformation.Format = PF_R8;

			newPage.Texture = Texture::Create(pageTextureCreateInformation);
			AddResourceDependency(newPage.Texture);

			TreeTextureAtlasLayoutSettings atlasLayoutSettings;
			atlasLayoutSettings.Size = Size2UI(kFontPageSize, kFontPageSize);
			atlasLayoutSettings.MaximumPageCount = 1;

			newPage.Layout = TreeTextureAtlasLayout(atlasLayoutSettings);
			layoutAllocation = newPage.Layout.AddElement(bitmapSize);

			mFontPages.push_back(newPage);
		}

		if(!layoutAllocation || !B3D_ENSURE(targetPageIndex != ~0u))
		{
			B3D_LOG(Error, LogFont, "Failed to render glyph '{0}'. Failed to allocate a slot in the texture atlas.", characterId);
			continue;
		}

		B3D_ASSERT(layoutAllocation->PageId == 0);

		FontBitmapPage& page = mFontPages[targetPageIndex];

		GlyphBitmap glyphBitmap;
		glyphBitmap.GlyphTexture = B3DGetRenderProxy(texture);
		glyphBitmap.AtlasTexture = B3DGetRenderProxy(page.Texture);
		glyphBitmap.Size = bitmapSize;
		glyphBitmap.PositionInAtlas = layoutAllocation->Position;

		const float inversePageWidth = 1.0f / (float)page.Texture->GetProperties().Width;
		const float inversePageHeight = 1.0f / (float)page.Texture->GetProperties().Height;

		characterInformation.Page = targetPageIndex;
		characterInformation.UvX = inversePageWidth * (float)glyphBitmap.PositionInAtlas.X;
		characterInformation.UvY = inversePageHeight * (float)glyphBitmap.PositionInAtlas.Y;
		characterInformation.UvWidth = inversePageWidth * (float)cellWidth;
		characterInformation.UvHeight = inversePageHeight * (float)cellHeight;
		characterInformation.DynamicLayoutAllocation = layoutAllocation;

		bitmapInformation->Characters[characterId] = std::move(characterInformation);
		glyphBitmaps.push_back(std::move(glyphBitmap));
	}

	FontAtlasRenderer::Instance().BlitGlyphs(std::move(glyphBitmaps));
	return false;
}

void Font::ClearGlyphs(FontBitmapInformation& bitmapInformation, bool onlyRuntime)
{
	for(auto it = bitmapInformation.Characters.begin(); it != bitmapInformation.Characters.end();)
	{
		const CharacterInformation& characterInformation = it->second;
		FontBitmapPage& fontBitmapPage = mFontPages[characterInformation.Page];

		const bool isBakedOrLoaded = fontBitmapPage.Type == FontBitmapPageType::Baked || fontBitmapPage.Type == FontBitmapPageType::Loaded;
		if(isBakedOrLoaded && onlyRuntime)
		{
			++it;
			continue;
		}

		if(characterInformation.DynamicLayoutAllocation.has_value())
		{
			fontBitmapPage.Layout.RemoveElement(characterInformation.DynamicLayoutAllocation->PageId, characterInformation.DynamicLayoutAllocation->NodeId);

			if(fontBitmapPage.Layout.IsEmpty())
				RemovePage(characterInformation.Page);
		}

		it = bitmapInformation.Characters.erase(it);
	}
}

void Font::ClearGlyphs(float size, bool onlyRuntime)
{
	const float quantizedFontSize = GetQuantizedFontSize(size);

	TShared<FontBitmapInformation> bitmapInformation;
	if(auto found = mCharactersByPointSize.find(quantizedFontSize); found != mCharactersByPointSize.end())
		bitmapInformation = found->second;

	if(bitmapInformation == nullptr)
	{
		mCharactersByPointSize.erase(quantizedFontSize);
		return;
	}

	ClearGlyphs(*bitmapInformation, onlyRuntime);

	if(bitmapInformation->Characters.empty())
		mCharactersByPointSize.erase(quantizedFontSize);
}

void Font::ClearGlyphs(bool onlyRuntime)
{
	for(auto it = mCharactersByPointSize.begin(); it != mCharactersByPointSize.end();)
	{
		TShared<FontBitmapInformation> bitmapInformation = it->second;

		if(bitmapInformation == nullptr)
		{
			it = mCharactersByPointSize.erase(it);
			continue;
		}

		ClearGlyphs(*bitmapInformation, onlyRuntime);

		if(bitmapInformation->Characters.empty())
			it = mCharactersByPointSize.erase(it);
		
		++it;
	}
}

void Font::RemovePage(u32 pageIndex)
{
	if(pageIndex >= (u32)mFontPages.size())
		return;

	mFontPages.erase(mFontPages.begin() + pageIndex);

	for(const auto& bitmapPair : mCharactersByPointSize)
	{
		const TShared<FontBitmapInformation>& bitmapInformation = bitmapPair.second;
		if(!B3D_ENSURE(bitmapInformation != nullptr))
			continue;

		for(auto& characterPair : bitmapInformation->Characters)
		{
			CharacterInformation& characterInformation = characterPair.second;

			// Caller must make sure to remove characters referencing this page, before calling this method
			B3D_ENSURE(characterInformation.Page != pageIndex);

			if(characterInformation.Page > pageIndex)
				characterInformation.Page--;
		}
	}
}

TShared<FontBitmapInformation> Font::GetBitmap(float size) const
{
	const float quantizedFontSize = GetQuantizedFontSize(size);

	if(auto found = mCharactersByPointSize.find(quantizedFontSize); found != mCharactersByPointSize.end())
		return found->second;

	return nullptr;
}

float Font::GetClosestExistingBitmapSize(float size) const
{
	float minDiff = std::numeric_limits<float>::max();
	float bestSize = size;

	for(auto iter = mCharactersByPointSize.begin(); iter != mCharactersByPointSize.end(); ++iter)
	{
		if(iter->first == size)
			return size;
		else if(iter->first > size)
		{
			const float diff = iter->first - size;
			if(diff < minDiff)
			{
				minDiff = diff;
				bestSize = iter->first;
			}
		}
		else
		{
			const float diff = size - iter->first;
			if(diff < minDiff)
			{
				minDiff = diff;
				bestSize = iter->first;
			}
		}
	}

	return bestSize;
}

const CharacterInformation* Font::FindCharacterInformation(u32 characterId, float sizeInPoints) const
{
	auto foundBitmapInformation = mCharactersByPointSize.find(sizeInPoints);
	if(foundBitmapInformation != mCharactersByPointSize.end())
	{
		const TShared<FontBitmapInformation> bitmapInformation = foundBitmapInformation->second;
		const CharacterInformation& characterInformation = foundBitmapInformation->second->GetCharacterInformation(characterId);
		if(characterInformation.CharId == characterId)
			return &characterInformation;
	}

	return nullptr;
}

const FontBitmapPage& Font::GetPage(u32 pageIndex) const
{
	if(pageIndex >= (u32)mFontPages.size())
	{
		static FontBitmapPage kEmpty;
		return kEmpty;
	}

	return mFontPages[pageIndex];
}

float Font::GetPointSizeForGlyphThatFitsArea(u32 glyphId, const Size2I& size) const
{
	// Use the provided width as a starting point. The actual pixel width will likely not match the requested size, but
	// we adjust for that below
	const float approximateWidthInPoints = (size.Width * 72.0f) / (float)mInformation.DPI;

	if(mImplementation->Face == nullptr)
	{
		B3D_LOG(Error, LogFont, "Failed to get glyph size. Font renderer is not initialized.");
		return approximateWidthInPoints;
	}

	const FT_Face& face = mImplementation->Face;
	FT_Error error = FT_Set_Char_Size(face, ConvertFloatToFixed26Dot6(approximateWidthInPoints), 0, mInformation.DPI, mInformation.DPI);
	if (error)
	{
		B3D_LOG(Error, LogFont, "Failed to get glyph size. Failed to set character size.");
		return approximateWidthInPoints;
	}

	const FT_Int32 loadFlags = ConvertFontRenderModeToLoadFlags(mInformation.RenderMode);
	error = FT_Load_Char(face, (FT_ULong)glyphId, loadFlags);
	if (error)
	{
		B3D_LOG(Error, LogFont, "Failed to get glyph size. Failed to load character.");
		return approximateWidthInPoints;
	}

	const FT_GlyphSlot slot = face->glyph;
	const float approximateGlyphWidthInPixels = ConvertFixed26Dot6ToFloat(slot->metrics.width);
	const float approximateGlyphHeightInPixels = ConvertFixed26Dot6ToFloat(slot->metrics.height);

	// Determine scaling factor we need to apply to the font size in order to reach the requested pixel width/height without stretching or cropping.
	// Note: This might still not yield exact requested size. We might want to do another iteration of the process above to be more precise, or
	//       figure out a better algorithm.
	const float horizontalRatio = (float)size.Width / (float)approximateGlyphWidthInPixels;
	const float verticalRatio = (float)size.Height / (float)approximateGlyphHeightInPixels;

	return approximateWidthInPoints * Math::Min(verticalRatio, horizontalRatio);
}

void Font::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& page : mFontPages)
	{
		if(page.Texture.IsLoaded())
			dependencies.push_back(page.Texture.Get());
	}
}

float Font::GetQuantizedFontSize(float size)
{
	return (float)Math::RoundToI32(size * kFontQuantizeAmount) / (float)kFontQuantizeAmount;
}

HFont Font::Create(const FontCreateInformation& createInformation)
{
	TShared<Font> newFont = CreateShared(createInformation);

	return B3DStaticResourceCast<Font>(GetResources().CreateResourceHandle(newFont));
}

TShared<Font> Font::CreateShared(const FontCreateInformation& createInformation)
{
	TShared<Font> newFont = B3DMakeSharedFromExisting<Font>(new(B3DAllocate<Font>()) Font(createInformation));
	newFont->SetShared(newFont);
	newFont->Initialize();

	return newFont;
}

TShared<Font> Font::CreateEmpty()
{
	TShared<Font> newFont = B3DMakeSharedFromExisting<Font>(new(B3DAllocate<Font>()) Font(FontCreateInformation()));
	newFont->SetShared(newFont);

	return newFont;
}

RTTIType* Font::GetRttiStatic()
{
	return FontRTTI::Instance();
}

RTTIType* Font::GetRtti() const
{
	return Font::GetRttiStatic();
}

RTTIType* FontMetaData::GetRttiStatic()
{
	return FontMetaDataRTTI::Instance();
}

RTTIType* FontMetaData::GetRtti() const
{
	return FontMetaData::GetRttiStatic();
}

void FontAtlasRenderer::OnStartUp()
{
	TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if (!gpuDevice)
		return;

	auto fnCreateCommandBufferPool = [this, gpuDevice]()
	{
		render::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Type = GQT_GRAPHICS;

		mCommandBufferPool = gpuDevice->CreateGpuCommandBufferPool(poolCreateInformation);
	};

	GetRenderThread().PostCommand(fnCreateCommandBufferPool, "FontAtlasRenderer::CreateCommandBufferPool");
}

void FontAtlasRenderer::OnShutDown()
{
	auto fnDestroyCommandBufferPool = [this]()
	{
		mCommandBufferPool = nullptr;
	};

	GetRenderThread().PostCommand(fnDestroyCommandBufferPool, "FontAtlasRenderer::DestroyCommandBufferPool", true);
}

void FontAtlasRenderer::BlitGlyphs(Vector<GlyphBitmap> glyphBitmaps)
{
	auto fnBlitToAtlas = [this, glyphBitmaps = std::move(glyphBitmaps)]() {

		if (!B3D_ENSURE(mCommandBufferPool))
			return;

		const TShared<render::GpuCommandBuffer> commandBuffer = mCommandBufferPool->Create(render::GpuCommandBufferCreateInformation::Create("BlitGlyphBitmaps"));

		for(const auto& entry : glyphBitmaps)
		{
			TextureBlitInformation blitInformation;
			blitInformation.DestinationVolume.Left = entry.PositionInAtlas.X;
			blitInformation.DestinationVolume.Top = entry.PositionInAtlas.Y;
			blitInformation.DestinationVolume.Right = entry.PositionInAtlas.X + (i32)entry.Size.Width;
			blitInformation.DestinationVolume.Bottom = entry.PositionInAtlas.Y + (i32)entry.Size.Height;
			blitInformation.DestinationVolume.Back = 1;

			commandBuffer->BlitTexture(entry.GlyphTexture, entry.AtlasTexture, blitInformation);
		}

		GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
		gpuContext.SubmitCommandBuffer(commandBuffer);
	};

	GetRenderThread().PostCommand(fnBlitToAtlas, "FontAtlasRenderer::BlitGlyphs");
}

