//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptShaderInclude.h"
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