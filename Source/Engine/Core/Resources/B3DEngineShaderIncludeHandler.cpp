//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DEngineShaderIncludeHandler.h"

#include "FileSystem/B3DDataStream.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResources.h"
#include "Importer/B3DImporter.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

HShaderInclude EngineShaderIncludeHandler::FindInclude(const String& name) const
{
	const Path path = DetermineFullPath(name);

	if(path.IsEmpty())
		return HShaderInclude();

	return GetImporter().Import<ShaderInclude>(path);
}

TOptional<String> EngineShaderIncludeHandler::FindIncludeSource(const String& name) const
{
	const Path path = DetermineFullPath(name);

	if(path.IsEmpty())
		return {};
	
	if(FileSystem::IsFile(path))
	{
		if(const SPtr<DataStream> stream = FileSystem::OpenFile(path))
			return stream->GetAsString();
	}

	return {};
}

Path EngineShaderIncludeHandler::DetermineFullPath(const String& name) const
{
	if(name.size() >= 8 && name.substr(0, 8) == "$ENGINE$")
	{
		Path fullPath = BuiltinResources::GetShaderIncludeFolder();
		if(name.size() > 8)
		{
			Path includePath = name.substr(9, name.size() - 9);
			fullPath.Append(includePath);
		}

		return fullPath;
	}
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
