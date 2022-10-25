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

		ScriptResourceBase* scriptSpriteTex = ScriptResourceManager::Instance().GetScriptResource(whiteTexture, true);
		return scriptSpriteTex->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::InternalGetBuiltinShader(BuiltinShader type)
	{
		HShader diffuseShader = BuiltinResources::Instance().GetBuiltinShader(type);

		ScriptResourceBase* scriptShader = ScriptResourceManager::Instance().GetScriptResource(diffuseShader, true);
		return scriptShader->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::InternalGetMesh(BuiltinMesh meshType)
	{
		HMesh mesh = BuiltinResources::Instance().GetMesh(meshType);

		ScriptResourceBase* scriptMesh = ScriptResourceManager::Instance().GetScriptResource(mesh, true);
		return scriptMesh->GetManagedInstance();
	}

	MonoObject* ScriptBuiltin::InternalGetDefaultFont()
	{
		HFont font = BuiltinResources::Instance().GetDefaultFont();

		ScriptResourceBase* scriptFont = ScriptResourceManager::Instance().GetScriptResource(font, true);
		return scriptFont->GetManagedInstance();
	}
} // namespace bs
