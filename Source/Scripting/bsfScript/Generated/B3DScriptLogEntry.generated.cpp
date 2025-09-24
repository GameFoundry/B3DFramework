//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptLogEntry.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptLogEntry::ScriptLogEntry()
	{ }

	MonoObject* ScriptLogEntry::Box(const __LogEntryInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__LogEntryInterop ScriptLogEntry::Unbox(MonoObject* value)
	{
		return *(__LogEntryInterop*)MonoUtil::Unbox(value);
	}

	LogEntry ScriptLogEntry::FromInterop(const __LogEntryInterop& value)
	{
		LogEntry output;
		String tmpMessage;
		tmpMessage = MonoUtil::MonoToString(value.Message);
		output.Message = tmpMessage;
		output.Verbosity = value.Verbosity;
		String tmpCategoryName;
		tmpCategoryName = MonoUtil::MonoToString(value.CategoryName);
		output.CategoryName = tmpCategoryName;
		output.LocalTime = value.LocalTime;

		return output;
	}

	__LogEntryInterop ScriptLogEntry::ToInterop(const LogEntry& value)
	{
		__LogEntryInterop output;
		MonoString* tmpMessage;
		tmpMessage = MonoUtil::StringToMono(value.Message);
		output.Message = tmpMessage;
		output.Verbosity = value.Verbosity;
		MonoString* tmpCategoryName;
		tmpCategoryName = MonoUtil::StringToMono(value.CategoryName);
		output.CategoryName = tmpCategoryName;
		output.LocalTime = value.LocalTime;

		return output;
	}

}
