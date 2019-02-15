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

	void ScriptBuiltin::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetWhiteTexture", (void*)&ScriptBuiltin::internal_GetWhiteTexture);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinShader", (void*)&ScriptBuiltin::internal_GetBuiltinShader);
		metaData.scriptClass->addInternalCall("Internal_GetMesh", (void*)&ScriptBuiltin::internal_GetMesh);
		metaData.scriptClass->addInternalCall("Internal_GetDefaultFont", (void*)&ScriptBuiltin::internal_GetDefaultFont);
	}

	MonoObject* ScriptBuiltin::internal_GetWhiteTexture()
	{
		HSpriteTexture whiteTexture = BuiltinResources::instance().getWhiteSpriteTexture();

		ScriptResourceBase* scriptSpriteTex = ScriptResourceManager::instance().getScriptResource(whiteTexture, true);
		return scriptSpriteTex->getManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetBuiltinShader(BuiltinShader type)
	{
		HShader diffuseShader = BuiltinResources::instance().getBuiltinShader(type);

		ScriptResourceBase* scriptShader = ScriptResourceManager::instance().getScriptResource(diffuseShader, true);
		return scriptShader->getManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetMesh(BuiltinMesh meshType)
	{
		HMesh mesh = BuiltinResources::instance().getMesh(meshType);

		ScriptResourceBase* scriptMesh = ScriptResourceManager::instance().getScriptResource(mesh, true);
		return scriptMesh->getManagedInstance();
	}

	MonoObject* ScriptBuiltin::internal_GetDefaultFont()
	{
		HFont font = BuiltinResources::instance().getDefaultFont();

		ScriptResourceBase* scriptFont = ScriptResourceManager::instance().getScriptResource(font, true);
		return scriptFont->getManagedInstance();
	}
}