//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptShaderInclude.h"
#include "BsMonoManager.h"

using namespace bs;
ScriptShaderInclude::ScriptShaderInclude(const HShaderInclude& nativeObject, MonoObject* scriptObject)
	: TScriptResourceWrapper(nativeObject, scriptObject)
{
	RegisterEvents();
}

void ScriptShaderInclude::SetupScriptBindings()
{
}

MonoObject* ScriptShaderInclude::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

ShaderInclude* ScriptShaderInclude::GetNativeObject() const
{
	return static_cast<ShaderInclude*>(TScriptResourceWrapper::GetNativeObject());
}
