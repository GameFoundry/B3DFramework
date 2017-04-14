//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptBuiltin.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsBuiltinResources.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptShader.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptResourceManager.h"

namespace bs
{
	ScriptBuiltin::ScriptBuiltin(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBuiltin::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetWhiteTexture", &ScriptBuiltin::internal_GetWhiteTexture);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinShader", &ScriptBuiltin::internal_GetBuiltinShader);
		metaData.scriptClass->addInternalCall("Internal_GetMesh", &ScriptBuiltin::internal_GetMesh);
		metaData.scriptClass->addInternalCall("Internal_GetDefaultFont", &ScriptBuiltin::internal_GetDefaultFont);
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