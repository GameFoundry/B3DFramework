//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShaderManager.h"
#include "BsResources.h"
#include "BsImporter.h"

namespace bs
{
	HShaderInclude DefaultShaderIncludeHandler::findInclude(const String& name) const
	{
		return Importer::instance().import<ShaderInclude>(name);
	}

	HShaderInclude ShaderManager::findInclude(const String& name) const
	{
		return mIncludeHandler->findInclude(name);
	}
}