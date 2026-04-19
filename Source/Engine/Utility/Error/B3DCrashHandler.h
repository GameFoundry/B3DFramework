//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <functional>

#define B3D_MAX_STACKTRACE_DEPTH 200
#define B3D_MAX_STACKTRACE_NAME_BYTES 1024

namespace b3d
{
	/** Struct for holding crash handler settings */
	struct CrashHandlerSettings
	{
		/** Called when reporting a crash begins. Return true to skip default action. */
		std::function<bool(const String& type, const String& description, const String& function, const String& file, u32 line)> OnBeforeReportCrash;

#if B3D_PLATFORM_WIN32
		/** Called when a windows SEH exception is started to be handled. Return true to skip default action */
		std::function<bool(void* exceptionData)> OnBeforeWindowsSehReportCrash;
#endif
		/**
		 * Called after the crash callstack is written to log. Return true to skip writing to file and doing
		 * further other on crash actions
		 */
		std::function<bool()> OnCrashPrintedToLog;

		/** If true then on UNIX a signal handler is not automatically registered to call the crash handler */
		bool DisableCrashSignalHandler = false;
	};

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Error-Internal
	 *  @{
	 */

	/** Saves crash data and notifies the user when a crash occurs. */
	// TODO - Crashes are reported in the same process as the main application. This can be a problem if the crash was caused
	// by heap. Any further use of the heap by the reporting methods will cause a silent crash, failing to log it. A more
	// appropriate way of doing it should be to resume another process to actually handle the crash.
	//  - Perhaps an even better option would be to use a private heap for all engine allocations. So when corruptions does
	//    happen the crash handler can use the default heap with no issues.
	class B3D_EXPORT CrashHandler
	{
	public:
		CrashHandler(const CrashHandlerSettings& settings);
		~CrashHandler();

		/** Constructs and starts the module. */
		static void StartUp(const CrashHandlerSettings& settings = CrashHandlerSettings())
		{
			if(InstanceInternal() == nullptr)
				InstanceInternal() = B3DNew<CrashHandler>(settings);
		}

		/** Shuts down this module and frees any resources it is using. */
		static void ShutDown()
		{
			if(InstanceInternal() != nullptr)
			{
				B3DDelete(InstanceInternal());
				InstanceInternal() = nullptr;
			}
		}

		/** Returns a reference to the module instance. */
		static CrashHandler& Instance() { return *InstanceInternal(); }

		/**
		 * Records a crash with a custom error message.
		 *
		 * @param	type		Type of the crash that occurred. For example "InvalidParameter".
		 * @param	description	More detailed description of the issue that caused the crash.
		 * @param	function	Optional name of the function where the error occurred.
		 * @param	file		Optional name of the source code file in which the code that crashed the program exists.
		 * @param	line		Optional source code line at which the crash was triggered at.
		 */
		void ReportCrash(const String& type, const String& description, const String& function = StringUtility::kBlank, const String& file = StringUtility::kBlank, u32 line = 0) const;

#if B3D_PLATFORM_WIN32
		/**
		 * Records a crash resulting from a Windows-specific SEH exception.
		 *
		 * @param	exceptionData	Exception data returned from GetExceptionInformation()
		 * @return					Code that signals the __except exception handler on how to proceed.
		 *
		 * @note	Available in Windows builds only.
		 */
		int ReportCrash(void* exceptionData) const;
#endif

		/**
		 * Returns a string containing a current stack trace. If function can be found in the symbol table its readable
		 * name will be present in the stack trace, otherwise just its address.
		 *
		 * @return	String containing the call stack with each function on its own line.
		 */
		static String GetStackTrace();

	private:
		/** Does what it says. Internal utility function used by reportCrash(). */
		void LogErrorAndStackTrace(const String& message, const String& stackTrace) const;
		/** Does what it says. Internal utility function used by reportCrash(). */
		void LogErrorAndStackTrace(const String& type, const String& description, const String& function, const String& file, u32 line) const;
		/** Does what it says. Internal utility function used by reportCrash(). */
		void SaveCrashLog() const;
		/** Creates the crash report directory and returns its path. */
		static const Path& GetCrashFolder();
		/** Returns the current time as a string timestamp.  This is used
		 * to name the crash report directory.. */
		static String GetCrashTimestamp();

		/** Returns a singleton instance of this module. */
		static CrashHandler*& InstanceInternal()
		{
			static CrashHandler* inst = nullptr;
			return inst;
		}

		/** Handling customization callbacks */
		CrashHandlerSettings mSettings;

		/** The name of the crash reports directory. */
		static const String kSCrashReportFolder;
		/** The name of the HTML crash log file. */
		static const String kSCrashLogName;
		/** Error message to display on program failure. */
		static const String kSFatalErrorMsg;

#if B3D_PLATFORM_WIN32
		struct Data;
		Data* m;
#endif
	};

	/** Easier way of accessing the CrashHandler. */
	B3D_EXPORT CrashHandler& GetCrashHandler();

	/** @} */
	/** @} */
} // namespace b3d
