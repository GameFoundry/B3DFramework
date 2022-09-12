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
		metaData.scriptClass->AddInternalCall("Internal_Log", (void*)&ScriptDebug::internal_log);
		metaData.scriptClass->AddInternalCall("Internal_LogWarning", (void*)&ScriptDebug::internal_logWarning);
		metaData.scriptClass->AddInternalCall("Internal_LogError", (void*)&ScriptDebug::internal_logError);
		metaData.scriptClass->AddInternalCall("Internal_LogMessage", (void*)&ScriptDebug::internal_logMessage);
		metaData.scriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptDebug::internal_clear);
		metaData.scriptClass->AddInternalCall("Internal_GetMessages", (void*)&ScriptDebug::internal_getMessages);

		onAddedThunk = (OnAddedThunkDef)metaData.scriptClass->GetMethod("Internal_OnAdded", 3)->getThunk();
	}

	void ScriptDebug::StartUp()
	{
		mOnLogEntryAddedConn = gDebug().onLogEntryAdded.Connect(&ScriptDebug::onLogEntryAdded);
	}

	void ScriptDebug::ShutDown()
	{
		mOnLogEntryAddedConn.Disconnect();
	}

	void ScriptDebug::OnLogEntryAdded(const LogEntry& entry)
	{
		MonoString* message = MonoUtil::stringToMono(entry.GetMessage());

		MonoUtil::invokeThunk(onAddedThunk, message, (INT32)entry.GetVerbosity(), entry.getCategory());
	}

	void ScriptDebug::internal_log(MonoString* message, UINT32 category)
	{
		gDebug().Log(MonoUtil::monoToString(message), LogVerbosity::Info, category);
	}

	void ScriptDebug::internal_logWarning(MonoString* message, UINT32 category)
	{
		gDebug().Log(MonoUtil::monoToString(message), LogVerbosity::Warning, category);
	}

	void ScriptDebug::internal_logError(MonoString* message, UINT32 category)
	{
		gDebug().Log(MonoUtil::monoToString(message), LogVerbosity::Error, category);
	}

	void ScriptDebug::internal_logMessage(MonoString* message, LogVerbosity type, UINT32 category)
	{
		gDebug().Log(MonoUtil::monoToString(message), type, category);
	}

	void ScriptDebug::internal_clear(LogVerbosity verbosity, UINT32 category)
	{
		gDebug().GetLog().clear(verbosity, category);
	}

	MonoArray* ScriptDebug::internal_getMessages()
	{
		Vector<LogEntry> entries = gDebug().GetLog().getEntries();

		UINT32 numEntries = (UINT32)entries.Size();
		ScriptArray output = ScriptArray::create<ScriptLogEntry>(numEntries);
		for (UINT32 i = 0; i < numEntries; i++)
		{
			MonoString* message = MonoUtil::stringToMono(entries[i].GetMessage());

			ScriptLogEntryData scriptEntry = { message, entries[i].GetVerbosity(), entries[i].getCategory() };
			output.Set(i, scriptEntry);
		}

		return output.GetInternal();
	}
}
