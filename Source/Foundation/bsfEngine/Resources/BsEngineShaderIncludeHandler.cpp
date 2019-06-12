//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Importer/BsImporter.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	HShaderInclude EngineShaderIncludeHandler::findInclude(const String& name) const
	{
		Path path = toResourcePath(name);

		if (path.isEmpty())
			return HShaderInclude();

		if (name.size() >= 8)
		{
			if (name.substr(0, 8) == "$ENGINE$")
				return static_resource_cast<ShaderInclude>(Resources::instance().load(path));
		}

		for(auto& folder : mSearchPaths)
		{
			Path entry = folder;
			entry.append(name);

			if(FileSystem::exists(entry))
			{
				path = entry;
				break;
			}
		}

		path = Paths::findPath(path);
		return Importer::instance().import<ShaderInclude>(path);
	}

	Path EngineShaderIncludeHandler::toResourcePath(const String& name)
	{
		if (name.substr(0, 8) == "$ENGINE$")
		{
			if (name.size() > 8)
			{
				Path fullPath = BuiltinResources::getShaderIncludeFolder();
				Path includePath = name.substr(9, name.size() - 9);

				fullPath.append(includePath);
				fullPath.setFilename(includePath.getFilename() + ".asset");

				return fullPath;
			}
		}

		return name;
	}
}
