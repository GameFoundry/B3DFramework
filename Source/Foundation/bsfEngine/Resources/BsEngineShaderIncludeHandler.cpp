//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsEngineShaderIncludeHandler.h"

#include "FileSystem/BsDataStream.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Importer/BsImporter.h"
#include "FileSystem/BsFileSystem.h"

using namespace bs;

HShaderInclude EngineShaderIncludeHandler::FindInclude(const String& name) const
{
	const Path path = DetermineFullPath(name);

	if(path.IsEmpty())
		return HShaderInclude();

	return GetImporter().Import<ShaderInclude>(path);
}

Optional<String> EngineShaderIncludeHandler::FindIncludeSource(const String& name) const
{
	const Path path = DetermineFullPath(name);

	if(path.IsEmpty())
		return {};
	
	if(FileSystem::IsFile(path))
	{
		Lock fileLock = FileScheduler::GetLock(path);

		if(const SPtr<DataStream> stream = FileSystem::OpenFile(path))
			return stream->GetAsString();
	}

	return {};
}

Path EngineShaderIncludeHandler::DetermineFullPath(const String& name) const
{
	if(name.size() >= 8 && name.substr(0, 8) == "$ENGINE$")
	{
		Path fullPath = BuiltinResources::GetRawShaderIncludeFolder();
		if(name.size() > 8)
		{
			Path includePath = name.substr(9, name.size() - 9);
			fullPath.Append(includePath);
		}

		return fullPath;
	}
#ifdef BS_IS_ASSET_TOOL
	else if(name.size() >= 8 && name.substr(0, 8) == "$EDITOR$")
	{
		Path fullPath = BuiltinResources::GetEditorRawShaderIncludeFolder();
		if(name.size() > 8)
		{
			Path includePath = name.substr(9, name.size() - 9);
			fullPath.append(includePath);
		}

		return fullPath;
	}
#endif
	else
	{
		Path fullPath = name;
		for(auto& folder : mSearchPaths)
		{
			Path entry = folder;
			entry.Append(name);

			if(FileSystem::Exists(entry))
			{
				fullPath = entry;
				break;
			}
		}

		return Paths::FindPath(fullPath);
	}
}
