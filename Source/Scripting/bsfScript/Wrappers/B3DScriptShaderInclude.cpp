//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptShaderInclude.h"
#include "B3DMonoManager.h"

using namespace b3d;
ScriptShaderInclude::ScriptShaderInclude(const HShaderInclude& nativeObject)
	: TScriptResourceWrapper(nativeObject)
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
