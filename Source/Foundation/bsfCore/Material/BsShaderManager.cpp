//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShaderManager.h"
#include "Resources/BsResources.h"
#include "Importer/BsImporter.h"

using namespace bs;

HShaderInclude DefaultShaderIncludeHandler::FindInclude(const String& name) const
{
	return Importer::Instance().Import<ShaderInclude>(name);
}

HShaderInclude ShaderManager::FindInclude(const String& name) const
{
	return mIncludeHandler->FindInclude(name);
}

void ShaderManager::AddSearchPath(const Path& path)
{
	if(mIncludeHandler)
		mIncludeHandler->AddSearchPath(path);
}
