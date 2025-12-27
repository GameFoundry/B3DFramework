//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DJSONTestOutput.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "ThirdParty/json.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

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

	void WriteResultsAsJSON(const Path& outputPath, const Vector<JSONTestSuiteInfo>& suites)
	{
		nlohmann::json root;
		root["type"] = "unit_test";
		root["timestamp"] = GetTimestamp().c_str();

		nlohmann::json suitesArray = nlohmann::json::array();
		u32 totalTests = 0, passedTests = 0, failedTests = 0;
		u64 totalDuration = 0;

		for(const auto& suite : suites)
		{
			nlohmann::json suiteJson;
			suiteJson["name"] = suite.Name.c_str();
			suiteJson["totalTests"] = suite.TotalTestCount;
			suiteJson["passedTests"] = suite.PassedTestCount;
			suiteJson["failedTests"] = suite.FailedTestCount;
			suiteJson["durationUs"] = suite.DurationUs;

			nlohmann::json testsArray = nlohmann::json::array();
			for(const auto& test : suite.Tests)
			{
				nlohmann::json testJson;
				testJson["name"] = test.Name.c_str();
				testJson["passed"] = test.Passed;
				testJson["durationUs"] = test.DurationUs;

				if(!test.Failures.empty())
				{
					nlohmann::json failuresArray = nlohmann::json::array();
					for(const auto& failure : test.Failures)
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
			{"totalSuites", suites.size()},
			{"totalTests", totalTests},
			{"passedTests", passedTests},
			{"failedTests", failedTests},
			{"totalDurationUs", totalDuration}
		};

		String jsonString(root.dump(2).c_str());
		SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(outputPath);
		if(fileStream)
			fileStream->WriteString(jsonString);
	}
}

using namespace b3d;

JSONTestOutput::JSONTestOutput(const Path& outputPath)
	: mOutputPath(outputPath)
{
}

JSONTestOutput::~JSONTestOutput()
{
	WriteResultsAsJSON(mOutputPath, mSuites);
}

void JSONTestOutput::DoOnOutputFail(const String& description, const String& function,
	const String& file, long line)
{
	JSONTestFailureInfo failure;
	failure.TestName = mCurrentTestName;
	failure.Description = description;
	failure.File = file;
	failure.Line = line;

	mCurrentTest.Failures.push_back(failure);
	mHadFailures = true;
}

void JSONTestOutput::DoOnSuiteStart(const String& suiteName)
{
	mCurrentSuite = JSONTestSuiteInfo();
	mCurrentSuite.Name = suiteName;
}

void JSONTestOutput::DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs)
{
	mCurrentSuite.TotalTestCount = totalTestCount;
	mCurrentSuite.PassedTestCount = passedTestCount;
	mCurrentSuite.FailedTestCount = failedTestCount;
	mCurrentSuite.DurationUs = durationUs;

	mSuites.push_back(mCurrentSuite);
}

void JSONTestOutput::DoOnTestStart(const String& testName)
{
	mCurrentTest = JSONTestInfo();
	mCurrentTest.Name = testName;
	mCurrentTestName = testName;
}

void JSONTestOutput::DoOnTestEnd(const String& testName, bool passed, u64 durationUs)
{
	mCurrentTest.Passed = passed;
	mCurrentTest.DurationUs = durationUs;

	mCurrentSuite.Tests.push_back(mCurrentTest);
}

void JSONTestOutput::DoOnOutputSuccess(const String& testName)
{
	// Success is already handled in DoOnTestEnd with passed=true
}
