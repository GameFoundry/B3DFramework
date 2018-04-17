//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Debug/BsDebug.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsPath.h"

namespace bs
{
	const String CrashHandler::sCrashReportFolder = "CrashReports";
	const String CrashHandler::sCrashLogName = u8"log.html";
	const String CrashHandler::sFatalErrorMsg =
		"A fatal error occurred and the program has to terminate!";

	CrashHandler& gCrashHandler()
	{
		return CrashHandler::instance();
	}

	const Path& CrashHandler::getCrashFolder()
	{
		static const Path path = FileSystem::getWorkingDirectoryPath() + sCrashReportFolder + 
			getCrashTimestamp();
		
		static bool first = true;
		if (first) 
		{
			FileSystem::createDir(path);
			first = false;
		}

		return path;
	}

	void CrashHandler::logErrorAndStackTrace(const String& errorMsg, const String& stackTrace) const
	{
		StringStream errorMessage;
		errorMessage << sFatalErrorMsg << std::endl;
		errorMessage << errorMsg;
		errorMessage << "\n\nStack trace: \n";
		errorMessage << stackTrace;

		gDebug().logError(errorMessage.str());
	}

	void CrashHandler::logErrorAndStackTrace(const String& type, const String& description, const String& function,
		const String& file, UINT32 line) const
	{
		StringStream errorMessage;
		errorMessage << "  - Error: " << type << std::endl;
		errorMessage << "  - Description: " << description << std::endl;
		errorMessage << "  - In function: " << function << std::endl;
		errorMessage << "  - In file: " << file << ":" << line;
		logErrorAndStackTrace(errorMessage.str(), getStackTrace());
	}

	void CrashHandler::saveCrashLog() const
	{
		gDebug().saveLog(getCrashFolder() + sCrashLogName);
	}
}
