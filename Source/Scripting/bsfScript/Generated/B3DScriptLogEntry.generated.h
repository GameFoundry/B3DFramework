//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Debug/BsLog.h"
#include "../../../Foundation/bsfUtility/Debug/BsLog.h"

namespace b3d
{
	struct __LogEntryInterop
	{
		MonoString* Message;
		LogVerbosity Verbosity;
		MonoString* CategoryName;
		uint64_t LocalTime;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLogEntry : public TScriptTypeDefinition<ScriptLogEntry>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LogEntry")

		static MonoObject* Box(const __LogEntryInterop& value);
		static __LogEntryInterop Unbox(MonoObject* value);
		static LogEntry FromInterop(const __LogEntryInterop& value);
		static __LogEntryInterop ToInterop(const LogEntry& value);

	private:
		ScriptLogEntry();

	};
}
