//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptBuiltin.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Resources/BsBuiltinResources.h"
#include "BsScriptResourceManager.h"

#include "Generated/BsScriptFont.generated.h"
#include "Generated/BsScriptSpriteTexture.generated.h"

namespace bs
{
	ScriptBuiltin::ScriptBuiltin(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBuiltin::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetWhiteTexture", (void*)&ScriptBuiltin::internal_GetWhiteTexture);
		metaData.scriptClass->AddInternalCall("Internal_GetBuiltinShader", (void*)&ScriptBuiltin::internal_GetBuiltinShader);
		metaData.scriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptBuiltin::internal_GetMesh);
		metaData.scriptClass->AddInternalCall("Internal_GetDefaultFont", (void*)&ScriptBuiltin::internal_GetDefaultFont);
	}

	MonoObject* ScriptBuiltin::internal_GetWhiteTexture()
	{
		HSpriteTexture whiteTexture = BuiltinResources::instance().GetWhiteSpriteTexture();

		ScriptResourceBase* scriptSpriteTex = ScriptResourceManager::instance().GetScriptResource(whiteTexture, true);
		return scriptSpriteTex->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetBuiltinShader(BuiltinShader type)
	{
		HShader diffuseShader = BuiltinResources::instance().GetBuiltinShader(type);

		ScriptResourceBase* scriptShader = ScriptResourceManager::instance().GetScriptResource(diffuseShader, true);
		return scriptShader->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetMesh(BuiltinMesh meshType)
	{
		HMesh mesh = BuiltinResources::instance().GetMesh(meshType);

		ScriptResourceBase* scriptMesh = ScriptResourceManager::instance().GetScriptResource(mesh, true);
		return scriptMesh->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetDefaultFont()
	{
		HFont font = BuiltinResources::instance().GetDefaultFont();

		ScriptResourceBase* scriptFont = ScriptResourceManager::instance().GetScriptResource(font, true);
		return scriptFont->GetManagedInstance();
	}
}
