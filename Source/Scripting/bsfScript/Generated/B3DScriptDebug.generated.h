//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Utility/Debug/B3DDebug.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Utility/Debug/B3DLog.h"
#include "../../../Foundation/Utility/Debug/B3DLog.h"

namespace b3d { struct __LogEntryInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDebug : public TScriptTypeDefinition<ScriptDebug>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Debug")

		ScriptDebug();

		static void SetupScriptBindings();

		static void StartUp();
		static void ShutDown();

	private:
		static void OnLogEntryAdded(const LogEntry& p0);
		static void OnLogModified();

		typedef void(B3D_THUNKCALL *OnLogEntryAddedThunkDefinition) (MonoObject* p0, MonoException**);
		static OnLogEntryAddedThunkDefinition OnLogEntryAddedThunk;
		typedef void(B3D_THUNKCALL *OnLogModifiedThunkDefinition) (MonoException**);
		static OnLogModifiedThunkDefinition OnLogModifiedThunk;

		static HEvent OnLogEntryAddedConnection;
		static HEvent OnLogModifiedConnection;

		static void InternalLog(MonoString* message, LogVerbosity verbosity, MonoString* categoryName);
		static void InternalClearLog(MonoString* categoryName, LogVerbosity verbosity);
		static MonoArray* InternalGetLogEntries();
	};
}
