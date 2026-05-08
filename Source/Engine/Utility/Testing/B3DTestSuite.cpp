//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
	output.DoOnSuiteStart(mSuiteName);

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
		output.DoOnTestStart(testEntry.Name);

		// Clear per-test state
		mCapturedLogs.clear();
		mActiveLogScopes.clear();

		// Track failures before test
		u32 failureCountBefore = mFailureCount;

		// Start timing this test
		Timer testTimer;

		// Execute test
		(this->*(testEntry.Test))();

		// Verify any remaining unhandled logs (if in Strict mode)
		if(logMode == LogMode::Strict)
			VerifyUnhandledLogs();

		// Assert that all scopes were properly cleaned up
		if(!mActiveLogScopes.empty())
		{
			mFailureCount++;
			mOutput->DoOnOutputFail("Test ended with active log scopes - ensure all LoggingScope objects go out of scope", testEntry.Name, __FILE__, __LINE__);
			mActiveLogScopes.clear();
		}

		// Calculate test duration
		u64 testDurationUs = testTimer.GetMicroseconds();

		// Check if test passed
		u32 failureCountAfter = mFailureCount;
		bool passed = (failureCountAfter == failureCountBefore);

		if(passed)
		{
			passedTests++;
			output.DoOnOutputSuccess(testEntry.Name);
		}
		else
			failedTests++;

		// Notify test end
		output.DoOnTestEnd(testEntry.Name, passed, testDurationUs);
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
	output.DoOnSuiteEnd(mSuiteName, totalTests, passedTests, failedTests, suiteDurationUs);

	// Run child suites recursively
	for(auto& suite : mSuites)
		suite->Run(output);
}

void TestSuite::Add(const TShared<TestSuite>& suite)
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
		mOutput->DoOnOutputFail(description, mActiveTestName, file, line);
	}
}

bool TestSuite::OnLogEntry(const String& message, LogVerbosity verbosity, const char* categoryName)
{
	bool shouldSuppress = false;

	// Check all active scopes (most recent first) for expected/ignored patterns
	for(auto* scope : mActiveLogScopes)
	{
		// Check expected logs
		for(auto& expected : scope->mExpected)
		{
			if(expected.Verbosity == verbosity && message.find(expected.Pattern) != String::npos)
			{
				expected.WasFound = true;
				shouldSuppress = true;
				break;
			}
		}

		if(shouldSuppress)
			break;

		// Check ignored logs
		for(const auto& ignored : scope->mIgnored)
		{
			if(ignored.Verbosity == verbosity && message.find(ignored.Pattern) != String::npos)
			{
				shouldSuppress = true;
				break;
			}
		}

		if(shouldSuppress)
			break;
	}

	// Only capture logs that weren't handled by any scope
	if(!shouldSuppress)
	{
		LogEntry entry;
		entry.Verbosity = verbosity;
		entry.Message = message;
		entry.CategoryName = categoryName;
		mCapturedLogs.push_back(entry);
	}

	return shouldSuppress;
}

void TestSuite::VerifyUnhandledLogs()
{
	// Check for any unhandled warnings/errors in mCapturedLogs
	// (logs that weren't matched by any active scope)
	for(const auto& log : mCapturedLogs)
	{
		if(log.Verbosity == LogVerbosity::Warning)
		{
			mFailureCount++;
			mOutput->DoOnOutputFail("Unexpected warning: " + log.Message, mActiveTestName, __FILE__, __LINE__);
		}
		else if(log.Verbosity <= LogVerbosity::Error)
		{
			mFailureCount++;
			mOutput->DoOnOutputFail("Unexpected error: " + log.Message, mActiveTestName, __FILE__, __LINE__);
		}
	}
}

void TestSuite::RegisterLogScope(LoggingScope* scope)
{
	mActiveLogScopes.insert(mActiveLogScopes.begin(), scope);
}

void TestSuite::UnregisterLogScope(LoggingScope* scope)
{
	auto iterator = std::find(mActiveLogScopes.begin(), mActiveLogScopes.end(), scope);
	if(iterator != mActiveLogScopes.end())
		mActiveLogScopes.erase(iterator);
}

// LoggingScope implementation

TestSuite::LoggingScope::LoggingScope(TestSuite& suite)
	: mSuite(suite)
{
	mSuite.RegisterLogScope(this);
}

TestSuite::LoggingScope::~LoggingScope()
{
	// Verify all expected logs were found
	for(const auto& expected : mExpected)
	{
		if(!expected.WasFound)
		{
			mSuite.mFailureCount++;
			mSuite.mOutput->DoOnOutputFail("Expected log not found: " + expected.Pattern, mSuite.mActiveTestName, __FILE__, __LINE__);
		}
	}

	mSuite.UnregisterLogScope(this);
}

void TestSuite::LoggingScope::ExpectLog(LogVerbosity verbosity, const String& pattern)
{
	ExpectedEntry entry;
	entry.Verbosity = verbosity;
	entry.Pattern = pattern;
	mExpected.push_back(entry);
}

void TestSuite::LoggingScope::ExpectWarning(const String& pattern)
{
	ExpectLog(LogVerbosity::Warning, pattern);
}

void TestSuite::LoggingScope::ExpectError(const String& pattern)
{
	ExpectLog(LogVerbosity::Error, pattern);
}

void TestSuite::LoggingScope::IgnoreLog(LogVerbosity verbosity, const String& pattern)
{
	IgnoredEntry entry;
	entry.Verbosity = verbosity;
	entry.Pattern = pattern;
	mIgnored.push_back(entry);
}

void TestSuite::LoggingScope::IgnoreWarning(const String& pattern)
{
	IgnoreLog(LogVerbosity::Warning, pattern);
}

void TestSuite::LoggingScope::IgnoreError(const String& pattern)
{
	IgnoreLog(LogVerbosity::Error, pattern);
}
