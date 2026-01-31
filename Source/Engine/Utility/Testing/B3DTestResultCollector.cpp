//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestResultCollector.h"

namespace b3d
{
	void TestResultCollector::DoOnSuiteStart(const String& suiteName)
	{
		mCurrentSuite = TestSuiteResult();
		mCurrentSuite.Name = suiteName;
	}

	void TestResultCollector::DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs)
	{
		mCurrentSuite.TotalTestCount = totalTestCount;
		mCurrentSuite.PassedTestCount = passedTestCount;
		mCurrentSuite.FailedTestCount = failedTestCount;
		mCurrentSuite.DurationUs = durationUs;

		mResults.push_back(mCurrentSuite);
	}

	void TestResultCollector::DoOnTestStart(const String& testName)
	{
		mCurrentTest = TestResult();
		mCurrentTest.Name = testName;
		mCurrentTestName = testName;
	}

	void TestResultCollector::DoOnTestEnd(const String& testName, bool passed, u64 durationUs)
	{
		mCurrentTest.Passed = passed;
		mCurrentTest.DurationUs = durationUs;

		mCurrentSuite.Tests.push_back(mCurrentTest);
	}

	void TestResultCollector::DoOnOutputFail(const String& description, const String& function, const String& file, long line)
	{
		TestFailureInfo failure;
		failure.TestName = mCurrentTestName;
		failure.Description = description;
		failure.File = file;
		failure.Line = line;

		mCurrentTest.Failures.push_back(failure);
		mHadFailures = true;
	}

	void TestResultCollector::DoOnOutputSuccess(const String& testName)
	{
		// Success is already handled in DoOnTestEnd with passed=true
	}
} // namespace b3d
