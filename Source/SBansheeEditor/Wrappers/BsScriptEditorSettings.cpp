//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptEditorSettings.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsEditorApplication.h"
#include "Settings/BsEditorSettings.h"

namespace bs
{
	ScriptEditorSettings::ScriptEditorSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetMoveHandleSnapActive", (void*)&ScriptEditorSettings::internal_GetMoveHandleSnapActive);
		metaData.scriptClass->addInternalCall("Internal_SetMoveHandleSnapActive", (void*)&ScriptEditorSettings::internal_SetMoveHandleSnapActive);
		metaData.scriptClass->addInternalCall("Internal_GetRotateHandleSnapActive", (void*)&ScriptEditorSettings::internal_GetRotateHandleSnapActive);
		metaData.scriptClass->addInternalCall("Internal_SetRotateHandleSnapActive", (void*)&ScriptEditorSettings::internal_SetRotateHandleSnapActive);
		metaData.scriptClass->addInternalCall("Internal_GetMoveHandleSnapAmount", (void*)&ScriptEditorSettings::internal_GetMoveHandleSnapAmount);
		metaData.scriptClass->addInternalCall("Internal_SetMoveHandleSnapAmount", (void*)&ScriptEditorSettings::internal_SetMoveHandleSnapAmount);
		metaData.scriptClass->addInternalCall("Internal_GetRotateHandleSnapAmount", (void*)&ScriptEditorSettings::internal_GetRotateHandleSnapAmount);
		metaData.scriptClass->addInternalCall("Internal_SetRotateHandleSnapAmount", (void*)&ScriptEditorSettings::internal_SetRotateHandleSnapAmount);
		metaData.scriptClass->addInternalCall("Internal_GetDefaultHandleSize", (void*)&ScriptEditorSettings::internal_GetDefaultHandleSize);
		metaData.scriptClass->addInternalCall("Internal_SetDefaultHandleSize", (void*)&ScriptEditorSettings::internal_SetDefaultHandleSize);
		metaData.scriptClass->addInternalCall("Internal_GetActiveSceneTool", (void*)&ScriptEditorSettings::internal_GetActiveSceneTool);
		metaData.scriptClass->addInternalCall("Internal_SetActiveSceneTool", (void*)&ScriptEditorSettings::internal_SetActiveSceneTool);
		metaData.scriptClass->addInternalCall("Internal_GetActiveCoordinateMode", (void*)&ScriptEditorSettings::internal_GetActiveCoordinateMode);
		metaData.scriptClass->addInternalCall("Internal_SetActiveCoordinateMode", (void*)&ScriptEditorSettings::internal_SetActiveCoordinateMode);
		metaData.scriptClass->addInternalCall("Internal_GetActivePivotMode", (void*)&ScriptEditorSettings::internal_GetActivePivotMode);
		metaData.scriptClass->addInternalCall("Internal_SetActivePivotMode", (void*)&ScriptEditorSettings::internal_SetActivePivotMode);
		metaData.scriptClass->addInternalCall("Internal_GetFPSLimit", (void*)&ScriptEditorSettings::internal_GetFPSLimit);
		metaData.scriptClass->addInternalCall("Internal_SetFPSLimit", (void*)&ScriptEditorSettings::internal_SetFPSLimit);
		metaData.scriptClass->addInternalCall("Internal_GetMouseSensitivity", (void*)&ScriptEditorSettings::internal_GetMouseSensitivity);
		metaData.scriptClass->addInternalCall("Internal_SetMouseSensitivity", (void*)&ScriptEditorSettings::internal_SetMouseSensitivity);
		metaData.scriptClass->addInternalCall("Internal_GetLastOpenProject", (void*)&ScriptEditorSettings::internal_GetLastOpenProject);
		metaData.scriptClass->addInternalCall("Internal_SetLastOpenProject", (void*)&ScriptEditorSettings::internal_SetLastOpenProject);
		metaData.scriptClass->addInternalCall("Internal_GetAutoLoadLastProject", (void*)&ScriptEditorSettings::internal_GetAutoLoadLastProject);
		metaData.scriptClass->addInternalCall("Internal_SetAutoLoadLastProject", (void*)&ScriptEditorSettings::internal_SetAutoLoadLastProject);
		metaData.scriptClass->addInternalCall("Internal_GetRecentProjects", (void*)&ScriptEditorSettings::internal_GetRecentProjects);
		metaData.scriptClass->addInternalCall("Internal_SetRecentProjects", (void*)&ScriptEditorSettings::internal_SetRecentProjects);
		metaData.scriptClass->addInternalCall("Internal_SetFloat", (void*)&ScriptEditorSettings::internal_SetFloat);
		metaData.scriptClass->addInternalCall("Internal_SetInt", (void*)&ScriptEditorSettings::internal_SetInt);
		metaData.scriptClass->addInternalCall("Internal_SetBool", (void*)&ScriptEditorSettings::internal_SetBool);
		metaData.scriptClass->addInternalCall("Internal_SetString", (void*)&ScriptEditorSettings::internal_SetString);
		metaData.scriptClass->addInternalCall("Internal_GetFloat", (void*)&ScriptEditorSettings::internal_GetFloat);
		metaData.scriptClass->addInternalCall("Internal_GetInt", (void*)&ScriptEditorSettings::internal_GetInt);
		metaData.scriptClass->addInternalCall("Internal_GetBool", (void*)&ScriptEditorSettings::internal_GetBool);
		metaData.scriptClass->addInternalCall("Internal_GetString", (void*)&ScriptEditorSettings::internal_GetString);
		metaData.scriptClass->addInternalCall("Internal_HasKey", (void*)&ScriptEditorSettings::internal_HasKey);
		metaData.scriptClass->addInternalCall("Internal_DeleteKey", (void*)&ScriptEditorSettings::internal_DeleteKey);
		metaData.scriptClass->addInternalCall("Internal_DeleteAllKeys", (void*)&ScriptEditorSettings::internal_DeleteAllKeys);
		metaData.scriptClass->addInternalCall("Internal_GetHash", (void*)&ScriptEditorSettings::internal_GetHash);
		metaData.scriptClass->addInternalCall("Internal_Save", (void*)&ScriptEditorSettings::internal_Save);
	}

	bool ScriptEditorSettings::internal_GetMoveHandleSnapActive()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getMoveHandleSnapActive();
	}

	void ScriptEditorSettings::internal_SetMoveHandleSnapActive(bool value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setMoveHandleSnapActive(value);
	}

	bool ScriptEditorSettings::internal_GetRotateHandleSnapActive()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getRotateHandleSnapActive();
	}

	void ScriptEditorSettings::internal_SetRotateHandleSnapActive(bool value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setRotateHandleSnapActive(value);
	}

	float ScriptEditorSettings::internal_GetMoveHandleSnapAmount()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getMoveHandleSnap();
	}

	void ScriptEditorSettings::internal_SetMoveHandleSnapAmount(float value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setMoveHandleSnap(value);
	}

	float ScriptEditorSettings::internal_GetRotateHandleSnapAmount()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getRotationHandleSnap().valueDegrees();
	}

	void ScriptEditorSettings::internal_SetRotateHandleSnapAmount(float value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setRotationHandleSnap(Degree(value));
	}

	float ScriptEditorSettings::internal_GetDefaultHandleSize()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getHandleSize();
	}

	void ScriptEditorSettings::internal_SetDefaultHandleSize(float value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setHandleSize(value);
	}

	UINT32 ScriptEditorSettings::internal_GetActiveSceneTool()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getActiveSceneTool();
	}

	void ScriptEditorSettings::internal_SetActiveSceneTool(UINT32 value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setActiveSceneTool(value);
	}

	UINT32 ScriptEditorSettings::internal_GetActiveCoordinateMode()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getActiveCoordinateMode();
	}

	void ScriptEditorSettings::internal_SetActiveCoordinateMode(UINT32 value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setActiveCoordinateMode(value);
	}

	UINT32 ScriptEditorSettings::internal_GetActivePivotMode()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getActivePivotMode();
	}

	void ScriptEditorSettings::internal_SetActivePivotMode(UINT32 value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setActivePivotMode(value);
	}

	UINT32 ScriptEditorSettings::internal_GetFPSLimit()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getFPSLimit();
	}

	void ScriptEditorSettings::internal_SetFPSLimit(UINT32 value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setFPSLimit(value);
	}

	float ScriptEditorSettings::internal_GetMouseSensitivity()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getMouseSensitivity();
	}

	void ScriptEditorSettings::internal_SetMouseSensitivity(float value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setMouseSensitivity(value);
	}

	MonoString* ScriptEditorSettings::internal_GetLastOpenProject()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return MonoUtil::wstringToMono(settings->getLastOpenProject().toWString());
	}

	void ScriptEditorSettings::internal_SetLastOpenProject(MonoString* value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setLastOpenProject(MonoUtil::monoToWString(value));
	}

	bool ScriptEditorSettings::internal_GetAutoLoadLastProject()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getAutoLoadLastProject();
	}

	void ScriptEditorSettings::internal_SetAutoLoadLastProject(bool value)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setAutoLoadLastProject(value);
	}

	void ScriptEditorSettings::internal_GetRecentProjects(MonoArray** paths, MonoArray** timeStamps)
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		const Vector<RecentProject>& recentProjects = settings->getRecentProjects();
		UINT32 numEntries = (UINT32)recentProjects.size();

		ScriptArray outputPaths = ScriptArray::create<WString>(numEntries);
		ScriptArray outputTimeStamps = ScriptArray::create<UINT64>(numEntries);

		for (UINT32 i = 0; i < numEntries; i++)
		{
			WString projectPath = recentProjects[i].path.toWString();
			MonoString* monoPath = MonoUtil::wstringToMono(projectPath);

			outputPaths.set(i, monoPath);
			outputTimeStamps.set(i, recentProjects[i].accessTimestamp);
		}

		MonoUtil::referenceCopy(paths, (MonoObject*)outputPaths.getInternal());
		MonoUtil::referenceCopy(timeStamps, (MonoObject*)outputPaths.getInternal());
	}

	void ScriptEditorSettings::internal_SetRecentProjects(MonoArray* paths, MonoArray* timeStamps)
	{
		ScriptArray pathsArray(paths);
		ScriptArray timeStampsArray(timeStamps);
		UINT32 numEntries = pathsArray.size();

		Vector<RecentProject> recentProjects(numEntries);
		for (UINT32 i = 0; i < numEntries; i++)
		{
			recentProjects[i].path = pathsArray.get<WString>(i);
			recentProjects[i].accessTimestamp = timeStampsArray.get<UINT64>(i);
		}

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setRecentProjects(recentProjects);
	}

	void ScriptEditorSettings::internal_SetFloat(MonoString* name, float value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setFloat(nativeName, value);
	}

	void ScriptEditorSettings::internal_SetInt(MonoString* name, int value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setInt(nativeName, value);
	}

	void ScriptEditorSettings::internal_SetBool(MonoString* name, bool value)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setBool(nativeName, value);
	}

	void ScriptEditorSettings::internal_SetString(MonoString* name, MonoString* value)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeValue = MonoUtil::monoToWString(value);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->setString(nativeName, nativeValue);
	}

	float ScriptEditorSettings::internal_GetFloat(MonoString* name, float defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getFloat(nativeName, defaultValue);
	}

	int ScriptEditorSettings::internal_GetInt(MonoString* name, int defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getInt(nativeName, defaultValue);
	}

	bool ScriptEditorSettings::internal_GetBool(MonoString* name, bool defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getBool(nativeName, defaultValue);
	}

	MonoString* ScriptEditorSettings::internal_GetString(MonoString* name, MonoString* defaultValue)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeDefaultValue = MonoUtil::monoToWString(defaultValue);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		WString nativeValue = settings->getString(nativeName, nativeDefaultValue);

		return MonoUtil::wstringToMono(nativeValue);
	}

	bool ScriptEditorSettings::internal_HasKey(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->hasKey(nativeName);
	}

	void ScriptEditorSettings::internal_DeleteKey(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->deleteKey(nativeName);
	}

	void ScriptEditorSettings::internal_DeleteAllKeys()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		settings->deleteAllKeys();
	}

	UINT32 ScriptEditorSettings::internal_GetHash()
	{
		SPtr<EditorSettings> settings = gEditorApplication().getEditorSettings();
		return settings->getHash();
	}

	void ScriptEditorSettings::internal_Save()
	{
		gEditorApplication().saveEditorSettings();
	}
}
