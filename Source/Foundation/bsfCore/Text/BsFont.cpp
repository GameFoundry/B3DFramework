//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFont.h"
#include "Resources/BsResources.h"
#include "Image/BsTextureAtlasLayout.h"
#include "Private/RTTI/BsFontRTTI.h"
#include "BsCoreApplication.h"
#include "FileSystem/BsDataStream.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererManager.h"
#include "RenderAPI/BsGpuCommandBuffer.h"

#define USE_FREETYPE2_STATIC
#include <ft2build.h>
#include <freetype/freetype.h>
#include FT_FREETYPE_H

B3D_LOG_CATEGORY_STATIC(Font, Log)

using namespace bs;

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
	for(const auto& it : mFontBitmaps)
	{
		for(auto& page : it.second->TexturePages)
		{
			if(page.Texture != nullptr)
				AddResourceDependency(page.Texture);
		}
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
		B3D_LOG(Error, Font, "Failed to initialize font renderer. Font data is null.");
		return false;
	}

	static FT_Library library = nullptr;
	if(library == nullptr)
	{
		error = FT_Init_FreeType(&library);
		if(error)
		{
			B3D_LOG(Error, Font, "Failed to initialize font renderer. Error occurred during FreeType library initialization.");
			return false;
		}
	}

	mImplementation->Face = nullptr;
	error = FT_New_Memory_Face(library, (FT_Byte*)mInformation.FontData->Data(), (FT_Long)mInformation.FontData->Size(), 0, &mImplementation->Face);

	if(error == FT_Err_Unknown_File_Format)
	{
		B3D_LOG(Error, Font, "Failed to initialize font renderer. Unsupported file format.");
	}
	else if(error)
	{
		B3D_LOG(Error, Font, "Failed to initialize font renderer. Unknown error.");
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

bool Font::RenderGlyphs(float size, const TArrayView<u32>& characterIds)
{
	const float quantizedFontSize = GetQuantizedFontSize(size);

	SPtr<FontBitmapInformation> bitmapInformation = GetOrCreateBitmapInformationForSize(size);
	if(!bitmapInformation)
	{
		B3D_LOG(Error, Font, "Failed to render font glyphs. Unable to create bitmap information for requested font size.");
		return false;
	}

	if(mImplementation->Face == nullptr)
	{
		B3D_LOG(Error, Font, "Failed to render font glyphs. Font renderer is not initialized.");
		return nullptr;
	}

	if(!B3D_ENSURE(FT_Set_Char_Size(mImplementation->Face, ConvertFloatToFixed26Dot6(quantizedFontSize), 0, mInformation.DPI, mInformation.DPI) == 0))
	{
		B3D_LOG(Error, Font, "Failed to render font glyphs. Failed to set character size.");
		return nullptr;
	}

	Vector<GlyphBitmap> glyphBitmaps;

	const FT_Int32 loadFlags = ConvertFontRenderModeToLoadFlags(mInformation.RenderMode);
	const FT_Face& face = mImplementation->Face;
	for(const auto& characterId : characterIds)
	{
		auto it = bitmapInformation->Characters.find(characterId);
		if(it != bitmapInformation->Characters.end())
			continue;

		FT_Error error = FT_Load_Char(face, (FT_ULong)characterId, loadFlags);

		if(error)
		{
			B3D_LOG(Error, Font, "Failed to render font glyph '{0}'. Failed to load character.", characterId);
			continue;
		}

		FT_Render_Glyph(face->glyph, FT_LOAD_TARGET_MODE(loadFlags));

		if(error)
		{
			B3D_LOG(Error, Font, "Failed to render font glyph '{0}'. Failed to render character.", characterId);
			continue;
		}

		const FT_GlyphSlot& glyph = face->glyph;

		CharacterInformation characterInformation;
		characterInformation.CharId = characterId;
		characterInformation.Width = ConvertFixed26Dot6ToFloat(glyph->metrics.width);
		characterInformation.Height = ConvertFixed26Dot6ToFloat(glyph->metrics.height);
		characterInformation.XOffset = ConvertFixed26Dot6ToFloat(glyph->metrics.horiBearingX);
		characterInformation.YOffset = ConvertFixed26Dot6ToFloat(glyph->metrics.horiBearingY);
		characterInformation.XAdvance = ConvertFixed26Dot6ToFloat(glyph->metrics.horiAdvance);
		characterInformation.YAdvance = ConvertFixed26Dot6ToFloat(glyph->advance.y);

		// Parse kerning
		for(auto& keyValuePair : bitmapInformation->Characters)
		{
			const u32 otherCharacterId = keyValuePair.first;
			CharacterInformation& otherCharacterInformation = keyValuePair.second;

			auto fnAddKerning = [this, &face](CharacterInformation& leftCharacterInformation, u32 rightCharacterId) {
				FT_Vector kerning;

				const FT_Error error = FT_Get_Kerning(face, leftCharacterInformation.CharId, rightCharacterId, FT_KERNING_UNFITTED, &kerning);
				if(error)
				{
					B3D_LOG(Error, Font, "Failed to get kerning information for glyphs '{0}', '{1}'.",  leftCharacterInformation.CharId, rightCharacterId);
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

			fnAddKerning(characterInformation, otherCharacterId);
			fnAddKerning(otherCharacterInformation, characterId);
		}

		// Read pixels
		if(glyph->bitmap.buffer == nullptr && glyph->bitmap.rows > 0 && glyph->bitmap.width > 0)
		{
			B3D_LOG(Error, Font, "Failed to render glyph '{0}. Bitmap is empty.", characterId);
			continue;
		}

		u8* sourceBuffer = glyph->bitmap.buffer;

		const Size2UI bitmapSize((u32)glyph->bitmap.width, (u32)glyph->bitmap.rows);
		if(bitmapSize.Width == 0 || bitmapSize.Height == 0)
			continue;

		TextureCreateInformation textureCreateInformation;
		textureCreateInformation.Name = "FontGlyph";
		textureCreateInformation.Width = bitmapSize.Width;
		textureCreateInformation.Height = bitmapSize.Height;
		textureCreateInformation.Format = PF_R8;

		// Note: Probably not efficient creating a new texture for each glyph
		HTexture texture = Texture::Create(textureCreateInformation);

		const SPtr<PixelData> destinationPixelData = texture->GetProperties().AllocBuffer(0, 0);
		u8* destinationBuffer = destinationPixelData->GetData();

		if(glyph->bitmap.pixel_mode == ft_pixel_mode_grays)
		{
			for(u32 bitmapY = 0; bitmapY < bitmapSize.Height; bitmapY++)
			{
				memcpy(destinationBuffer, sourceBuffer, bitmapSize.Width);

				destinationBuffer += glyph->bitmap.width;
				sourceBuffer += glyph->bitmap.pitch;
			}
		}
		else if(glyph->bitmap.pixel_mode == ft_pixel_mode_mono)
		{
			// 8 pixels are packed into a byte, so do some unpacking
			for(u32 bitmapY = 0; bitmapY < bitmapSize.Height; bitmapY++)
			{
				for(u32 bitmapX = 0; bitmapX < bitmapSize.Width; bitmapX++)
				{
					const u8 sourceValue = sourceBuffer[bitmapX >> 3];
					destinationBuffer[bitmapX] = (sourceValue & (128 >> (bitmapX & 7))) != 0 ? 255 : 0;
				}

				destinationBuffer += glyph->bitmap.width;
				sourceBuffer += glyph->bitmap.pitch;
			}
		}
		else
		{
			B3D_LOG(Error, Font, "Failed to render glyph '{0}'. Unsupported pixel mode.", characterId);
			continue;
		}

		texture->WriteData(destinationPixelData);

		u32 firstDynamicPageIndex = 0;
		for(const auto& page : bitmapInformation->TexturePages)
		{
			if(page.IsDynamic)
				break;

			firstDynamicPageIndex++;
		}

		u32 targetPageIndex = ~0u;
		Optional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
		for(u32 pageIndex = firstDynamicPageIndex; pageIndex < (u32)bitmapInformation->TexturePages.size(); ++pageIndex)
		{
			FontBitmapPage& page = bitmapInformation->TexturePages[pageIndex];
			if(!B3D_ENSURE(page.IsDynamic))
				continue;

			layoutAllocation = page.Layout.AddElement(bitmapSize);
			if(!layoutAllocation)
				continue;

			targetPageIndex = pageIndex;
			break;
		}

		if(!layoutAllocation.has_value())
		{
			targetPageIndex = (u32)bitmapInformation->TexturePages.size();

			FontBitmapPage newPage;
			newPage.IsDynamic = true;

			// TODO - These textures should be shared between fonts. Its wasteful to allocate a 1024x1024 texture for each font+font size combination
			TextureCreateInformation pageTextureCreateInformation;
			pageTextureCreateInformation.Name = StringUtil::Format("Font Page Font: {0}, Size: {1}, Index:{2}", GetName(), bitmapInformation->Size, targetPageIndex);
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

			bitmapInformation->TexturePages.push_back(newPage);
		}

		if(!layoutAllocation || !B3D_ENSURE(targetPageIndex != ~0u))
		{
			B3D_LOG(Error, Font, "Failed to render glyph '{0}'. Failed to allocate a slot in the texture atlas.", characterId);
			continue;
		}

		B3D_ASSERT(layoutAllocation->PageId == 0);

		FontBitmapPage& page = bitmapInformation->TexturePages[targetPageIndex];

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
		characterInformation.UvWidth = inversePageWidth * (float)glyphBitmap.Size.Width;
		characterInformation.UvHeight = inversePageHeight * (float)glyphBitmap.Size.Height;

		bitmapInformation->Characters[characterId] = std::move(characterInformation);
		glyphBitmaps.push_back(std::move(glyphBitmap));
	}

	FontAtlasRenderer::Instance().BlitGlyphs(std::move(glyphBitmaps));
	return false;
}

void Font::Bake(bool clearFontData)
{
	for(const auto& bitmapInformationPair : mFontBitmaps)
	{
		if(!B3D_ENSURE(bitmapInformationPair.second))
			continue;

		for(auto& page : bitmapInformationPair.second->TexturePages)
		{
			page.IsDynamic = false;
			page.Layout.Clear();
		}
	}

	if(clearFontData)
	{
		DestroyFontRenderer();
		mInformation.FontData = nullptr;
	}
}

SPtr<FontBitmapInformation> Font::GetOrCreateBitmapInformationForSize(float size)
{
	const float quantizedFontSize = GetQuantizedFontSize(size);

	auto itFontBitmap = mFontBitmaps.find(quantizedFontSize);
	if(itFontBitmap != mFontBitmaps.end())
		return itFontBitmap->second;

	if(mImplementation->Face == nullptr)
	{
		B3D_LOG(Error, Font, "Cannot create font bitmap information. Font renderer is not initialized.");
		return nullptr;
	}

	if(!B3D_ENSURE(FT_Set_Char_Size(mImplementation->Face, ConvertFloatToFixed26Dot6(quantizedFontSize), 0, mInformation.DPI, mInformation.DPI) == 0))
	{
		B3D_LOG(Error, Font, "Cannot create font bitmap information. Failed to set character size.");
		return nullptr;
	}

	SPtr<FontBitmapInformation> newBitmapInformation = B3DMakeShared<FontBitmapInformation>();
	const FT_Int32 loadFlags = ConvertFontRenderModeToLoadFlags(mInformation.RenderMode);

	const FT_Size_Metrics& faceMetrics = mImplementation->Face->size->metrics;

	newBitmapInformation->Size = quantizedFontSize;
	newBitmapInformation->LineHeight = ConvertFixed26Dot6ToFloat(faceMetrics.height);
	newBitmapInformation->BaselineOffset = ConvertFixed26Dot6ToFloat(faceMetrics.height + faceMetrics.descender);

	constexpr FT_ULong kSpaceCharacterId = 32;
	FT_Load_Char(mImplementation->Face, kSpaceCharacterId, loadFlags);

	const FT_GlyphSlot& glyph = mImplementation->Face->glyph;
	newBitmapInformation->SpaceWidth = ConvertFixed26Dot6ToFloat(glyph->advance.x);

	mFontBitmaps[quantizedFontSize] = newBitmapInformation;

	u32 kMissingGlyphId[] = { 0 };
	RenderGlyphs(size, TArrayView(kMissingGlyphId, 1));

	auto found = newBitmapInformation->Characters.find(0);
	if(B3D_ENSURE(found != newBitmapInformation->Characters.end()))
		newBitmapInformation->MissingGlyph = found->second;

	return newBitmapInformation;
}

SPtr<FontBitmapInformation> Font::GetBitmap(float size) const
{
	const float quantizedFontSize = GetQuantizedFontSize(size);
	auto itFound = mFontBitmaps.find(quantizedFontSize);

	if(itFound == mFontBitmaps.end())
		return nullptr;

	return itFound->second;
}

float Font::GetClosestSize(float size) const
{
	float minDiff = std::numeric_limits<float>::max();
	float bestSize = size;

	for(auto iter = mFontBitmaps.begin(); iter != mFontBitmaps.end(); ++iter)
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

void Font::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	for(auto& fontDataEntry : mFontBitmaps)
	{
		for(auto& page : fontDataEntry.second->TexturePages)
		{
			if(page.Texture.IsLoaded())
				dependencies.push_back(page.Texture.Get());
		}
	}
}

float Font::GetQuantizedFontSize(float size)
{
	return (float)Math::RoundToI32(size * kFontQuantizeAmount) / (float)kFontQuantizeAmount;
}

HFont Font::Create(const FontCreateInformation& createInformation)
{
	SPtr<Font> newFont = CreateShared(createInformation);

	return B3DStaticResourceCast<Font>(GetResources().CreateResourceHandle(newFont));
}

SPtr<Font> Font::CreateShared(const FontCreateInformation& createInformation)
{
	SPtr<Font> newFont = B3DMakeSharedFromExisting<Font>(new(B3DAllocate<Font>()) Font(createInformation));
	newFont->SetShared(newFont);
	newFont->Initialize();

	return newFont;
}

SPtr<Font> Font::CreateEmpty()
{
	SPtr<Font> newFont = B3DMakeSharedFromExisting<Font>(new(B3DAllocate<Font>()) Font(FontCreateInformation()));
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

void FontAtlasRenderer::OnStartUp()
{
	SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if (!gpuDevice)
		return;

	auto fnCreateCommandBufferPool = [this, gpuDevice]()
	{
		ct::GpuCommandBufferPoolCreateInformation poolCreateInformation;
		poolCreateInformation.Thread = B3D_CURRENT_THREAD_ID;
		poolCreateInformation.Usage = GQT_GRAPHICS;

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

		const SPtr<ct::GpuCommandBuffer> commandBuffer = mCommandBufferPool->Create(ct::GpuCommandBufferCreateInformation::Create("BlitGlyphBitmaps"));

		for(const auto& entry : glyphBitmaps)
		{
			TextureBlitInformation blitInformation;
			blitInformation.DestinationVolume.Left = entry.PositionInAtlas.X;
			blitInformation.DestinationVolume.Top = entry.PositionInAtlas.Y;
			blitInformation.DestinationVolume.Right = entry.PositionInAtlas.X + (i32)entry.Size.Width;
			blitInformation.DestinationVolume.Bottom = entry.PositionInAtlas.Y + (i32)entry.Size.Height;
			blitInformation.DestinationVolume.Back = 1;

			entry.GlyphTexture->Blit(*commandBuffer, entry.AtlasTexture, blitInformation);
		}

		const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
		if(!B3D_ENSURE(gpuDevice))
			return;

		gpuDevice->SubmitCommandBuffer(commandBuffer);
	};

	GetRenderThread().PostCommand(fnBlitToAtlas, "FontAtlasRenderer::BlitGlyphs");
}

