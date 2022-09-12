//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsPlainTextImporter.h"
#include "Resources/BsPlainText.h"
#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	bool PlainTextImporter::IsExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == u8"txt" || lowerCaseExt == u8"xml" || lowerCaseExt == u8"json";
	}

	bool PlainTextImporter::IsMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so we don't even check for magic number
	}

	SPtr<Resource> PlainTextImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		WString textData;
		{
			Lock fileLock = FileScheduler::getLock(filePath);

			SPtr<DataStream> stream = FileSystem::openFile(filePath);
			textData = stream->GetAsWString();
		}

		SPtr<PlainText> plainText = PlainText::_createPtr(textData);

		String fileName = filePath.GetFilename(false);
		plainText->SetName(fileName);

		return plainText;
	}
}
