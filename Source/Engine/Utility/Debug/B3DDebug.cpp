//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/B3DDebug.h"
#include "Debug/B3DLog.h"
#include "Debug/B3DBitmapWriter.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Utility/B3DTime.h"

#if B3D_IS_ENGINE
#	include "B3DEngineConfig.h"
#endif

#if B3D_PLATFORM_WIN32 && B3D_COMPILER_MSVC
#	include <windows.h>
#	include <iostream>

#	undef GetMessage

static void LogToIdeConsole(const b3d::String& message, const char* channel)
{
	static b3d::Mutex mutex;

	b3d::Lock lock(mutex);
	OutputDebugString("[");
	OutputDebugString(channel);
	OutputDebugString("] ");
	OutputDebugString(message.c_str());
	OutputDebugString("\n");

	// Also default output in case we're running without debugger attached
	std::cout << "[" << channel << "] " << message << std::endl;
}
#else
static void LogToIdeConsole(const b3d::String& message, const char* channel)
{
	std::cout << "[" << channel << "] " << message << std::endl;
}
#endif

namespace b3d
{
	B3D_LOG_CATEGORY(LogUncategorized)
	B3D_LOG_CATEGORY(LogFileSystem)
	B3D_LOG_CATEGORY(LogRTTI)
	B3D_LOG_CATEGORY(LogGeneric)
	B3D_LOG_CATEGORY(LogPlatform)
	B3D_LOG_CATEGORY(LogSerialization)
	B3D_LOG_CATEGORY(LogUnitTest)
} // namespace b3d

using namespace b3d;

static void LogToIdeConsole(const String& message, LogVerbosity verbosity)
{
	if(verbosity != LogVerbosity::Log)
	{
		switch(verbosity)
		{
		case LogVerbosity::Fatal:
			LogToIdeConsole(message, "FATAL");
			break;
		case LogVerbosity::Error:
			LogToIdeConsole(message, "ERROR");
			break;
		case LogVerbosity::Warning:
			LogToIdeConsole(message, "WARNING");
			break;
		default:
		case LogVerbosity::Info:
			LogToIdeConsole(message, "INFO");
			break;
		case LogVerbosity::Verbose:
			LogToIdeConsole(message, "VERBOSE");
			break;
		case LogVerbosity::VeryVerbose:
			LogToIdeConsole(message, "VERY_VERBOSE");
			break;
		}
	}
}

void Debug::Log(const String& message, LogVerbosity verbosity, const String& categoryName)
{
	if(mCustomLogCallback)
	{
		// Run the custom callback and if it returns true skip the default action
		if(mCustomLogCallback(message, verbosity, categoryName.c_str()))
			return;
	}

	mLog.LogMessage(message, verbosity, categoryName);
	LogToIdeConsole(message, verbosity);
}

void Debug::WriteAsBmp(u8* rawPixels, u32 bytesPerPixel, u32 width, u32 height, const Path& filePath, bool overwrite) const
{
	if(FileSystem::IsFile(filePath))
	{
		if(overwrite)
			FileSystem::Remove(filePath);
		else
		{
			if(!B3D_ENSURE_LOG(false, "File already exists at specified location: {0}", filePath.ToString()))
				return;
		}
	}

	SPtr<DataStream> ds = FileSystem::CreateAndOpenFile(filePath);

	u32 bmpDataSize = BitmapWriter::GetBmpSize(width, height, bytesPerPixel);
	u8* bmpBuffer = B3DNewMultiple<u8>(bmpDataSize);

	BitmapWriter::RawPixelsToBmp(rawPixels, bmpBuffer, width, height, bytesPerPixel);

	ds->Write(bmpBuffer, bmpDataSize);
	ds->Close();

	B3DDeleteMultiple(bmpBuffer, bmpDataSize);
}

void Debug::TriggerCallbacksInternal()
{
	LogEntry entry;
	while(mLog.GetUnreadEntry(entry))
	{
		OnLogEntryAdded(entry);
	}

	u64 hash = mLog.GetHash();
	if(mLogHash != hash)
	{
		OnLogModified();
		mLogHash = hash;
	}
}

void Debug::SaveLog(const Path& path, SavedLogType type) const
{
	switch(type)
	{
	default:
	case SavedLogType::HTML:
		SaveHtmlLog(path);
		break;
	case SavedLogType::Textual:
		SaveTextLog(path);
		break;
	}
}

void Debug::SaveHtmlLog(const Path& path) const
{
	static const char* style =
		R"(html {
  font-family: sans-serif;
}
			
table
{
	border-collapse: collapse;
	border-spacing: 0;
	empty-cells: show;
	border: 1px solid #cbcbcb;
	width:100%;
	table-layout:fixed;
}

table caption
{
	color: #000;
	font: italic 85%/1 arial, sans-serif;
	padding: 1em 0;
	text-align: center;
}

table td,
table th
{
	border-left: 1px solid #cbcbcb;/*  inner column border */
	border-width: 0 0 0 1px;
	font-size: inherit;
	margin: 0;
	overflow: visible; /*to make ths where the title is really long work*/
	padding: 0.5em 1em; /* cell padding */
}

table td:first-child,
table th:first-child
{
	border-left-width: 0;
}

table thead
{
	background-color: #e0e0e0;
	color: #000;
	text-align: left;
	vertical-align: bottom;
}

table td
{
	background-color: transparent;
	word-wrap:break-word;
	vertical-align: top;
	color: #7D7D7D;
}

.debug-row td {
	background-color: #FFFFFF;
}

.debug-alt-row td {
	background-color: #f2f2f2;
}

.warn-row td {
	background-color: #ffc016;
	color: #5F5F5F;
}

.warn-alt-row td {
	background-color: #fdcb41;
	color: #5F5F5F;
}

.error-row td {
	background-color: #9f1621;
	color: #9F9F9F;
}

.error-alt-row td {
	background-color: #ae1621;
	color: #9F9F9F;
}
)";

	static const char* htmlPreStyleHeader =
		R"(<!DOCTYPE html>
<html lang="en">
<head>
<style type="text/css">
)";

#if B3D_IS_ENGINE
	static const char* htmlPostStyleHeader =
		R"(</style>
<title>Banshee Engine Log</title>
</head>
<body>
)";
#else
	static const char* htmlPostStyleHeader =
		R"(</style>
<title>B3D Framework Log</title>
</head>
<body>
)";
#endif

	static const char* htmlEntriesTableHeader =
		R"(<table border="1" cellpadding="1" cellspacing="1">
	<thead>
		<tr>
			<th scope="col" style="width:85px">Type</th>
			<th scope="col" style="width:70px">Time</th>
			<th scope="col" style="width:160px">Category</th>
			<th scope="col">Description</th>
		</tr>
	</thead>
	<tbody>
)";

	static const char* htmlFooter =
		R"(   </tbody>
</table>
</body>
</html>)";

	StringStream stream;
	stream << htmlPreStyleHeader;
	stream << style;
	stream << htmlPostStyleHeader;
#if B3D_IS_ENGINE
	stream << "<h1>Banshee Engine Log</h1>\n";
#else
	stream << "<h1>B3D Framework Log</h1>\n";
#endif

	stream << "<h2>System information</h2>\n";

	// Write header information
	stream << "<p>B3D Framework version: " << B3D_FRAMEWORK_VERSION_MAJOR << "." << B3D_FRAMEWORK_VERSION_MINOR << "." << B3D_FRAMEWORK_VERSION_PATCH << "</p>\n";

	if(Time::IsStarted())
		stream << "<p>Started on: " << GetTime().GetAppStartUpDateString(false) << "</p>\n";

	SystemInfo systemInfo = PlatformUtility::GetSystemInfo();
	stream << "<p>OS version: " << systemInfo.OsName << " " << (systemInfo.OsIs64Bit ? "64-bit" : "32-bit") << "</p>\n";
	stream << "<h3>CPU information:</h3>\n";
	stream << "<p>CPU vendor: " << systemInfo.CpuManufacturer << "</p>\n";
	stream << "<p>CPU name: " << systemInfo.CpuModel << "</p>\n";
	stream << "<p>CPU clock speed: " << systemInfo.CpuClockSpeedMhz << "Mhz</p>\n";
	stream << "<p>CPU core count: " << systemInfo.CpuNumCores << "</p>\n";

	stream << "<h3>GPU List:</h3>\n";
	if(systemInfo.GpuInfo.NumGpUs == 1)
		stream << "<p>GPU: " << systemInfo.GpuInfo.Names[0] << "</p>\n";
	else
	{
		for(u32 gpuIndex = 0; gpuIndex < systemInfo.GpuInfo.NumGpUs; gpuIndex++)
			stream << "<p>GPU #" << gpuIndex << ": " << systemInfo.GpuInfo.Names[gpuIndex] << "</p>\n";
	}

	// Write log entries
	stream << "<h2>Log entries</h2>\n";
	stream << htmlEntriesTableHeader;

	bool alternate = false;
	Vector<LogEntry> entries = mLog.GetAllEntries();
	for(auto& entry : entries)
	{
		String channelName;

		LogVerbosity verbosity = entry.Verbosity;
		switch(verbosity)
		{
		case LogVerbosity::Fatal:
		case LogVerbosity::Error:
			if(!alternate)
				stream << R"(		<tr class="error-row">)" << std::endl;
			else
				stream << R"(		<tr class="error-alt-row">)" << std::endl;
			break;
		case LogVerbosity::Warning:
			if(!alternate)
				stream << R"(		<tr class="warn-row">)" << std::endl;
			else
				stream << R"(		<tr class="warn-alt-row">)" << std::endl;
			break;
		default:
		case LogVerbosity::Info:
		case LogVerbosity::Log:
		case LogVerbosity::Verbose:
		case LogVerbosity::VeryVerbose:
			if(!alternate)
				stream << R"(		<tr class="debug-row">)" << std::endl;
			else
				stream << R"(		<tr class="debug-alt-row">)" << std::endl;
			break;
		}
		stream << R"(			<td>)" << ToString(verbosity) << R"(</td>)" << std::endl;

		stream << R"(			<td>)" << ToString(entry.LocalTime, false, false, TimeToStringConversionType::Time)
			   << "</td>" << std::endl;

		stream << R"(			<td>)" << entry.CategoryName << "</td>" << std::endl;

		String parsedMessage = StringUtility::ReplaceAll(entry.Message, "\n", "<br>\n");

		stream << R"(			<td>)" << parsedMessage << "</td>" << std::endl;
		stream << R"(		</tr>)" << std::endl;

		alternate = !alternate;
	}

	stream << htmlFooter;

	SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(path);
	fileStream->WriteString(stream.str());
}

/* Internal function to get the given number of spaces, so that the log looks properly indented */
String GetSpacesIndentationInternal(size_t spaceCount)
{
	String tmp;
	for(u8 spaceIndex = 0; spaceIndex < spaceCount; spaceIndex++)
		tmp.append(" ");
	return tmp;
}

void Debug::SaveTextLog(const Path& path) const
{
#if B3D_IS_ENGINE
	static const char* engineHeader = "This is Banshee Engine ";
	static const char* bsfBasedHeader = "Based on B3D Framework ";
#else
	static const char* bsfOnlyHeader = "This is B3D Framework ";
#endif

	StringStream stream;
#if B3D_IS_ENGINE
	stream << engineHeader << B3D_VERSION_MAJOR << "." << B3D_VERSION_MINOR << "." << B3D_VERSION_PATCH << "\n";
	stream << bsfBasedHeader << B3D_FRAMEWORK_VERSION_MAJOR << "." << B3D_FRAMEWORK_VERSION_MINOR << "." << B3D_FRAMEWORK_VERSION_PATCH << "\n";
#else
	stream << bsfOnlyHeader << B3D_FRAMEWORK_VERSION_MAJOR << "." << B3D_FRAMEWORK_VERSION_MINOR << "." << B3D_FRAMEWORK_VERSION_PATCH << "\n";
#endif
	if(Time::IsStarted())
		stream << "Started on: " << GetTime().GetAppStartUpDateString(false) << "\n";

	stream << "\n";
	stream << "System information:\n"
		   << "================================================================================\n";

	SystemInfo systemInfo = PlatformUtility::GetSystemInfo();
	stream << "OS version: " << systemInfo.OsName << " " << (systemInfo.OsIs64Bit ? "64-bit" : "32-bit") << "\n";
	stream << "CPU information:\n";
	stream << "CPU vendor: " << systemInfo.CpuManufacturer << "\n";
	stream << "CPU name: " << systemInfo.CpuModel << "\n";
	stream << "CPU clock speed: " << systemInfo.CpuClockSpeedMhz << "Mhz\n";
	stream << "CPU core count: " << systemInfo.CpuNumCores << "\n";

	stream << "\n";
	stream << "GPU List:\n"
		   << "================================================================================\n";

	if(systemInfo.GpuInfo.NumGpUs == 1)
		stream << "GPU: " << systemInfo.GpuInfo.Names[0] << "\n";
	else
	{
		for(u32 gpuIndex = 0; gpuIndex < systemInfo.GpuInfo.NumGpUs; gpuIndex++)
			stream << "GPU #" << gpuIndex << ": " << systemInfo.GpuInfo.Names[gpuIndex] << "\n";
	}

	stream << "\n";
	stream << "Log entries:\n"
		   << "================================================================================\n";

	Vector<LogEntry> entries = mLog.GetAllEntries();
	for(auto& entry : entries)
	{
		String builtMsg;
		builtMsg.append(ToString(entry.LocalTime, false, true, TimeToStringConversionType::Full));
		builtMsg.append(" ");

		switch(entry.Verbosity)
		{
		case LogVerbosity::Fatal:
			builtMsg.append("[FATAL]");
			break;
		case LogVerbosity::Error:
			builtMsg.append("[ERROR]");
			break;
		case LogVerbosity::Warning:
			builtMsg.append("[WARNING]");
			break;
		case LogVerbosity::Info:
			builtMsg.append("[INFO]");
			break;
		case LogVerbosity::Log:
			builtMsg.append("[LOG]");
			break;
		case LogVerbosity::Verbose:
			builtMsg.append("[VERBOSE]");
			break;
		case LogVerbosity::VeryVerbose:
			builtMsg.append("[VERY_VERBOSE]");
			break;
		}

		if(!entry.CategoryName.empty())
		{
			builtMsg.append(" <");
			builtMsg.append(entry.CategoryName);
			builtMsg.append(">");
		}

		builtMsg.append(" | ");

		String tmpSpaces = GetSpacesIndentationInternal(builtMsg.length());

		String parsedMessage = StringUtility::ReplaceAll(entry.Message, "\n\t\t", "\n" + tmpSpaces);
		builtMsg.append(parsedMessage);

		stream << builtMsg << "\n";
	}

	SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(path);
	fileStream->WriteString(stream.str());
}

namespace b3d
{
B3D_EXPORT Debug& GetDebug()
{
	static Debug debug;
	return debug;
}
} // namespace b3d
