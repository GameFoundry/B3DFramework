//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuite.h"
#include "Testing/B3DTestOutput.h"
#include "Utility/B3DTimer.h"
#include "Debug/B3DDebug.h"
#include "Debug/B3DLog.h"

using namespace b3d;

TestSuite::TestEntry::TestEntry(Func test, const String& name)
	: Test(test), Name(name)
{}

void TestSuite::Run(TestOutput& output)
{
	mOutput = &output;

	// Initialize log capture based on mode
	LogMode logMode = GetLogMode();
	if(logMode != LogMode::Disabled)
	{
		// Install log callback
		auto fnCallback = [this](const String& msg, LogVerbosity verb, const char* cat) -> bool {
			return OnLogEntry(msg, verb, cat);
		};
		GetDebug().SetLogCallback(fnCallback);
		mLogCaptureActive = true;
	}

	// Notify suite start
	output.OnSuiteStart(mSuiteName);

	// Start timing
	Timer suiteTimer;
	u32 totalTests = static_cast<u32>(mTests.size());
	u32 passedTests = 0;
	u32 failedTests = 0;

	StartUp();

	// Run each test
	for(auto& testEntry : mTests)
	{
		mActiveTestName = testEntry.Name;

		// Notify test start
		output.OnTestStart(testEntry.Name);

		// Clear per-test state
		mCapturedLogs.clear();
		mIgnoredLogs.clear();
		mExpectedLogs.clear();

		// Track failures before test
		u32 failureCountBefore = mFailureCount;

		// Start timing this test
		Timer testTimer;

		// Execute test
		(this->*(testEntry.Test))();

		// Verify logs after test (if in Strict mode)
		if(logMode == LogMode::Strict)
			VerifyLogs();

		// Calculate test duration
		u64 testDurationUs = testTimer.GetMicroseconds();

		// Check if test passed
		u32 failureCountAfter = mFailureCount;
		bool passed = (failureCountAfter == failureCountBefore);

		if(passed)
		{
			passedTests++;
			output.OutputSuccess(testEntry.Name);
		}
		else
			failedTests++;

		// Notify test end
		output.OnTestEnd(testEntry.Name, passed, testDurationUs);
	}

	ShutDown();

	// Disable log callback
	if(mLogCaptureActive)
	{
		GetDebug().SetLogCallback(nullptr);
		mLogCaptureActive = false;
	}

	// Calculate total duration
	u64 suiteDurationUs = suiteTimer.GetMicroseconds();

	// Notify suite end
	output.OnSuiteEnd(mSuiteName, totalTests, passedTests, failedTests, suiteDurationUs);

	// Run child suites recursively
	for(auto& suite : mSuites)
		suite->Run(output);
}

void TestSuite::Add(const SPtr<TestSuite>& suite)
{
	mSuites.push_back(suite);
}

void TestSuite::AddTest(Func test, const String& name)
{
	mTests.push_back(TestEntry(test, name));
}

void TestSuite::Assertment(bool success, const String& description, const String& file, long line)
{
	if(!success)
	{
		mFailureCount++;
		mOutput->OutputFail(description, mActiveTestName, file, line);
	}
}

bool TestSuite::OnLogEntry(const String& message, LogVerbosity verbosity, const char* categoryName)
{
	// Capture the log
	LogEntry entry;
	entry.Verbosity = verbosity;
	entry.Message = message;
	entry.CategoryName = categoryName;
	mCapturedLogs.push_back(entry);

	bool shouldSuppress = false;

	// Check if this log is EXPECTED (must occur)
	for(auto& expected : mExpectedLogs)
	{
		if(expected.Verbosity == verbosity && message.find(expected.Pattern) != String::npos)
		{
			expected.WasFound = true;
			shouldSuppress = true;
			break;
		}
	}

	// Check if this log is IGNORED (optional)
	if(!shouldSuppress)
	{
		for(auto& ignored : mIgnoredLogs)
		{
			if(ignored.Verbosity == verbosity && message.find(ignored.Pattern) != String::npos)
			{
				ignored.WasMatched = true;
				shouldSuppress = true;
				break;
			}
		}
	}

	return shouldSuppress;
}

void TestSuite::VerifyLogs()
{
	LogMode mode = GetLogMode();
	if(mode != LogMode::Strict)
		return;

	// 1. Check that all EXPECTED logs were found
	for(const auto& expected : mExpectedLogs)
	{
		if(!expected.WasFound)
		{
			mFailureCount++;
			mOutput->OutputFail(
				"Expected log not found: " + expected.Pattern,
				mActiveTestName,
				__FILE__,
				__LINE__
			);
		}
	}

	// 2. Check for UNEXPECTED warnings/errors
	for(const auto& log : mCapturedLogs)
	{
		// Check if this log was expected or ignored
		bool wasHandled = false;

		// Check expected list
		for(const auto& expected : mExpectedLogs)
		{
			if(expected.Verbosity == log.Verbosity &&
			   log.Message.find(expected.Pattern) != String::npos)
			{
				wasHandled = true;
				break;
			}
		}

		// Check ignored list
		if(!wasHandled)
		{
			for(const auto& ignored : mIgnoredLogs)
			{
				if(ignored.Verbosity == log.Verbosity &&
				   log.Message.find(ignored.Pattern) != String::npos)
				{
					wasHandled = true;
					break;
				}
			}
		}

		// If not handled, it's unexpected
		if(!wasHandled)
		{
			if(log.Verbosity == LogVerbosity::Warning)
			{
				mFailureCount++;
				mOutput->OutputFail(
					"Unexpected warning: " + log.Message,
					mActiveTestName,
					__FILE__,
					__LINE__
				);
			}
			else if(log.Verbosity <= LogVerbosity::Error)
			{
				mFailureCount++;
				mOutput->OutputFail(
					"Unexpected error: " + log.Message,
					mActiveTestName,
					__FILE__,
					__LINE__
				);
			}
		}
	}
}

void TestSuite::IgnoreLog(LogVerbosity verbosity, const String& pattern)
{
	IgnoredLogEntry entry;
	entry.Verbosity = verbosity;
	entry.Pattern = pattern;
	mIgnoredLogs.push_back(entry);
}

void TestSuite::IgnoreWarning(const String& pattern)
{
	IgnoreLog(LogVerbosity::Warning, pattern);
}

void TestSuite::IgnoreError(const String& pattern)
{
	IgnoreLog(LogVerbosity::Error, pattern);
}

void TestSuite::ExpectLog(LogVerbosity verbosity, const String& pattern)
{
	ExpectedLogEntry entry;
	entry.Verbosity = verbosity;
	entry.Pattern = pattern;
	mExpectedLogs.push_back(entry);
}

void TestSuite::ExpectWarning(const String& pattern)
{
	ExpectLog(LogVerbosity::Warning, pattern);
}

void TestSuite::ExpectError(const String& pattern)
{
	ExpectLog(LogVerbosity::Error, pattern);
}
