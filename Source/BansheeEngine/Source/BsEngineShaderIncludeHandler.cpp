//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEngineShaderIncludeHandler.h"
#include "BsResources.h"
#include "BsBuiltinResources.h"
#include "BsImporter.h"

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

		return Importer::instance().import<ShaderInclude>(name);
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
		else
		{
			return name;
		}

		return Path::BLANK;
	}
}