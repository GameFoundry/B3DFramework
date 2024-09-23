//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../Wrappers/BsScriptDebug.h"
#include "../../../Foundation/bsfUtility/Debug/BsLog.h"

namespace bs
{
	struct __ScriptExportableLogEntryInterop
	{
		MonoString* Message;
		LogVerbosity Verbosity;
		MonoString* CategoryName;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLogEntry : public TScriptTypeDefinition<ScriptLogEntry>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LogEntry")

		static MonoObject* Box(const __ScriptExportableLogEntryInterop& value);
		static __ScriptExportableLogEntryInterop Unbox(MonoObject* value);
		static ScriptExportableLogEntry FromInterop(const __ScriptExportableLogEntryInterop& value);
		static __ScriptExportableLogEntryInterop ToInterop(const ScriptExportableLogEntry& value);

	private:
		ScriptLogEntry();

	};
}
