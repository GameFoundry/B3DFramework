//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBuiltinResources.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/Resources/BsBuiltinResources.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptSpriteTexture.generated.h"
#include "BsScriptFont.generated.h"
#include "BsScriptShader.generated.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptMesh.generated.h"

namespace b3d
{
	ScriptBuiltinResources::ScriptBuiltinResources()
		:TScriptTypeDefinition()
	{
	}

	void ScriptBuiltinResources::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWhiteSpriteTexture", (void*)&ScriptBuiltinResources::InternalGetWhiteSpriteTexture);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBuiltinShader", (void*)&ScriptBuiltinResources::InternalGetBuiltinShader);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptBuiltinResources::InternalGetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShader", (void*)&ScriptBuiltinResources::InternalGetShader);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFont", (void*)&ScriptBuiltinResources::InternalGetFont);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrCompileShader", (void*)&ScriptBuiltinResources::InternalGetOrCompileShader);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDefaultFont", (void*)&ScriptBuiltinResources::InternalGetDefaultFont);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTexture", (void*)&ScriptBuiltinResources::InternalGetTexture);

	}

	MonoObject* ScriptBuiltinResources::InternalGetWhiteSpriteTexture()
	{
		TResourceHandle<SpriteTexture> tmp__output;
		tmp__output = BuiltinResources::Instance().GetWhiteSpriteTexture();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetBuiltinShader(BuiltinShader type)
	{
		TResourceHandle<Shader> tmp__output;
		tmp__output = BuiltinResources::Instance().GetBuiltinShader(type);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetMesh(BuiltinMesh mesh)
	{
		TResourceHandle<Mesh> tmp__output;
		tmp__output = BuiltinResources::Instance().GetMesh(mesh);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetShader(MonoString* path)
	{
		TResourceHandle<Shader> tmp__output;
		Path tmppath;
		tmppath = MonoUtil::MonoToString(path);
		tmp__output = BuiltinResources::Instance().GetShader(tmppath);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetFont(MonoString* font)
	{
		TResourceHandle<Font> tmp__output;
		String tmpfont;
		tmpfont = MonoUtil::MonoToString(font);
		tmp__output = BuiltinResources::Instance().GetFont(tmpfont);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetOrCompileShader(MonoString* path)
	{
		TResourceHandle<Shader> tmp__output;
		Path tmppath;
		tmppath = MonoUtil::MonoToString(path);
		tmp__output = BuiltinResources::Instance().GetOrCompileShader(tmppath);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetDefaultFont()
	{
		TResourceHandle<Font> tmp__output;
		tmp__output = BuiltinResources::Instance().GetDefaultFont();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptBuiltinResources::InternalGetTexture(BuiltinTexture type)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = BuiltinResources::GetTexture(type);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}
}
