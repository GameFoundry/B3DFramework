//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptProjectSettings.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsEditorApplication.h"
#include "BsProjectSettings.h"

namespace bs
{
	ScriptProjectSettings::ScriptProjectSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptProjectSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetLastOpenScene", &ScriptProjectSettings::internal_GetLastOpenScene);
		metaData.scriptClass->addInternalCall("Internal_SetLastOpenScene", &ScriptProjectSettings::internal_SetLastOpenScene);
		metaData.scriptClass->addInternalCall("Internal_SetFloat", &ScriptProjectSettings::internal_SetFloat);
		metaData.scriptClass->addInternalCall("Internal_SetInt", &ScriptProjectSettings::internal_SetInt);
		metaData.scriptClass->addInternalCall("Internal_SetBool", &ScriptProjectSettings::internal_SetBool);
		metaData.scriptClass->addInternalCall("Internal_SetString", &ScriptProjectSettings::internal_SetString);
		metaData.scriptClass->addInternalCall("Internal_GetFloat", &ScriptProjectSettings::internal_GetFloat);
		metaData.scriptClass->addInternalCall("Internal_GetInt", &ScriptProjectSettings::internal_GetInt);
		metaData.scriptClass->addInternalCall("Internal_GetBool", &ScriptProjectSettings::internal_GetBool);
		metaData.scriptClass->addInternalCall("Internal_GetString", &ScriptProjectSettings::internal_GetString);
		metaData.scriptClass->addInternalCall("Internal_HasKey", &ScriptProjectSettings::internal_HasKey);
		metaData.scriptClass->addInternalCall("Internal_DeleteKey", &ScriptProjectSettings::internal_DeleteKey);
		metaData.scriptClass->addInternalCall("Internal_DeleteAllKeys", &ScriptProjectSettings::internal_DeleteAllKeys);
		metaData.scriptClass->addInternalCall("Internal_GetHash", &ScriptProjectSettings::internal_GetHash);
		metaData.scriptClass->addInternalCall("Internal_Save", &ScriptProjectSettings::internal_Save);
	}

	MonoString* ScriptProjectSettings::internal_GetLastOpenScene()
	{
		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return MonoUtil::stringToMono(settings->getLastOpenScene());
	}

	void ScriptProjectSettings::internal_SetLastOpenScene(MonoString* value)
	{
		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->setLastOpenScene(MonoUtil::monoToString(value));
	}

	void ScriptProjectSettings::internal_SetFloat(MonoString* name, float value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->setFloat(nativeName, value);
	}

	void ScriptProjectSettings::internal_SetInt(MonoString* name, int value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->setInt(nativeName, value);
	}

	void ScriptProjectSettings::internal_SetBool(MonoString* name, bool value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->setBool(nativeName, value);
	}

	void ScriptProjectSettings::internal_SetString(MonoString* name, MonoString* value)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeValue = MonoUtil::monoToWString(value);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->setString(nativeName, nativeValue);
	}

	float ScriptProjectSettings::internal_GetFloat(MonoString* name, float defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return settings->getFloat(nativeName, defaultValue);
	}

	int ScriptProjectSettings::internal_GetInt(MonoString* name, int defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return settings->getInt(nativeName, defaultValue);
	}

	bool ScriptProjectSettings::internal_GetBool(MonoString* name, bool defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return settings->getBool(nativeName, defaultValue);
	}

	MonoString* ScriptProjectSettings::internal_GetString(MonoString* name, MonoString* defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeDefaultValue = MonoUtil::monoToWString(defaultValue);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		WString nativeValue = settings->getString(nativeName, nativeDefaultValue);

		return MonoUtil::wstringToMono(nativeValue);
	}

	bool ScriptProjectSettings::internal_HasKey(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return settings->hasKey(nativeName);
	}

	void ScriptProjectSettings::internal_DeleteKey(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->deleteKey(nativeName);
	}

	void ScriptProjectSettings::internal_DeleteAllKeys()
	{
		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		settings->deleteAllKeys();
	}

	UINT32 ScriptProjectSettings::internal_GetHash()
	{
		SPtr<ProjectSettings> settings = gEditorApplication().getProjectSettings();
		return settings->getHash();
	}

	void ScriptProjectSettings::internal_Save()
	{
		gEditorApplication().saveProjectSettings();
	}
}
