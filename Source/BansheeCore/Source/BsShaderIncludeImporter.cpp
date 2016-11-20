//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShaderIncludeImporter.h"
#include "BsShaderInclude.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"

namespace bs
{
	ShaderIncludeImporter::ShaderIncludeImporter()
		:SpecificImporter()
	{

	}

	ShaderIncludeImporter::~ShaderIncludeImporter()
	{

	}

	bool ShaderIncludeImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"bslinc";
	}

	bool ShaderIncludeImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so I don't even check for magic number
	}

	SPtr<Resource> ShaderIncludeImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<DataStream> stream = FileSystem::openFile(filePath);
		String includeString = stream->getAsString();

		SPtr<ShaderInclude> gpuInclude = ShaderInclude::_createPtr(includeString);

		WString fileName = filePath.getWFilename(false);
		gpuInclude->setName(fileName);

		return gpuInclude;
	}
}