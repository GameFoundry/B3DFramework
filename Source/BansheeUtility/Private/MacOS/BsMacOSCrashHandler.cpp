//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Prerequisites/BsPrerequisitesUtil.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <csignal>

namespace bs
{
	CrashHandler::CrashHandler() {}
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
		logErrorAndStackTrace(type, description, function, file, line);
		saveCrashLog();

		// Allow the debugger a chance to attach
		std::raise(SIGINT);
	}
}
