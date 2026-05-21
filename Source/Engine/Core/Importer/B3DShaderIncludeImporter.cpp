//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/B3DShaderIncludeImporter.h"
#include "Material/B3DShaderInclude.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

bool ShaderIncludeImporter::IsExtensionSupported(const String& extension) const
{
	String lowerCaseExtension = extension;
	StringUtility::ToLowerCase(lowerCaseExtension);

	return lowerCaseExtension == u8"bslinc";
}

bool ShaderIncludeImporter::IsMagicNumberSupported(const u8* magicNumber, u32 magicNumberSize) const
{
	return true; // Plain-text so I don't even check for magic number
}

TShared<Resource> ShaderIncludeImporter::Import(const Path& filePath, TShared<const ImportOptions> importOptions)
{
	String includeString;
	{
		TShared<DataStream> stream = FileSystem::OpenFile(filePath);
		includeString = stream->GetAsString();
	}

	TShared<ShaderInclude> gpuInclude = ShaderInclude::CreateShared(includeString);

	const String fileName = filePath.GetFilename(false);
	gpuInclude->SetName(fileName);

	return gpuInclude;
}
