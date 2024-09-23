//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptDebug.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include "Generated/BsScriptScriptExportableLogEntry.generated.h"

using namespace bs;
HEvent ScriptDebug::mOnLogEntryAddedConn;
ScriptDebug::OnAddedThunkDef ScriptDebug::onAddedThunk = nullptr;

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
	MonoString *const message = MonoUtil::StringToMono(entry.Message);
	MonoString* const categoryName = MonoUtil::StringToMono(entry.CategoryName);

	MonoUtil::InvokeThunk(onAddedThunk, message, (i32)entry.Verbosity, categoryName);
}

void ScriptDebug::InternalLog(MonoString* message, MonoString* categoryName)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Info, MonoUtil::MonoToString(categoryName).c_str());
}

void ScriptDebug::InternalLogWarning(MonoString* message, MonoString* categoryName)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Warning, MonoUtil::MonoToString(categoryName).c_str());
}

void ScriptDebug::InternalLogError(MonoString* message, MonoString* categoryName)
{
	GetDebug().Log(MonoUtil::MonoToString(message), LogVerbosity::Error, MonoUtil::MonoToString(categoryName).c_str());
}

void ScriptDebug::InternalLogMessage(MonoString* message, LogVerbosity type, MonoString* categoryName)
{
	GetDebug().Log(MonoUtil::MonoToString(message), type, MonoUtil::MonoToString(categoryName).c_str());
}

void ScriptDebug::InternalClear(MonoString* categoryName, LogVerbosity verbosity)
{
	GetDebug().GetLog().Clear(MonoUtil::MonoToString(categoryName).c_str(), verbosity);
}

MonoArray* ScriptDebug::InternalGetMessages()
{
	Vector<LogEntry> entries = GetDebug().GetLog().GetEntries();

	u32 numEntries = (u32)entries.size();
	ScriptArray output = ScriptArray::Create<ScriptLogEntry>(numEntries);
	for(u32 i = 0; i < numEntries; i++)
	{
		ScriptExportableLogEntry logEntry = { entries[i].Message, entries[i].Verbosity, entries[i].CategoryName };
		output.Set(i, ScriptLogEntry::ToInterop(logEntry));
	}

	return output.GetInternal();
}
