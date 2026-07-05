//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/B3DPrerequisitesUtil.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <csignal>
#include <ctime>

using namespace b3d;

i32 SIGNALS[] = { SIGFPE, SIGILL, SIGSEGV, SIGTERM };
struct sigaction gSavedSignals[4];

void signalHandler(int signal, siginfo_t* info, void* context)
{
	// Restore old signal handlers
	i32 signalIndex = 0;
	for(auto& entry : SIGNALS)
	{
		sigaction(entry, &gSavedSignals[signalIndex], nullptr);
		signalIndex++;
	}

	const char* signalNameSz = strsignal(signal);

	String signalName;
	if(signalNameSz)
		signalName = signalNameSz;
	else
		signalName = "Unknown signal #" + ToString(signal);

	// Note: Not safe to grab a stack-trace here (nor do memory allocations), but we might as well try since we're
	// crashing anyway
	CrashHandler::Instance().ReportCrash(signalName, "Received fatal signal", "", "");

	kill(getpid(), signal);
	exit(signal);
}

CrashHandler::CrashHandler(const CrashHandlerSettings& settings)
	: mSettings(settings)
{
	if(mSettings.DisableCrashSignalHandler)
		return;

	struct sigaction action;
	sigemptyset(&action.sa_mask);
	action.sa_sigaction = &signalHandler;
	action.sa_flags = SA_SIGINFO;

	i32 signalIndex = 0;
	for(auto& entry : SIGNALS)
	{
		memset(&gSavedSignals[signalIndex], 0, sizeof(struct sigaction));
		sigaction(entry, &action, &gSavedSignals[signalIndex]);

		signalIndex++;
	}
}

CrashHandler::~CrashHandler() {}

String CrashHandler::GetCrashTimestamp()
{
	std::time_t t = time(nullptr);

	struct tm timeInfo;
	localtime_r(&t, &timeInfo);

	String timeStamp = "{0}{1}{2}_{3}{4}";

	// tm_year counts years since 1900 and tm_mon is zero-based.
	String strYear = ToString(timeInfo.tm_year + 1900, 4, '0');
	String strMonth = ToString(timeInfo.tm_mon + 1, 2, '0');
	String strDay = ToString(timeInfo.tm_mday, 2, '0');
	String strHour = ToString(timeInfo.tm_hour, 2, '0');
	String strMinute = ToString(timeInfo.tm_min, 2, '0');
	return StringUtility::Format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
}

String CrashHandler::GetStackTrace()
{
	StringStream stackTrace;
	void* trace[B3D_MAX_STACKTRACE_DEPTH];

	int traceSize = backtrace(trace, B3D_MAX_STACKTRACE_DEPTH);
	char** messages = backtrace_symbols(trace, traceSize);

	for(int traceIndex = 0; traceIndex < traceSize && messages != nullptr; ++traceIndex)
	{
#if B3D_PLATFORM_MACOS
		stackTrace << std::to_string(traceIndex) << ") " << messages[traceIndex];

		// Try parsing a human readable name
		Dl_info info;
		if(dladdr(trace[traceIndex], &info) && info.dli_sname)
		{
			stackTrace << ": ";

			if(info.dli_sname[0] == '_')
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
			for(char* p = messages[traceIndex]; *p; ++p)
			{
				if(*p == '+')
				{
					stackTrace << " " << p;
					break;
				}
			}
		}
		else
			stackTrace << String(messages[traceIndex]);
#elif B3D_PLATFORM_LINUX
		// Try to find the characters surrounding the mangled name: '(' and '+'
		char* mangledName = nullptr;
		char* offsetBegin = nullptr;
		char* offsetEnd = nullptr;
		for(char* p = messages[traceIndex]; *p; ++p)
		{
			if(*p == '(')
				mangledName = p;
			else if(*p == '+')
				offsetBegin = p;
			else if(*p == ')')
			{
				offsetEnd = p;
				break;
			}
		}

		bool lineContainsMangledSymbol = mangledName != nullptr && offsetBegin != nullptr && offsetEnd != nullptr &&
			mangledName < offsetBegin;

		stackTrace << ToString(traceIndex) << ") ";

		if(lineContainsMangledSymbol)
		{
			*mangledName++ = '\0';
			*offsetBegin++ = '\0';
			*offsetEnd++ = '\0';

			int status;
			char* real_name = abi::__cxa_demangle(mangledName, 0, 0, &status);
			char* output_name = status == 0 /* Demangling successful */ ? real_name : mangledName;
			stackTrace << String(messages[traceIndex])
					   << ": " << output_name
					   << "+" << offsetBegin << offsetEnd;

			free(real_name);
		}
		else
			stackTrace << String(messages[traceIndex]);
#endif

		if(traceIndex < traceSize - 1)
			stackTrace << "\n";
	}

	free(messages);

	return stackTrace.str();
}

void CrashHandler::ReportCrash(const String& type, const String& description, const String& function, const String& file, u32 line) const
{
	if(mSettings.OnBeforeReportCrash)
	{
		if(mSettings.OnBeforeReportCrash(type, description, function, file, line))
			return;
	}

	LogErrorAndStackTrace(type, description, function, file, line);

	if(mSettings.OnCrashPrintedToLog)
	{
		if(mSettings.OnCrashPrintedToLog())
			return;
	}

	SaveCrashLog();

	// In headless/CI runs there is no one to attach a debugger, so skip the SIGINT hook and let the caller terminate.
	if(!mSettings.SuppressErrorPopup)
	{
		// Allow the debugger a chance to attach
		std::raise(SIGINT);
	}
}
