//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShaderManager.h"
#include "Resources/BsResources.h"
#include "Importer/BsImporter.h"

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

	void ShaderManager::addSearchPath(const Path& path)
	{
		if(mIncludeHandler)
			mIncludeHandler->addSearchPath(path);
	}
}