//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFontImporter.h"
#include "Text/B3DFontImportOptions.h"
#include "Image/B3DTexture.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include FT_FREETYPE_H

using namespace b3d;

FontImporter::FontImporter()
	: SpecificImporter()
{
	mExtensions.push_back(u8"ttf");
	mExtensions.push_back(u8"otf");
}

bool FontImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtility::ToLowerCase(lowerCaseExt);

	return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
}

bool FontImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	// Not used
	return false;
}

TShared<ImportOptions> FontImporter::CreateImportOptions() const
{
	return B3DMakeShared<FontImportOptions>();
}

TShared<Resource> FontImporter::Import(const Path& filePath, TShared<const ImportOptions> importOptions)
{
	const FontImportOptions* fontImportOptions = static_cast<const FontImportOptions*>(importOptions.get());

	const Vector<CharRange>& charIndexRanges = fontImportOptions->CharIndexRanges;
	const Vector<float>& fontSizes = fontImportOptions->FontSizes;

	const String fileName = filePath.GetFilename(false);

	FontCreateInformation fontCreateInformation;
	fontCreateInformation.Name = fileName;
	fontCreateInformation.DPI = fontImportOptions->Dpi;
	fontCreateInformation.RenderMode = fontImportOptions->RenderMode;

	TShared<DataStream> fontDataStream = FileSystem::OpenFile(filePath);
	if(fontDataStream == nullptr)
		return nullptr;

	fontCreateInformation.FontData = B3DMakeShared<MemoryDataStream>(fontDataStream);

	FrameAllocatorScope frameScope;
	FrameVector<u32> charactersToRender;

	TShared<Font> font = Font::CreateShared(fontCreateInformation);
	for(size_t sizeIndex = 0; sizeIndex < fontSizes.size(); sizeIndex++)
	{
		for(const auto& characterIndexRange : charIndexRanges)
		{
			for(u32 characterIndex = characterIndexRange.Start; characterIndex <= characterIndexRange.End; ++characterIndex)
				charactersToRender.push_back(characterIndex);

			font->RenderGlyphs(fontSizes[sizeIndex], charactersToRender, true);
			charactersToRender.clear();
		}
	}

	return font;
}
