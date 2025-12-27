//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DConsoleTestOutput.h"
#include "String/B3DString.h"

#include <iostream>

#if B3D_PLATFORM_WIN32
#	include <windows.h>
#endif

using namespace b3d;

ConsoleTestOutput::ConsoleTestOutput()
{
#if B3D_PLATFORM_WIN32
	mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	if(GetConsoleScreenBufferInfo((HANDLE)mConsoleHandle, &consoleInfo))
	{
		mDefaultAttributes = consoleInfo.wAttributes;
		mSupportsColor = true;
	}
	else
		mSupportsColor = false;
#else
	// Assume ANSI color support on Unix-like systems
	mSupportsColor = true;
#endif
}

ConsoleTestOutput::~ConsoleTestOutput()
{
	ResetConsoleColor();
}

void ConsoleTestOutput::DoOnOutputFail(const String& description, const String& function, const String& file, long line)
{
	// Store failure for summary
	FailureInfo failure;
	failure.TestName = mCurrentTest;
	failure.Description = description;
	failure.File = file;
	failure.Line = line;
	mFailures.push_back(failure);

	mHadFailures = true;

	// Also output immediately in traditional format
	std::cout << file << ":" << line << ": failure: " << description << "\n";
}

void ConsoleTestOutput::DoOnOutputSuccess(const String& testName)
{
	// Success reporting is handled in OnTestEnd
}

void ConsoleTestOutput::DoOnSuiteStart(const String& suiteName)
{
	PrintSeparator();
	PrintColored("Running Test Suite: " + suiteName, ConsoleColor::Yellow);
	PrintSeparator();

	mCurrentTestIndex = 0;
}

void ConsoleTestOutput::DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs)
{
	std::cout << "\n";
	PrintSeparator();
	PrintColored(suiteName + " Summary", ConsoleColor::Yellow);
	PrintSeparator();

	// Statistics
	std::cout << "Total: " << ToString((u64)totalTestCount) << " tests\n";

	float passedPercent = (float)passedTestCount * 100.0f / (float)totalTestCount;
	PrintColored("Passed: " + ToString((u64)passedTestCount) + " (" + ToString(passedPercent, 1, 0, ' ', std::ios::fixed) + "%)", ConsoleColor::Green);

	if(failedTestCount > 0)
	{
		float failedPercent = (float)failedTestCount * 100.0f / (float)totalTestCount;
		PrintColored("Failed: " + ToString((u64)failedTestCount) + " (" + ToString(failedPercent, 1, 0, ' ', std::ios::fixed) + "%)", ConsoleColor::Red);
	}
	else
		std::cout << "Failed: 0 (0.0%)\n";

	// Display duration: show ms with 2 decimal places for precision
	float durationMs = durationUs / 1000.0f;
	PrintColored("Duration: " + ToString(durationMs, 2, 0, ' ', std::ios::fixed) + "ms", ConsoleColor::Cyan);

	// List failures
	if(!mFailures.empty())
	{
		std::cout << "\n";
		PrintColored("Failed Tests:", ConsoleColor::Red);
		for(const auto& failure : mFailures)
		{
			std::cout << "  - " << failure.TestName << "\n";
			std::cout << "    " << failure.File << ":" << failure.Line << "\n";
			std::cout << "    " << failure.Description << "\n";
		}
	}

	PrintSeparator();
	std::cout << "\n" << std::flush;

	// Clear failures for next suite
	mFailures.clear();
}

void ConsoleTestOutput::DoOnTestStart(const String& testName)
{
	mCurrentTest = testName;
	mCurrentTestIndex++;

	// Print progress line without newline
	std::cout << "Running test " << ToString((u64)mCurrentTestIndex) << ": " << testName << "... " << std::flush;
}

void ConsoleTestOutput::DoOnTestEnd(const String& testName, bool passed, u64 durationUs)
{
	// Print result on same line
	if(passed)
	{
		SetConsoleColor(ConsoleColor::Green);
		std::cout << "[PASS]";
		ResetConsoleColor();
	}
	else
	{
		SetConsoleColor(ConsoleColor::Red);
		std::cout << "[FAIL]";
		ResetConsoleColor();
	}

	// Display duration: show us for sub-millisecond, otherwise ms with decimals
	if(durationUs < 1000)
		std::cout << " (" << ToString(durationUs) << "us)\n" << std::flush;
	else
	{
		float durationMs = durationUs / 1000.0f;
		std::cout << " (" << ToString(durationMs, 2, 0, ' ', std::ios::fixed) << "ms)\n" << std::flush;
	}
}

void ConsoleTestOutput::PrintColored(const String& text, ConsoleColor color)
{
	SetConsoleColor(color);
	std::cout << text << "\n";
	ResetConsoleColor();
}

void ConsoleTestOutput::PrintSeparator()
{
	std::cout << "========================================\n";
}

void ConsoleTestOutput::SetConsoleColor(ConsoleColor color)
{
	if(!mSupportsColor)
		return;

#if B3D_PLATFORM_WIN32
	WORD attributes = mDefaultAttributes;

	switch(color)
	{
	case ConsoleColor::Green:
		attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		break;
	case ConsoleColor::Red:
		attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
		break;
	case ConsoleColor::Yellow:
		attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		break;
	case ConsoleColor::Cyan:
		attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		break;
	case ConsoleColor::Default:
	default:
		attributes = mDefaultAttributes;
		break;
	}

	SetConsoleTextAttribute((HANDLE)mConsoleHandle, attributes);
#else
	// ANSI escape codes
	const char* code = "\033[0m";  // Default

	switch(color)
	{
	case ConsoleColor::Green:
		code = "\033[32;1m";
		break;
	case ConsoleColor::Red:
		code = "\033[31;1m";
		break;
	case ConsoleColor::Yellow:
		code = "\033[33;1m";
		break;
	case ConsoleColor::Cyan:
		code = "\033[36;1m";
		break;
	case ConsoleColor::Default:
	default:
		code = "\033[0m";
		break;
	}

	std::cout << code;
#endif
}

void ConsoleTestOutput::ResetConsoleColor()
{
	if(!mSupportsColor)
		return;

#if B3D_PLATFORM_WIN32
	SetConsoleTextAttribute((HANDLE)mConsoleHandle, mDefaultAttributes);
#else
	std::cout << "\033[0m";
#endif
}
