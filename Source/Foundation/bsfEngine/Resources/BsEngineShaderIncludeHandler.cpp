//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Importer/BsImporter.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	HShaderInclude EngineShaderIncludeHandler::FindInclude(const String& name) const
	{
		Path path = ToResourcePath(name);

		if (path.IsEmpty())
			return HShaderInclude();

		if (name.size() >= 8)
		{
			if (name.substr(0, 8) == "$ENGINE$" || name.substr(0, 8) == "$EDITOR$")
				return static_resource_cast<ShaderInclude>(Resources::Instance().Load(path));
		}

		for(auto& folder : mSearchPaths)
		{
			Path entry = folder;
			entry.Append(name);

			if(FileSystem::Exists(entry))
			{
				path = entry;
				break;
			}
		}

		path = Paths::FindPath(path);
		return Importer::Instance().Import<ShaderInclude>(path);
	}

	Path EngineShaderIncludeHandler::ToResourcePath(const String& name)
	{
		if (name.substr(0, 8) == "$ENGINE$")
		{
			if (name.size() > 8)
			{
				Path fullPath = BuiltinResources::GetShaderIncludeFolder();
				Path includePath = name.substr(9, name.size() - 9);

				fullPath.Append(includePath);
				fullPath.SetFilename(includePath.GetFilename() + ".asset");

				return fullPath;
			}
		}
#ifdef BS_IS_ASSET_TOOL
		else if (name.substr(0, 8) == "$EDITOR$")
		{
			if (name.size() > 8)
			{
				Path fullPath = BuiltinResources::getEditorShaderIncludeFolder();
				Path includePath = name.substr(9, name.size() - 9);

				fullPath.append(includePath);
				fullPath.setFilename(includePath.getFilename() + ".asset");

				return fullPath;
			}
		}
#endif

		return name;
	}
}
