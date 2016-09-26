//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorShaderIncludeHandler.h"
#include "BsProjectLibrary.h"
#include "BsResources.h"
#include "BsProjectResourceMeta.h"
#include "BsBuiltinResources.h"
#include "BsBuiltinEditorResources.h"

namespace BansheeEngine
{
	HShaderInclude EditorShaderIncludeHandler::findInclude(const String& name) const
	{
		Path path = toResourcePath(name);

		if (path.isEmpty())
			return HShaderInclude();

		ResourceLoadFlags loadFlags = ResourceLoadFlag::Default | ResourceLoadFlag::KeepSourceData;
		if (name.size() >= 8)
		{
			if (name.substr(0, 8) == "$ENGINE$" || name.substr(0, 8) == "$EDITOR$")
				return static_resource_cast<ShaderInclude>(Resources::instance().load(path, loadFlags));
		}

		ProjectLibrary::LibraryEntry* entry = gProjectLibrary().findEntry(path);
		if (entry != nullptr && entry->type == ProjectLibrary::LibraryEntryType::File)
		{
			ProjectLibrary::FileEntry* fileEntry = static_cast<ProjectLibrary::FileEntry*>(entry);

			if (fileEntry->meta != nullptr)
			{
				auto& resourceMetas = fileEntry->meta->getResourceMetaData();
				for(auto& resMeta : resourceMetas)
				{
					if(resMeta->getTypeID() == TID_ShaderInclude)
						return static_resource_cast<ShaderInclude>(Resources::instance().loadFromUUID(resMeta->getUUID(), false, loadFlags));
				}
			}
		}

		return HShaderInclude();
	}

	Path EditorShaderIncludeHandler::toResourcePath(const String& name)
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
		else if (name.substr(0, 8) == "$EDITOR$")
		{
			if (name.size() > 8)
			{
				Path fullPath = BuiltinEditorResources::getShaderIncludeFolder();
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