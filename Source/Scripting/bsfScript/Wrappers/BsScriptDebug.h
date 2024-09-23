//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Script-exportable variant of LogEntry. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), ExportName(LogEntry)) ScriptExportableLogEntry
	{
		String Message;
		LogVerbosity Verbosity;
		String CategoryName;
	};

	/**	Interop class between C++ & CLR for Debug. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDebug : public ScriptObject<ScriptDebug>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Debug")

		/**	Registers internal callbacks. Must be called on scripting system load. */
		static void StartUp();

		/**	Unregisters internal callbacks. Must be called on scripting system shutdown. */
		static void ShutDown();

	private:
		ScriptDebug(MonoObject* instance);

		/**	Triggered when a new entry is added to the debug log. */
		static void OnLogEntryAdded(const LogEntry& entry);

		static HEvent mOnLogEntryAddedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalLog(MonoString* message, MonoString* categoryName);
		static void InternalLogWarning(MonoString* message, MonoString* categoryName);
		static void InternalLogError(MonoString* message, MonoString* categoryName);
		static void InternalLogMessage(MonoString* message, LogVerbosity verbosity, MonoString* categoryName);
		static void InternalClear(MonoString* categoryName, LogVerbosity verbosity);
		static MonoArray* InternalGetMessages();

		typedef void(B3D_THUNKCALL* OnAddedThunkDef)(MonoString*, u32, MonoString*, MonoException**);

		static OnAddedThunkDef onAddedThunk;
	};

	/** @} */
} // namespace bs
