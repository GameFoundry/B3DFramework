//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <csignal>

namespace bs
{
	INT32 SIGNALS[] = { SIGFPE, SIGILL, SIGSEGV, SIGTERM };
	struct sigaction gSavedSignals[4];

	void signalHandler(int signal, siginfo_t* info, void* context)
	{
		// Restore old signal handlers
		INT32 i = 0;
		for(auto& entry : SIGNALS)
		{
			sigaction(entry, &gSavedSignals[i], nullptr);
			i++;
		}

		const char* signalNameSz = strsignal(signal);

		String signalName;
		if(signalNameSz)
			signalName = signalNameSz;
		else
			signalName = "Unknown signal #" + toString(signal);

		// Note: Not safe to grab a stack-trace here (nor do memory allocations), but we might as well try since we're
		// crashing anyway
		CrashHandler::instance().reportCrash(signalName, "Received fatal signal", "", "");

		kill(getpid(), signal);
		exit(signal);
	}

	CrashHandler::CrashHandler(const CrashHandlerSettings& settings) :
		mSettings(settings)
	{
		if(mSettings.disableCrashSignalHandler)
			return;

		struct sigaction action;
		sigemptyset(&action.sa_mask);
		action.sa_sigaction = &signalHandler;
		action.sa_flags = SA_SIGINFO;

		INT32 i = 0;
		for(auto& entry : SIGNALS)
		{
			memset(&gSavedSignals[i], 0, sizeof(struct sigaction));
			sigaction(entry, &action, &gSavedSignals[i]);

			i++;
		}
	}

	CrashHandler::~CrashHandler() {}

	String CrashHandler::getCrashTimestamp()
	{
		std::time_t t = time(0);
		struct tm *now = localtime(&t);

		String timeStamp = "{0}{1}{2}_{3}{4}";
		String strYear = toString(now->tm_year, 4, '0');
		String strMonth = toString(now->tm_mon, 2, '0');
		String strDay = toString(now->tm_mday, 2, '0');
		String strHour = toString(now->tm_hour, 2, '0');
		String strMinute = toString(now->tm_min, 2, '0');
		return StringUtil::format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
	}

	String CrashHandler::getStackTrace()
	{
		StringStream stackTrace;
		void* trace[BS_MAX_STACKTRACE_DEPTH];

		int traceSize = backtrace(trace, BS_MAX_STACKTRACE_DEPTH);
		char** messages = backtrace_symbols(trace, traceSize);

		for (int i = 0; i < traceSize && messages != nullptr; ++i)
		{
#if BS_PLATFORM == BS_PLATFORM_OSX
			stackTrace << std::to_string(i) << ") " << messages[i];

			// Try parsing a human readable name
			Dl_info info;
			if (dladdr(trace[i], &info) && info.dli_sname)
			{
				stackTrace << ": ";

				if (info.dli_sname[0] == '_')
				{
					int status = -1;
					char* demangledName = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);

					if(status == 0)
						stackTrace << demangledName;
					else
						stackTrace << info.dli_sname;

					free(demangledName);
				}
				else
					stackTrace << info.dli_sname;

				// Try to find the line number
				for (char *p = messages[i]; *p; ++p)
				{
					if (*p == '+')
					{
						stackTrace << " " << p;
						break;
					}
				}
			}
			else
				stackTrace << String(messages[i]);
#elif BS_PLATFORM == BS_PLATFORM_LINUX
			// Try to find the characters surrounding the mangled name: '(' and '+'
			char* mangedName = nullptr;
			char* offsetBegin = nullptr;
			char* offsetEnd = nullptr;
			for (char *p = messages[i]; *p; ++p)
			{
				if (*p == '(')
					mangedName = p;
				else if (*p == '+')
					offsetBegin = p;
				else if (*p == ')')
				{
					offsetEnd = p;
					break;
				}
			}

			bool lineContainsMangledSymbol = mangedName != nullptr && offsetBegin != nullptr && offsetEnd != nullptr &&
				mangedName < offsetBegin;

			stackTrace << toString(i) << ") ";

			if (lineContainsMangledSymbol)
			{
				*mangedName++ = '\0';
				*offsetBegin++ = '\0';
				*offsetEnd++ = '\0';

				int status;
				char *real_name = abi::__cxa_demangle(mangedName, 0, 0, &status);
				char *output_name = status == 0 /* Demangling successful */? real_name : mangedName;
				stackTrace << String(messages[i])
						   << ": " << output_name
						   << "+" << offsetBegin << offsetEnd;

				free(real_name);
			}
			else
				stackTrace << String(messages[i]);
#endif

			if (i < traceSize - 1)
				stackTrace << "\n";
		}

		free(messages);

		return stackTrace.str();
	}

	void CrashHandler::reportCrash(const String& type,
								   const String& description,
								   const String& function,
								   const String& file,
								   UINT32 line) const
	{
		if(mSettings.onBeforeReportCrash)
		{
			if(mSettings.onBeforeReportCrash(type, description, function, file, line))
				return;
		}

		logErrorAndStackTrace(type, description, function, file, line);

		if(mSettings.onCrashPrintedToLog)
		{
			if(mSettings.onCrashPrintedToLog())
				return;
		}

		saveCrashLog();

		// Allow the debugger a chance to attach
		std::raise(SIGINT);
	}
}
