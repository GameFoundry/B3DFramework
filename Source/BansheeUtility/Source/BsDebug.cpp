//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDebug.h"
#include "BsLog.h"
#include "BsException.h"
#include "BsBitmapWriter.h"
#include "BsFileSystem.h"
#include "BsDataStream.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32 && BS_COMPILER == BS_COMPILER_MSVC
#include <windows.h>
#include <iostream>

void logToIDEConsole(const bs::String& message)
{
	OutputDebugString(message.c_str());
	OutputDebugString("\n");

	// Also default output in case we're running without debugger attached
	std::cout << message << std::endl;
}
#else
void logToIDEConsole(const bs::String& message)
{
	std::cout << message << std::endl;
}
#endif

namespace bs
{
	void Debug::logDebug(const String& msg)
	{
		mLog.logMsg(msg, (UINT32)DebugChannel::Debug);
		logToIDEConsole(msg);
	}

	void Debug::logWarning(const String& msg)
	{
		mLog.logMsg(msg, (UINT32)DebugChannel::Warning);
		logToIDEConsole(msg);
	}

	void Debug::logError(const String& msg)
	{
		mLog.logMsg(msg, (UINT32)DebugChannel::Error);
		logToIDEConsole(msg);
	}

	void Debug::log(const String& msg, UINT32 channel)
	{
		mLog.logMsg(msg, channel);
		logToIDEConsole(msg);
	}

	void Debug::writeAsBMP(UINT8* rawPixels, UINT32 bytesPerPixel, UINT32 width, UINT32 height, const Path& filePath, 
		bool overwrite) const
	{
		if(FileSystem::isFile(filePath))
		{
			if(overwrite)
				FileSystem::remove(filePath);
			else
				BS_EXCEPT(FileNotFoundException, "File already exists at specified location: " + filePath.toString());
		}

		SPtr<DataStream> ds = FileSystem::createAndOpenFile(filePath);

		UINT32 bmpDataSize = BitmapWriter::getBMPSize(width, height, bytesPerPixel);
		UINT8* bmpBuffer = bs_newN<UINT8>(bmpDataSize);

		BitmapWriter::rawPixelsToBMP(rawPixels, bmpBuffer, width, height, bytesPerPixel);

		ds->write(bmpBuffer, bmpDataSize);
		ds->close();

		bs_deleteN(bmpBuffer, bmpDataSize);
	}

	void Debug::_triggerCallbacks()
	{
		LogEntry entry;
		while (mLog.getUnreadEntry(entry))
		{
			onLogEntryAdded(entry);
		}

		UINT64 hash = mLog.getHash();
		if(mLogHash != hash)
		{
			onLogModified();
			mLogHash = hash;
		}
	}

	void Debug::saveLog(const Path& path) const
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

		static const char* htmlPostStyleHeader =
			R"(</style>
<title>Banshee Engine Log</title>
</head>
<body>
)";

		static const char* htmlEntriesTableHeader =
			R"(<table border="1" cellpadding="1" cellspacing="1">
	<thead>
		<tr>
			<th scope="col" style="width:60px">Type</th>
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
		stream << "<h1>Banshee Engine Log</h1>\n";

		stream << "<h2>System information</h2>\n";

		// Write header information
		stream << "<p>Banshee version: " << BS_VERSION_MAJOR << "." << BS_VERSION_MINOR << "</p>\n";

		SystemInfo systemInfo = PlatformUtility::getSystemInfo();
		stream << "<p>OS version: " << systemInfo.osName << " " << (systemInfo.osIs64Bit ? "64-bit" : "32-bit") << "</p>\n";
		stream << "<p>CPU vendor: " << systemInfo.cpuManufacturer << "</p>\n";
		stream << "<p>CPU name: " << systemInfo.cpuModel << "</p>\n";
		stream << "<p>CPU clock speed: " << systemInfo.cpuClockSpeedMhz << "Mhz</p>\n";
		stream << "<p>CPU core count: " << systemInfo.cpuNumCores << "</p>\n";

		if(systemInfo.gpuInfo.numGPUs == 1)
			stream << "<p>GPU: " << systemInfo.gpuInfo.names[0] << "</p>\n";
		else
		{
			for(UINT32 i = 0; i < systemInfo.gpuInfo.numGPUs; i++)
				stream << "<p>GPU #" << i << ": " << systemInfo.gpuInfo.names[i] << "</p>\n";
		}

		// Write log entries
		stream << "<h2>Log entries</h2>\n";
		stream << htmlEntriesTableHeader;

		bool alternate = false;
		Vector<LogEntry> entries = mLog.getAllEntries();
		for (auto& entry : entries)
		{
			String channelName;
			if (entry.getChannel() == (UINT32)DebugChannel::Error || entry.getChannel() == (UINT32)DebugChannel::CompilerError)
			{
				if (!alternate)
					stream << R"(		<tr class="error-row">)" << std::endl;
				else
					stream << R"(		<tr class="error-alt-row">)" << std::endl;

				stream << R"(			<td>Error</td>)" << std::endl;
			}
			else if (entry.getChannel() == (UINT32)DebugChannel::Warning || entry.getChannel() == (UINT32)DebugChannel::CompilerWarning)
			{
				if (!alternate)
					stream << R"(		<tr class="warn-row">)" << std::endl;
				else
					stream << R"(		<tr class="warn-alt-row">)" << std::endl;

				stream << R"(			<td>Warning</td>)" << std::endl;
			}
			else
			{
				if (!alternate)
					stream << R"(		<tr class="debug-row">)" << std::endl;
				else
					stream << R"(		<tr class="debug-alt-row">)" << std::endl;

				stream << R"(			<td>Debug</td>)" << std::endl;
			}

			String parsedMessage = StringUtil::replaceAll(entry.getMessage(), "\n", "<br>\n");

			stream << R"(			<td>)" << parsedMessage << "</td>" << std::endl;
			stream << R"(		</tr>)" << std::endl;

			alternate = !alternate;
		}

		stream << htmlFooter;

		SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(path);
		fileStream->writeString(stream.str());
	}

	BS_UTILITY_EXPORT Debug& gDebug()
	{
		static Debug debug;
		return debug;
	}
}
