//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptDebug.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Debug/BsDebug.h"
#include "BsScriptLogEntry.generated.h"

namespace b3d
{
	ScriptDebug::OnLogEntryAddedThunkDefinition ScriptDebug::OnLogEntryAddedThunk; 
	ScriptDebug::OnLogModifiedThunkDefinition ScriptDebug::OnLogModifiedThunk; 

	HEvent ScriptDebug::OnLogEntryAddedConnection;
	HEvent ScriptDebug::OnLogModifiedConnection;

	ScriptDebug::ScriptDebug()
		:TScriptTypeDefinition()
	{
	}

	void ScriptDebug::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Log", (void*)&ScriptDebug::InternalLog);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearLog", (void*)&ScriptDebug::InternalClearLog);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLogEntries", (void*)&ScriptDebug::InternalGetLogEntries);

		OnLogEntryAddedThunk = (OnLogEntryAddedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnLogEntryAdded", "LogEntry&")->GetThunk();
		OnLogModifiedThunk = (OnLogModifiedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnLogModified", "")->GetThunk();
	}

	void ScriptDebug::StartUp()
	{
		OnLogEntryAddedConnection = GetDebug().OnLogEntryAdded.Connect(&ScriptDebug::OnLogEntryAdded);
		OnLogModifiedConnection = GetDebug().OnLogModified.Connect(&ScriptDebug::OnLogModified);
	}
	void ScriptDebug::ShutDown()
	{
		OnLogEntryAddedConnection.Disconnect();
		OnLogModifiedConnection.Disconnect();
	}

	void ScriptDebug::OnLogEntryAdded(const LogEntry& p0)
	{
		MonoObject* tmpp0;
		__LogEntryInterop interopp0;
		interopp0 = ScriptLogEntry::ToInterop(p0);
		tmpp0 = ScriptLogEntry::Box(interopp0);
		MonoUtil::InvokeThunk(OnLogEntryAddedThunk, tmpp0);
	}

	void ScriptDebug::OnLogModified()
	{
		MonoUtil::InvokeThunk(OnLogModifiedThunk);
	}

	void ScriptDebug::InternalLog(MonoString* message, LogVerbosity verbosity, MonoString* categoryName)
	{
		String tmpmessage;
		tmpmessage = MonoUtil::MonoToString(message);
		String tmpcategoryName;
		tmpcategoryName = MonoUtil::MonoToString(categoryName);
		GetDebug().Log(tmpmessage, verbosity, tmpcategoryName);
	}

	void ScriptDebug::InternalClearLog(MonoString* categoryName, LogVerbosity verbosity)
	{
		String tmpcategoryName;
		tmpcategoryName = MonoUtil::MonoToString(categoryName);
		GetDebug().ClearLog(tmpcategoryName, verbosity);
	}

	MonoArray* ScriptDebug::InternalGetLogEntries()
	{
		Vector<LogEntry> nativeArray__output;
		nativeArray__output = GetDebug().GetLogEntries();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptLogEntry>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptLogEntry::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
