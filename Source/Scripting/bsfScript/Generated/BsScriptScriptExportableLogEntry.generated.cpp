//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScriptExportableLogEntry.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptLogEntry::ScriptLogEntry()
	{ }

	MonoObject* ScriptLogEntry::Box(const __ScriptExportableLogEntryInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ScriptExportableLogEntryInterop ScriptLogEntry::Unbox(MonoObject* value)
	{
		return *(__ScriptExportableLogEntryInterop*)MonoUtil::Unbox(value);
	}

	ScriptExportableLogEntry ScriptLogEntry::FromInterop(const __ScriptExportableLogEntryInterop& value)
	{
		ScriptExportableLogEntry output;
		String tmpMessage;
		tmpMessage = MonoUtil::MonoToString(value.Message);
		output.Message = tmpMessage;
		output.Verbosity = value.Verbosity;
		String tmpCategoryName;
		tmpCategoryName = MonoUtil::MonoToString(value.CategoryName);
		output.CategoryName = tmpCategoryName;

		return output;
	}

	__ScriptExportableLogEntryInterop ScriptLogEntry::ToInterop(const ScriptExportableLogEntry& value)
	{
		__ScriptExportableLogEntryInterop output;
		MonoString* tmpMessage;
		tmpMessage = MonoUtil::StringToMono(value.Message);
		output.Message = tmpMessage;
		output.Verbosity = value.Verbosity;
		MonoString* tmpCategoryName;
		tmpCategoryName = MonoUtil::StringToMono(value.CategoryName);
		output.CategoryName = tmpCategoryName;

		return output;
	}

}
