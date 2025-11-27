//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DPath.h"

using namespace b3d;

const String CrashHandler::kSCrashReportFolder = "CrashReports";
const String CrashHandler::kSCrashLogName = "log.html";
const String CrashHandler::kSFatalErrorMsg =
	"A fatal error occurred and the program has to terminate!";

const Path& CrashHandler::GetCrashFolder()
{
	static const Path kPath = FileSystem::GetWorkingDirectoryPath() + kSCrashReportFolder +
		GetCrashTimestamp();

	static bool first = true;
	if(first)
	{
		FileSystem::CreateDir(kPath);
		first = false;
	}

	return kPath;
}

void CrashHandler::LogErrorAndStackTrace(const String& errorMsg, const String& stackTrace) const
{
	StringStream errorMessage;
	errorMessage << kSFatalErrorMsg << std::endl;
	errorMessage << errorMsg;
	errorMessage << "\n\nStack trace: \n";
	errorMessage << stackTrace;

	GetDebug().Log(errorMessage.str(), LogVerbosity::Fatal, "Crash");
}

void CrashHandler::LogErrorAndStackTrace(const String& type, const String& description, const String& function, const String& file, u32 line) const
{
	StringStream errorMessage;
	errorMessage << "  - Error: " << type << std::endl;
	errorMessage << "  - Description: " << description << std::endl;
	errorMessage << "  - In function: " << function << std::endl;
	errorMessage << "  - In file: " << file << ":" << line;
	LogErrorAndStackTrace(errorMessage.str(), GetStackTrace());
}

void CrashHandler::SaveCrashLog() const
{
	GetDebug().SaveLog(GetCrashFolder() + kSCrashLogName, SavedLogType::HTML);
}

namespace b3d
{
CrashHandler& GetCrashHandler()
{
	return CrashHandler::Instance();
}
} // namespace b3d
