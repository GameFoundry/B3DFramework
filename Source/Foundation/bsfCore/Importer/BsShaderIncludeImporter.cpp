//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsShaderIncludeImporter.h"
#include "Material/BsShaderInclude.h"
#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"

using namespace bs;

bool ShaderIncludeImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtil::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"bslinc";
}

bool ShaderIncludeImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // Plain-text so I don't even check for magic number
}

SPtr<Resource> ShaderIncludeImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	String includeString;
	{
		Lock fileLock = FileScheduler::GetLock(filePath);

		SPtr<DataStream> stream = FileSystem::OpenFile(filePath);
		includeString = stream->GetAsString();
	}

	SPtr<ShaderInclude> gpuInclude = ShaderInclude::CreatePtrInternal(includeString);

	const String fileName = filePath.GetFilename(false);
	gpuInclude->SetName(fileName);

	return gpuInclude;
}
