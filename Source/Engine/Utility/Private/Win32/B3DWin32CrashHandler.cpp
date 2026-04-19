//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "Utility/B3DDynamicLibrary.h"
#include "FileSystem/B3DFileSystem.h"
#include "windows.h"
#include <psapi.h>

// Disable warning in VS2015 that's not under my control
#pragma warning(disable : 4091)
#include "DbgHelp.h"
#include "String/B3DUnicode.h"
#pragma warning(default : 4091)

static const char* sMiniDumpName = "minidump.dmp";

using namespace b3d;

CrashHandler::CrashHandler(const CrashHandlerSettings& settings)
	: mSettings(settings)
{
	m = B3DNew<Data>();
}

CrashHandler::~CrashHandler()
{
	B3DDelete(m);
}

/**
 * Returns the raw stack trace using the provided context. Raw stack trace contains only function addresses.
 *
 * @param[in]	context		Processor context from which to start the stack trace.
 * @param[in]	stackTrace	Output parameter that will contain the function addresses. First address is the deepest
 * 							called function and following address is its caller and so on.
 * @return					Number of functions in the call stack.
 */
u32 Win32GetRawStackTrace(CONTEXT context, u64 stackTrace[B3D_MAX_STACKTRACE_DEPTH])
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	u32 machineType;

	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(stackFrame));

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrStack.Offset = context.Rsp;
	stackFrame.AddrFrame.Offset = context.Rbp;

	machineType = IMAGE_FILE_MACHINE_AMD64;
#else
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrFrame.Offset = context.Ebp;

	machineType = IMAGE_FILE_MACHINE_I386;
#endif

	u32 numEntries = 0;
	while(true)
	{
		if(!StackWalk64(machineType, hProcess, hThread, &stackFrame, &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
		{
			break;
		}

		if(numEntries < B3D_MAX_STACKTRACE_DEPTH)
			stackTrace[numEntries] = stackFrame.AddrPC.Offset;

		numEntries++;

		if(stackFrame.AddrPC.Offset == 0 || stackFrame.AddrFrame.Offset == 0)
			break;
	}

	return numEntries;
}

/**
 * Returns a string containing a stack trace using the provided context. If function can be found in the symbol table
 * its readable name will be present in the stack trace, otherwise just its address.
 *
 * @param[in]	context		Processor context from which to start the stack trace.
 * @param[in]	skip		Number of bottom-most call stack entries to skip.
 * @return					String containing the call stack with each function on its own line.
 */
String Win32GetStackTrace(CONTEXT context, u32 skip = 0)
{
	u64 rawStackTrace[B3D_MAX_STACKTRACE_DEPTH];
	u32 numEntries = Win32GetRawStackTrace(context, rawStackTrace);

	numEntries = std::min((u32)B3D_MAX_STACKTRACE_DEPTH, numEntries);

	u32 bufferSize = sizeof(PIMAGEHLP_SYMBOL64) + B3D_MAX_STACKTRACE_NAME_BYTES;
	u8* buffer = (u8*)B3DAllocate(bufferSize);

	PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
	symbol->SizeOfStruct = bufferSize;
	symbol->MaxNameLength = B3D_MAX_STACKTRACE_NAME_BYTES;

	HANDLE hProcess = GetCurrentProcess();

	StringStream outputStream;
	for(u32 entryIndex = skip; entryIndex < numEntries; entryIndex++)
	{
		if(entryIndex > skip)
			outputStream << std::endl;

		DWORD64 funcAddress = rawStackTrace[entryIndex];

		// Output function name
		DWORD64 dummy;
		if(SymGetSymFromAddr64(hProcess, funcAddress, &dummy, symbol))
			outputStream << StringUtility::Format("{0}() - ", symbol->Name);

		// Output file name and line
		IMAGEHLP_LINE64 lineData;
		lineData.SizeOfStruct = sizeof(lineData);

		String addressString = ToString(funcAddress, 0, ' ', std::ios::hex);

		DWORD column;
		if(SymGetLineFromAddr64(hProcess, funcAddress, &column, &lineData))
		{
			Path filePath = lineData.FileName;

			outputStream << StringUtility::Format("0x{0} File[{1}:{2} ({3})]", addressString, filePath.GetFilename(), (u32)lineData.LineNumber, (u32)column);
		}
		else
		{
			outputStream << StringUtility::Format("0x{0}", addressString);
		}

		// Output module name
		IMAGEHLP_MODULE64 moduleData;
		moduleData.SizeOfStruct = sizeof(moduleData);

		if(SymGetModuleInfo64(hProcess, funcAddress, &moduleData))
		{
			Path filePath = moduleData.ImageName;

			outputStream << StringUtility::Format(" Module[{0}]", filePath.GetFilename());
		}
	}

	B3DFree(buffer);

	return outputStream.str();
}

typedef bool(WINAPI* EnumProcessModulesType)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD(WINAPI* GetModuleBaseNameType)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
typedef DWORD(WINAPI* GetModuleFileNameExType)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
typedef bool(WINAPI* GetModuleInformationType)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

static DynamicLibrary* gPSAPILib = nullptr;

static EnumProcessModulesType gEnumProcessModules;
static GetModuleBaseNameType gGetModuleBaseName;
static GetModuleFileNameExType gGetModuleFileNameEx;
static GetModuleInformationType gGetModuleInformation;

/**	Dynamically load the PSAPI.dll and the required symbols, if not already loaded. */
void Win32InitPsapi()
{
	if(gPSAPILib != nullptr)
		return;

	gPSAPILib = B3DNew<DynamicLibrary>("PSAPI.dll");
	gEnumProcessModules = (EnumProcessModulesType)gPSAPILib->GetSymbol("EnumProcessModules");
	gGetModuleBaseName = (GetModuleBaseNameType)gPSAPILib->GetSymbol("GetModuleFileNameExA");
	gGetModuleFileNameEx = (GetModuleFileNameExType)gPSAPILib->GetSymbol("GetModuleBaseNameA");
	gGetModuleInformation = (GetModuleInformationType)gPSAPILib->GetSymbol("GetModuleInformation");
}

/**	Unloads the PSAPI.dll if is loaded. */
void Win32UnloadPsapi()
{
	if(gPSAPILib == nullptr)
		return;

	gPSAPILib->Unload();
	B3DDelete(gPSAPILib);
	gPSAPILib = nullptr;
}

static bool gSymbolsLoaded = false;

/**
 * Loads symbols for all modules in the current process. Loaded symbols allow the stack walker to retrieve human
 * readable method, file, module names and other information.
 */
void Win32LoadSymbols()
{
	if(gSymbolsLoaded)
		return;

	HANDLE hProcess = GetCurrentProcess();
	u32 options = SymGetOptions();

	options |= SYMOPT_LOAD_LINES;
	options |= SYMOPT_EXACT_SYMBOLS;
	options |= SYMOPT_UNDNAME;
	options |= SYMOPT_FAIL_CRITICAL_ERRORS;
	options |= SYMOPT_NO_PROMPTS;

	SymSetOptions(options);
	if(!SymInitialize(hProcess, nullptr, false))
	{
		B3D_LOG(Error, LogGeneric, "SymInitialize failed. Error code: {0}", +(u32)GetLastError());
		return;
	}

	DWORD bufferSize;
	gEnumProcessModules(hProcess, nullptr, 0, &bufferSize);

	HMODULE* modules = (HMODULE*)B3DAllocate(bufferSize);
	gEnumProcessModules(hProcess, modules, bufferSize, &bufferSize);

	u32 numModules = bufferSize / sizeof(HMODULE);
	for(u32 moduleIndex = 0; moduleIndex < numModules; moduleIndex++)
	{
		MODULEINFO moduleInfo;

		char moduleName[B3D_MAX_STACKTRACE_NAME_BYTES];
		char imageName[B3D_MAX_STACKTRACE_NAME_BYTES];

		gGetModuleInformation(hProcess, modules[moduleIndex], &moduleInfo, sizeof(moduleInfo));
		gGetModuleFileNameEx(hProcess, modules[moduleIndex], imageName, B3D_MAX_STACKTRACE_NAME_BYTES);
		gGetModuleBaseName(hProcess, modules[moduleIndex], moduleName, B3D_MAX_STACKTRACE_NAME_BYTES);

		char pdbSearchPath[B3D_MAX_STACKTRACE_NAME_BYTES];
		char* fileName = nullptr;
		GetFullPathNameA(moduleName, B3D_MAX_STACKTRACE_NAME_BYTES, pdbSearchPath, &fileName);
		*fileName = '\0';

		SymSetSearchPath(GetCurrentProcess(), pdbSearchPath);

		DWORD64 moduleAddress = SymLoadModule64(hProcess, modules[moduleIndex], imageName, moduleName, (DWORD64)moduleInfo.lpBaseOfDll, (DWORD)moduleInfo.SizeOfImage);

		if(moduleAddress != 0)
		{
			IMAGEHLP_MODULE64 imageInfo;
			memset(&imageInfo, 0, sizeof(imageInfo));
			imageInfo.SizeOfStruct = sizeof(imageInfo);

			if(!SymGetModuleInfo64(GetCurrentProcess(), moduleAddress, &imageInfo))
			{
				B3D_LOG(Warning, LogPlatform, "Failed retrieving module info for module: {0}. Error code: {1}", moduleName, (u32)GetLastError());
			}
			else
			{
				// Disabled because too much spam in the log, enable as needed
#if 0
					if (imageInfo.SymType == SymNone)
						B3D_LOG(Warning, LogPlatform, "Failed loading symbols for module: {0}", moduleName);
#endif
			}
		}
		else
		{
			B3D_LOG(Warning, LogPlatform, "Failed loading module {0}.Error code: {1}. Search path: {2}. Image name: {3}", moduleName, (u32)GetLastError(), pdbSearchPath, imageName);
		}
	}

	B3DFree(modules);
	gSymbolsLoaded = true;
}

/**	Converts an exception record into a human readable error message. */
String Win32GetExceptionMessage(EXCEPTION_RECORD* record)
{
	String exceptionAddress = ToString((u64)record->ExceptionAddress, 0, ' ', std::ios::hex);

	String format;
	switch(record->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		{
			DWORD_PTR violatedAddress = 0;
			if(record->NumberParameters == 2)
			{
				if(record->ExceptionInformation[0] == 0)
					format = "Unhandled exception at 0x{0}. Access violation reading 0x{1}.";
				else if(record->ExceptionInformation[0] == 8)
					format = "Unhandled exception at 0x{0}. Access violation DEP 0x{1}.";
				else
					format = "Unhandled exception at 0x{0}. Access violation writing 0x{1}.";

				violatedAddress = record->ExceptionInformation[1];
			}
			else
				format = "Unhandled exception at 0x{0}. Access violation.";

			String violatedAddressStr = ToString((u64)violatedAddress, 0, ' ', std::ios::hex);
			return StringUtility::Format(format, exceptionAddress, violatedAddressStr);
		}
	case EXCEPTION_IN_PAGE_ERROR:
		{
			DWORD_PTR violatedAddress = 0;
			DWORD_PTR code = 0;
			if(record->NumberParameters == 3)
			{
				if(record->ExceptionInformation[0] == 0)
					format = "Unhandled exception at 0x{0}. Page fault reading 0x{1} with code 0x{2}.";
				else if(record->ExceptionInformation[0] == 8)
					format = "Unhandled exception at 0x{0}. Page fault DEP 0x{1} with code 0x{2}.";
				else
					format = "Unhandled exception at 0x{0}. Page fault writing 0x{1} with code 0x{2}.";

				violatedAddress = record->ExceptionInformation[1];
				code = record->ExceptionInformation[3];
			}
			else
				format = "Unhandled exception at 0x{0}. Page fault.";

			String violatedAddressStr = ToString((u64)violatedAddress, 0, ' ', std::ios::hex);
			String codeStr = ToString((u64)code, 0, ' ', std::ios::hex);
			return StringUtility::Format(format, exceptionAddress, violatedAddressStr, codeStr);
		}
	case STATUS_ARRAY_BOUNDS_EXCEEDED:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access an out of range array element.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access missaligned data.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		{
			format = "Unhandled exception at 0x{0}. Floating point operand too small.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Floating point operation attempted to divide by zero.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_INVALID_OPERATION:
		{
			format = "Unhandled exception at 0x{0}. Floating point invalid operation.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point overflow.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_UNDERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point underflow.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_FLT_STACK_CHECK:
		{
			format = "Unhandled exception at 0x{0}. Floating point stack overflow/underflow.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute an illegal instruction.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_PRIV_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute a private instruction.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Integer operation attempted to divide by zero.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_INT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Integer operation result has overflown.";
			return StringUtility::Format(format, exceptionAddress);
		}
	case EXCEPTION_STACK_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Stack overflow.";
			return StringUtility::Format(format, exceptionAddress);
		}
	default:
		{
			format = "Unhandled exception at 0x{0}. Code 0x{1}.";

			String exceptionCode = ToString((u32)record->ExceptionCode, 0, ' ', std::ios::hex);
			return StringUtility::Format(format, exceptionAddress, exceptionCode);
		}
	}
}

struct MiniDumpParams
{
	Path FilePath;
	EXCEPTION_POINTERS* ExceptionData;
};

DWORD CALLBACK Win32WriteMiniDumpWorker(void* data)
{
	MiniDumpParams* params = (MiniDumpParams*)data;

	WString pathString = UTF8::ToWide(params->FilePath.ToString());
	HANDLE hFile = CreateFileW(pathString.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

		DumpExceptionInfo.ThreadId = GetCurrentThreadId();
		DumpExceptionInfo.ExceptionPointers = params->ExceptionData;
		DumpExceptionInfo.ClientPointers = false;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &DumpExceptionInfo, nullptr, nullptr);
		CloseHandle(hFile);
	}

	return 0;
}

void Win32WriteMiniDump(const Path& filePath, EXCEPTION_POINTERS* exceptionData)
{
	MiniDumpParams param = { filePath, exceptionData };

	// Write minidump on a second thread in order to preserve the current thread's call stack
	DWORD threadId = 0;
	HANDLE hThread = CreateThread(nullptr, 0, &Win32WriteMiniDumpWorker, &param, 0, &threadId);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

struct CrashHandler::Data
{
	Mutex Mutex;
};

void Win32PopupErrorMessageBox(const WString& msg, const Path& folder)
{
	WString simpleErrorMessage = msg + L"\n\nFor more information check the crash report located at:\n " + UTF8::ToWide(folder.ToString());

#if B3D_IS_ENGINE
	MessageBoxW(nullptr, simpleErrorMessage.c_str(), L"Banshee fatal error!", MB_OK);
#else
	MessageBoxW(nullptr, simpleErrorMessage.c_str(), L"B3D Framework fatal error!", MB_OK);
#endif
}

void CrashHandler::ReportCrash(const String& type, const String& description, const String& function, const String& file, u32 line) const
{
	if(mSettings.OnBeforeReportCrash)
	{
		if(mSettings.OnBeforeReportCrash(type, description, function, file, line))
			return;
	}

	// Win32 debug methods are not thread safe
	Lock lock(m->Mutex);

	LogErrorAndStackTrace(type, description, function, file, line);

	if(mSettings.OnCrashPrintedToLog)
	{
		if(mSettings.OnCrashPrintedToLog())
			return;
	}

	SaveCrashLog();

	Win32WriteMiniDump(GetCrashFolder() + String(sMiniDumpName), nullptr);
	Win32PopupErrorMessageBox(ToWString(kSFatalErrorMsg), GetCrashFolder());

	DebugBreak();

	// Note: Potentially also log Windows Error Report and/or send crash data to server
}

int CrashHandler::ReportCrash(void* exceptionDataPtr) const
{
	if(mSettings.OnBeforeWindowsSehReportCrash)
	{
		if(mSettings.OnBeforeWindowsSehReportCrash(exceptionDataPtr))
			return EXCEPTION_EXECUTE_HANDLER;
	}

	EXCEPTION_POINTERS* exceptionData = (EXCEPTION_POINTERS*)exceptionDataPtr;

	// Win32 debug methods are not thread safe
	Lock lock(m->Mutex);

	Win32InitPsapi();
	Win32LoadSymbols();

	LogErrorAndStackTrace(Win32GetExceptionMessage(exceptionData->ExceptionRecord), Win32GetStackTrace(*exceptionData->ContextRecord, 0));

	if(mSettings.OnCrashPrintedToLog)
	{
		if(mSettings.OnCrashPrintedToLog())
			return EXCEPTION_EXECUTE_HANDLER;
	}

	SaveCrashLog();

	Win32WriteMiniDump(GetCrashFolder() + String(sMiniDumpName), exceptionData);
	Win32PopupErrorMessageBox(ToWString(kSFatalErrorMsg), GetCrashFolder());

	DebugBreak();

	// Note: Potentially also log Windows Error Report and/or send crash data to server

	return EXCEPTION_EXECUTE_HANDLER;
}

String CrashHandler::GetCrashTimestamp()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);

	String timeStamp = "{0}{1}{2}_{3}{4}";
	String strYear = ToString(systemTime.wYear, 4, '0');
	String strMonth = ToString(systemTime.wMonth, 2, '0');
	String strDay = ToString(systemTime.wDay, 2, '0');
	String strHour = ToString(systemTime.wHour, 2, '0');
	String strMinute = ToString(systemTime.wMinute, 2, '0');
	return StringUtility::Format(timeStamp, strYear, strMonth, strDay, strHour, strMinute);
}

String CrashHandler::GetStackTrace()
{
	CONTEXT context;
	RtlCaptureContext(&context);

	Win32InitPsapi();
	Win32LoadSymbols();
	return Win32GetStackTrace(context, 2);
}
