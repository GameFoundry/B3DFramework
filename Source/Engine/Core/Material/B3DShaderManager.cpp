//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderManager.h"

#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Resources/B3DResources.h"
#include "Importer/B3DImporter.h"

using namespace b3d;

HShaderInclude DefaultShaderIncludeHandler::FindInclude(const String& name) const
{
	return Importer::Instance().Import<ShaderInclude>(name);
}

TOptional<String> DefaultShaderIncludeHandler::FindIncludeSource(const String& name) const
{
	const Path filePath = FileSystem::GetExecutableFolderPath() + name;
	if(FileSystem::IsFile(filePath))
	{
		if(const TShared<DataStream> stream = FileSystem::OpenFile(filePath))
			return stream->GetAsString();
	}

	return nullptr;
}

HShaderInclude ShaderManager::FindInclude(const String& name) const
{
	if(!mIncludeHandler)
		return nullptr;

	return mIncludeHandler->FindInclude(name);
}

TOptional<String> ShaderManager::FindIncludeSource(const String& name) const
{
	if(!mIncludeHandler)
		return nullptr;

	return mIncludeHandler->FindIncludeSource(name);
}

void ShaderManager::AddSearchPath(const Path& path)
{
	if(mIncludeHandler)
		mIncludeHandler->AddSearchPath(path);
}
