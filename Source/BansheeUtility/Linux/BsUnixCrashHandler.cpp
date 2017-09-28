//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Prerequisites/BsPrerequisitesUtil.h"

#include "Debug/BsDebug.h"
#include "FileSystem/BsFileSystem.h"

#include <cxxabi.h>
#include <execinfo.h>

#include <ctime>
#include <sstream>

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
		void *trace[BS_MAX_STACKTRACE_DEPTH];

		int trace_size = backtrace(trace, BS_MAX_STACKTRACE_DEPTH);
		char **messages = backtrace_symbols(trace, trace_size);

		// Most lines returned by backtrace_symbols() look like this:
		//
		//     <path/to/binary>(mangled_symbol+offset) [address]
		//
		// For those lines, we demangle the symbol with abi::__cxa_demangle(),
		// others are good as is.

		for (int i = 0; i < trace_size && messages != NULL; ++i)
		{
			// Try to find the characters surrounding the mangled name: '(' and '+'
			char *mangled_name = NULL, *offset_begin = NULL, *offset_end = NULL;
			for (char *p = messages[i]; *p; ++p)
			{
				if (*p == '(')
					mangled_name = p;
				else if (*p == '+')
					offset_begin = p;
				else if (*p == ')') {
					offset_end = p;
					break;
				}
			}

			bool lineContainsMangledSymbol = mangled_name != NULL
			                                 && offset_begin != NULL
			                                 && offset_end != NULL
			                                 && mangled_name < offset_begin;

			stackTrace << toString(i) << ") ";

			if (lineContainsMangledSymbol)
			{
				*mangled_name++ = '\0';
				*offset_begin++ = '\0';
				*offset_end++ = '\0';

				int status;
				char *real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
				char *output_name = status == 0 /* Demangling successful */? real_name : mangled_name;
				stackTrace << String(messages[i])
				           << ": " << output_name
				           << "+" << offset_begin << offset_end;

				free(real_name);
			}
			else
				stackTrace << String(messages[i]);

			if (i < trace_size - 1)
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
	}
}
