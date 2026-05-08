//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DPlainTextImporter.h"
#include "Resources/B3DPlainText.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

bool PlainTextImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtility::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"txt" || lowerCaseExt == u8"xml" || lowerCaseExt == u8"json";
}

bool PlainTextImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // Plain-text so we don't even check for magic number
}

TShared<Resource> PlainTextImporter::Import(const Path& filePath, TShared<const ImportOptions> importOptions)
{
	WString textData;
	{
		TShared<DataStream> stream = FileSystem::OpenFile(filePath);
		textData = stream->GetAsWString();
	}

	TShared<PlainText> plainText = PlainText::CreatePtrInternal(textData);

	String fileName = filePath.GetFilename(false);
	plainText->SetName(fileName);

	return plainText;
}
