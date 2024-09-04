//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptBuiltin.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Resources/BsBuiltinResources.h"
#include "BsScriptResourceManager.h"
#include "Text/BsFont.h"
#include "Material/BsShader.h"
#include "Mesh/BsMesh.h"

#include "Generated/BsScriptFont.generated.h"
#include "Generated/BsScriptSpriteTexture.generated.h"

using namespace bs;
ScriptBuiltin::ScriptBuiltin(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptBuiltin::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetWhiteTexture", (void*)&ScriptBuiltin::InternalGetWhiteTexture);
	metaData.ScriptClass->AddInternalCall("Internal_GetBuiltinShader", (void*)&ScriptBuiltin::InternalGetBuiltinShader);
	metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptBuiltin::InternalGetMesh);
	metaData.ScriptClass->AddInternalCall("Internal_GetDefaultFont", (void*)&ScriptBuiltin::InternalGetDefaultFont);
}

MonoObject* ScriptBuiltin::InternalGetWhiteTexture()
{
	HSpriteTexture whiteTexture = BuiltinResources::Instance().GetWhiteSpriteTexture();

	return ScriptResourceWrapper::GetOrCreateScriptObject(whiteTexture);
}

MonoObject* ScriptBuiltin::InternalGetBuiltinShader(BuiltinShader type)
{
	HShader diffuseShader = BuiltinResources::Instance().GetBuiltinShader(type);

	return ScriptResourceWrapper::GetOrCreateScriptObject(diffuseShader);
}

MonoObject* ScriptBuiltin::InternalGetMesh(BuiltinMesh meshType)
{
	HMesh mesh = BuiltinResources::Instance().GetMesh(meshType);

	return ScriptResourceWrapper::GetOrCreateScriptObject(mesh);
}

MonoObject* ScriptBuiltin::InternalGetDefaultFont()
{
	HFont font = BuiltinResources::Instance().GetDefaultFont();

	return ScriptResourceWrapper::GetOrCreateScriptObject(font);
}
