//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestResultWriter.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "String/B3DString.h"
#include "ThirdParty/json.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#if B3D_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace
{
	using namespace b3d;

	String GetTimestamp()
	{
		std::time_t now = std::time(nullptr);
		std::tm* utc = std::gmtime(&now);
		std::ostringstream ss;
		ss << std::put_time(utc, "%Y-%m-%dT%H:%M:%SZ");
		return String(ss.str().c_str());
	}

	/** Console color codes for colored output. */
	enum class ConsoleColor
	{
		Default,
		Green,
		Red,
		Yellow,
		Cyan
	};

#if B3D_PLATFORM_WIN32
	HANDLE gConsoleHandle = INVALID_HANDLE_VALUE;
	WORD gDefaultAttributes = 0;
	bool gSupportsColor = false;
	bool gColorInitialized = false;

	void InitConsoleColor()
	{
		if (gColorInitialized)
			return;

		gConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		if (GetConsoleScreenBufferInfo(gConsoleHandle, &consoleInfo))
		{
			gDefaultAttributes = consoleInfo.wAttributes;
			gSupportsColor = true;
		}
		else
			gSupportsColor = false;

		gColorInitialized = true;
	}

	void SetConsoleColor(ConsoleColor color)
	{
		InitConsoleColor();
		if (!gSupportsColor)
			return;

		WORD attributes = gDefaultAttributes;
		switch (color)
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
			attributes = gDefaultAttributes;
			break;
		}
		SetConsoleTextAttribute(gConsoleHandle, attributes);
	}

	void ResetConsoleColor()
	{
		if (!gSupportsColor)
			return;
		SetConsoleTextAttribute(gConsoleHandle, gDefaultAttributes);
	}
#else
	void SetConsoleColor(ConsoleColor color)
	{
		const char* code = "\033[0m";
		switch (color)
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
	}

	void ResetConsoleColor()
	{
		std::cout << "\033[0m";
	}
#endif

	void PrintColored(const String& text, ConsoleColor color)
	{
		SetConsoleColor(color);
		std::cout << text << "\n";
		ResetConsoleColor();
	}

	void PrintSeparator()
	{
		std::cout << "========================================\n";
	}
}

namespace b3d
{
	void TestResultWriter::WriteToJSON(const Path& outputPath, const Vector<TestSuiteResult>& results)
	{
		nlohmann::json root;
		root["type"] = "unit_test";
		root["timestamp"] = GetTimestamp().c_str();

		nlohmann::json suitesArray = nlohmann::json::array();
		u32 totalTests = 0, passedTests = 0, failedTests = 0;
		u64 totalDuration = 0;

		for (const auto& suite : results)
		{
			nlohmann::json suiteJson;
			suiteJson["name"] = suite.Name.c_str();
			suiteJson["totalTests"] = suite.TotalTestCount;
			suiteJson["passedTests"] = suite.PassedTestCount;
			suiteJson["failedTests"] = suite.FailedTestCount;
			suiteJson["durationUs"] = suite.DurationUs;

			nlohmann::json testsArray = nlohmann::json::array();
			for (const auto& test : suite.Tests)
			{
				nlohmann::json testJson;
				testJson["name"] = test.Name.c_str();
				testJson["passed"] = test.Passed;
				testJson["durationUs"] = test.DurationUs;

				if (!test.Failures.empty())
				{
					nlohmann::json failuresArray = nlohmann::json::array();
					for (const auto& failure : test.Failures)
					{
						failuresArray.push_back({
							{"description", failure.Description.c_str()},
							{"function", failure.TestName.c_str()},
							{"file", failure.File.c_str()},
							{"line", failure.Line}
						});
					}
					testJson["failures"] = failuresArray;
				}
				testsArray.push_back(testJson);
			}
			suiteJson["tests"] = testsArray;
			suitesArray.push_back(suiteJson);

			totalTests += suite.TotalTestCount;
			passedTests += suite.PassedTestCount;
			failedTests += suite.FailedTestCount;
			totalDuration += suite.DurationUs;
		}

		root["suites"] = suitesArray;
		root["summary"] = {
			{"totalSuites", results.size()},
			{"totalTests", totalTests},
			{"passedTests", passedTests},
			{"failedTests", failedTests},
			{"totalDurationUs", totalDuration}
		};

		String jsonString(root.dump(2).c_str());
		SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(outputPath);
		if (fileStream)
			fileStream->WriteString(jsonString);
	}

	void TestResultWriter::WriteToConsole(const Vector<TestSuiteResult>& results)
	{
		for (const auto& suite : results)
		{
			PrintSeparator();
			PrintColored("Running Test Suite: " + suite.Name, ConsoleColor::Yellow);
			PrintSeparator();

			u32 testIndex = 0;
			for (const auto& test : suite.Tests)
			{
				testIndex++;
				std::cout << "Running test " << ToString((u64)testIndex) << ": " << test.Name << "... ";

				if (test.Passed)
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

				if (test.DurationUs < 1000)
					std::cout << " (" << ToString(test.DurationUs) << "us)\n";
				else
				{
					float durationMs = test.DurationUs / 1000.0f;
					std::cout << " (" << ToString(durationMs, 2, 0, ' ', std::ios::fixed) << "ms)\n";
				}

				// Print failures inline
				for (const auto& failure : test.Failures)
					std::cout << failure.File << ":" << failure.Line << ": failure: " << failure.Description << "\n";
			}

			std::cout << "\n";
			PrintSeparator();
			PrintColored(suite.Name + " Summary", ConsoleColor::Yellow);
			PrintSeparator();

			std::cout << "Total: " << ToString((u64)suite.TotalTestCount) << " tests\n";

			float passedPercent = (float)suite.PassedTestCount * 100.0f / (float)suite.TotalTestCount;
			PrintColored("Passed: " + ToString((u64)suite.PassedTestCount) + " (" +
				ToString(passedPercent, 1, 0, ' ', std::ios::fixed) + "%)", ConsoleColor::Green);

			if (suite.FailedTestCount > 0)
			{
				float failedPercent = (float)suite.FailedTestCount * 100.0f / (float)suite.TotalTestCount;
				PrintColored("Failed: " + ToString((u64)suite.FailedTestCount) + " (" +
					ToString(failedPercent, 1, 0, ' ', std::ios::fixed) + "%)", ConsoleColor::Red);
			}
			else
				std::cout << "Failed: 0 (0.0%)\n";

			float durationMs = suite.DurationUs / 1000.0f;
			PrintColored("Duration: " + ToString(durationMs, 2, 0, ' ', std::ios::fixed) + "ms", ConsoleColor::Cyan);

			// List failures summary
			Vector<const TestFailureInfo*> failures;
			for (const auto& test : suite.Tests)
			{
				for (const auto& failure : test.Failures)
					failures.push_back(&failure);
			}

			if (!failures.empty())
			{
				std::cout << "\n";
				PrintColored("Failed Tests:", ConsoleColor::Red);
				for (const auto* failure : failures)
				{
					std::cout << "  - " << failure->TestName << "\n";
					std::cout << "    " << failure->File << ":" << failure->Line << "\n";
					std::cout << "    " << failure->Description << "\n";
				}
			}

			PrintSeparator();
			std::cout << "\n" << std::flush;
		}
	}
} // namespace b3d
