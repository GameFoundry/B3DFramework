//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Debug/BsDebug.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsPath.h"

using namespace bs;

const String CrashHandler::sCrashReportFolder = "CrashReports";
const String CrashHandler::sCrashLogName = u8"log.html";
const String CrashHandler::sFatalErrorMsg =
	"A fatal error occurred and the program has to terminate!";

const Path& CrashHandler::GetCrashFolder()
{
	static const Path path = FileSystem::GetWorkingDirectoryPath() + sCrashReportFolder +
		GetCrashTimestamp();

	static bool first = true;
	if(first)
	{
		FileSystem::CreateDir(path);
		first = false;
	}

	return path;
}

void CrashHandler::LogErrorAndStackTrace(const String& errorMsg, const String& stackTrace) const
{
	StringStream errorMessage;
	errorMessage << sFatalErrorMsg << std::endl;
	errorMessage << errorMsg;
	errorMessage << "\n\nStack trace: \n";
	errorMessage << stackTrace;

	gDebug().Log(errorMessage.str(), LogVerbosity::Fatal);
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
	gDebug().SaveLog(GetCrashFolder() + sCrashLogName, SavedLogType::HTML);
}

namespace bs
{
CrashHandler& gCrashHandler()
{
	return CrashHandler::Instance();
}
} // namespace bs
