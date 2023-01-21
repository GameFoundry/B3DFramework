//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSLImporter.h"
#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"
#include "BsBSLCompiler.h"
#include "Importer/BsShaderImportOptions.h"

using namespace bs;

bool SLImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtil::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"bsl";
}

bool SLImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // Plain-text so I don't even check for magic number
}

SPtr<Resource> SLImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	String source;
	{
		Lock fileLock = FileScheduler::GetLock(filePath);

		SPtr<DataStream> stream = FileSystem::OpenFile(filePath);
		source = stream->GetAsString();
	}

	SPtr<const ShaderImportOptions> io = std::static_pointer_cast<const ShaderImportOptions>(importOptions);
	const String shaderName = filePath.GetFilename(false);
	SPtr<Shader> shader;
	BSLResult result = BSLCompiler::Compile(shaderName, source, io->GetDefines(), io->Languages, shader);

	if(shader != nullptr)
		shader->SetName(shaderName);

	if(!result.ErrorMessage.empty())
	{
		String file;
		if(result.ErrorFile.empty())
			file = filePath.ToString();
		else
			file = result.ErrorFile;

		B3D_LOG(Error, BSLCompiler, "Compilation error when importing shader \"{0}\":\n{1}. Location: {2} ({3})", file, result.ErrorMessage, result.ErrorLine, result.ErrorColumn);
	}

	return shader;
}

SPtr<ImportOptions> SLImporter::CreateImportOptions() const
{
	return B3DMakeShared<ShaderImportOptions>();
}
