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
		UINT32 type;
		MonoString* message;
	};

	ScriptDebug::ScriptDebug(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptDebug::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Log", (void*)&ScriptDebug::internal_log);
		metaData.scriptClass->addInternalCall("Internal_LogWarning", (void*)&ScriptDebug::internal_logWarning);
		metaData.scriptClass->addInternalCall("Internal_LogError", (void*)&ScriptDebug::internal_logError);
		metaData.scriptClass->addInternalCall("Internal_LogMessage", (void*)&ScriptDebug::internal_logMessage);
		metaData.scriptClass->addInternalCall("Internal_Clear", (void*)&ScriptDebug::internal_clear);
		metaData.scriptClass->addInternalCall("Internal_ClearType", (void*)&ScriptDebug::internal_clearType);
		metaData.scriptClass->addInternalCall("Internal_GetMessages", (void*)&ScriptDebug::internal_getMessages);

		onAddedThunk = (OnAddedThunkDef)metaData.scriptClass->getMethod("Internal_OnAdded", 2)->getThunk();
	}

	void ScriptDebug::startUp()
	{
		mOnLogEntryAddedConn = gDebug().onLogEntryAdded.connect(&ScriptDebug::onLogEntryAdded);
	}

	void ScriptDebug::shutDown()
	{
		mOnLogEntryAddedConn.disconnect();
	}

	void ScriptDebug::onLogEntryAdded(const LogEntry& entry)
	{
		MonoString* message = MonoUtil::stringToMono(entry.getMessage());

		MonoUtil::invokeThunk(onAddedThunk, entry.getChannel(), message);
	}

	void ScriptDebug::internal_log(MonoString* message)
	{
		gDebug().logDebug(MonoUtil::monoToString(message));
	}

	void ScriptDebug::internal_logWarning(MonoString* message)
	{
		gDebug().logWarning(MonoUtil::monoToString(message));
	}

	void ScriptDebug::internal_logError(MonoString* message)
	{
		gDebug().logError(MonoUtil::monoToString(message));
	}

	void ScriptDebug::internal_logMessage(MonoString* message, UINT32 type)
	{
		gDebug().log(MonoUtil::monoToString(message), type);
	}

	void ScriptDebug::internal_clear()
	{
		gDebug().getLog().clear();
	}

	void ScriptDebug::internal_clearType(UINT32 type)
	{
		gDebug().getLog().clear(type);
	}

	MonoArray* ScriptDebug::internal_getMessages()
	{
		Vector<LogEntry> entries = gDebug().getLog().getEntries();

		UINT32 numEntries = (UINT32)entries.size();
		ScriptArray output = ScriptArray::create<ScriptLogEntry>(numEntries);
		for (UINT32 i = 0; i < numEntries; i++)
		{
			MonoString* message = MonoUtil::stringToMono(entries[i].getMessage());

			ScriptLogEntryData scriptEntry = { entries[i].getChannel(), message };
			output.set(i, scriptEntry);
		}

		return output.getInternal();
	}
}