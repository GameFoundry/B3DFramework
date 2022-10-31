//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptDebug.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include "Wrappers/BsScriptLogEntry.h"

using namespace bs;
HEvent ScriptDebug::mOnLogEntryAddedConn;
ScriptDebug::OnAddedThunkDef ScriptDebug::onAddedThunk = nullptr;

/**	C++ version of the managed LogEntry structure. */
struct ScriptLogEntryData
{
	MonoString* Message;
	LogVerbosity Verbosity;
	u32 Category;
};

ScriptDebug::ScriptDebug(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptDebug::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_Log", (void*)&ScriptDebug::InternalLog);
	metaData.ScriptClass->AddInternalCall("Internal_LogWarning", (void*)&ScriptDebug::InternalLogWarning);
	metaData.ScriptClass->AddInternalCall("Internal_LogError", (void*)&ScriptDebug::InternalLogError);
	metaData.ScriptClass->AddInternalCall("Internal_LogMessage", (void*)&ScriptDebug::InternalLogMessage);
	metaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptDebug::InternalClear);
	metaData.ScriptClass->AddInternalCall("Internal_GetMessages", (void*)&ScriptDebug::InternalGetMessages);

	onAddedThunk = (OnAddedThunkDef)metaData.ScriptClass->GetMethod("Internal_OnAdded", 3)->GetThunk();
}

void ScriptDebug::StartUp()
{
	mOnLogEntryAddedConn = GetDebug().OnLogEntryAdded.Connect(&ScriptDebug::OnLogEntryAdded);
}

void ScriptDebug::ShutDown()
{
	mOnLogEntryAddedConn.Disconnect();
}

void ScriptDebug::OnLogEntryAdded(const LogEntry& entry)
{
	MonoString* message = MonoUtil::StringToMono(entry.GetMessage());

	MonoUtil::InvokeThunk(onAddedThunk, message, (i32)entry.GetVerbosity(), entry.GetCategory());
}

void ScriptDebug::InternalLog(MonoString* message, u32 category)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Info, category);
}

void ScriptDebug::InternalLogWarning(MonoString* message, u32 category)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Warning, category);
}

void ScriptDebug::InternalLogError(MonoString* message, u32 category)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Error, category);
}

void ScriptDebug::InternalLogMessage(MonoString* message, LogVerbosity type, u32 category)
{
	GetDebug().Log(MonoUtil::MonoToString(message), type, category);
}

void ScriptDebug::InternalClear(LogVerbosity verbosity, u32 category)
{
	GetDebug().GetLog().Clear(verbosity, category);
}

MonoArray* ScriptDebug::InternalGetMessages()
{
	Vector<LogEntry> entries = GetDebug().GetLog().GetEntries();

	u32 numEntries = (u32)entries.size();
	ScriptArray output = ScriptArray::Create<ScriptLogEntry>(numEntries);
	for(u32 i = 0; i < numEntries; i++)
	{
		MonoString* message = MonoUtil::StringToMono(entries[i].GetMessage());

		ScriptLogEntryData scriptEntry = { message, entries[i].GetVerbosity(), entries[i].GetCategory() };
		output.Set(i, scriptEntry);
	}

	return output.GetInternal();
}
