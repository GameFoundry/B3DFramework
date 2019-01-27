//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptShaderInclude.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptShaderInclude::ScriptShaderInclude(MonoObject* instance, const HShaderInclude& shaderInclude)
		:TScriptResource(instance, shaderInclude)
	{

	}

	void ScriptShaderInclude::initRuntimeData()
	{

	}

	MonoObject* ScriptShaderInclude::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}