//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptDebug.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include "Wrappers/BsScriptLogEntry.h"

namespace bs
{
	HEvent ScriptDebug::mOnLogEntryAddedConn;
	ScriptDebug::OnAddedThunkDef ScriptDebug::onAddedThunk = nullptr;

	/**	C++ version of the managed LogEntry structure. */
	struct ScriptLogEntryData
	{
		MonoString* message;
		LogVerbosity verbosity;
		UINT32 category;
	};

	ScriptDebug::ScriptDebug(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptDebug::InitRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Log", (void*)&ScriptDebug::internal_log);
		metaData.scriptClass->addInternalCall("Internal_LogWarning", (void*)&ScriptDebug::internal_logWarning);
		metaData.scriptClass->addInternalCall("Internal_LogError", (void*)&ScriptDebug::internal_logError);
		metaData.scriptClass->addInternalCall("Internal_LogMessage", (void*)&ScriptDebug::internal_logMessage);
		metaData.scriptClass->addInternalCall("Internal_Clear", (void*)&ScriptDebug::internal_clear);
		metaData.scriptClass->addInternalCall("Internal_GetMessages", (void*)&ScriptDebug::internal_getMessages);

		onAddedThunk = (OnAddedThunkDef)metaData.scriptClass->getMethod("Internal_OnAdded", 3)->getThunk();
	}

	void ScriptDebug::StartUp()
	{
		mOnLogEntryAddedConn = gDebug().onLogEntryAdded.connect(&ScriptDebug::onLogEntryAdded);
	}

	void ScriptDebug::ShutDown()
	{
		mOnLogEntryAddedConn.disconnect();
	}

	void ScriptDebug::OnLogEntryAdded(const LogEntry& entry)
	{
		MonoString* message = MonoUtil::stringToMono(entry.getMessage());

		MonoUtil::invokeThunk(onAddedThunk, message, (INT32)entry.getVerbosity(), entry.getCategory());
	}

	void ScriptDebug::internal_log(MonoString* message, UINT32 category)
	{
		gDebug().log(MonoUtil::monoToString(message), LogVerbosity::Info, category);
	}

	void ScriptDebug::internal_logWarning(MonoString* message, UINT32 category)
	{
		gDebug().log(MonoUtil::monoToString(message), LogVerbosity::Warning, category);
	}

	void ScriptDebug::internal_logError(MonoString* message, UINT32 category)
	{
		gDebug().log(MonoUtil::monoToString(message), LogVerbosity::Error, category);
	}

	void ScriptDebug::internal_logMessage(MonoString* message, LogVerbosity type, UINT32 category)
	{
		gDebug().log(MonoUtil::monoToString(message), type, category);
	}

	void ScriptDebug::internal_clear(LogVerbosity verbosity, UINT32 category)
	{
		gDebug().getLog().clear(verbosity, category);
	}

	MonoArray* ScriptDebug::internal_getMessages()
	{
		Vector<LogEntry> entries = gDebug().getLog().getEntries();

		UINT32 numEntries = (UINT32)entries.size();
		ScriptArray output = ScriptArray::create<ScriptLogEntry>(numEntries);
		for (UINT32 i = 0; i < numEntries; i++)
		{
			MonoString* message = MonoUtil::stringToMono(entries[i].getMessage());

			ScriptLogEntryData scriptEntry = { message, entries[i].getVerbosity(), entries[i].getCategory() };
			output.set(i, scriptEntry);
		}

		return output.getInternal();
	}
}
