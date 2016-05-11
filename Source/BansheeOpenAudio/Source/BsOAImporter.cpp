//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAImporter.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"
#include "BsOAWaveReader.h"
#include "BsOAFLACReader.h"
#include "BsOAOggVorbisReader.h"

namespace BansheeEngine
{
	OAImporter::OAImporter()
		:SpecificImporter()
	{

	}

	OAImporter::~OAImporter()
	{

	}

	bool OAImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"wav" || lowerCaseExt == L"flac" || lowerCaseExt == L"ogg";
	}

	bool OAImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		// Don't check for magic number, rely on extension
		return true;
	}

	SPtr<Resource> OAImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<DataStream> stream = FileSystem::openFile(filePath);

		WString extension = filePath.getWExtension();
		StringUtil::toLowerCase(extension);

		// TODO - Respect import options and read mode
		AudioFileInfo info;
		UINT8* samples = nullptr;
		if(extension == L"wav")
		{
			OAWaveReader reader;
			if (!reader.isValid(stream))
				return nullptr;

			if (!reader.open(stream, info))
				return nullptr;

			//samples = bs_alloc(info.)
			//reader.read()
		}
		else if(extension == L"flac")
		{
			
		}
		else if(extension == L"ogg")
		{
			
		}

		return nullptr;
	}
}