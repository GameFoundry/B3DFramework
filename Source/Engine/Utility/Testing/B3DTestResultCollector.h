//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Testing/B3DTestOutput.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Information about a single test failure. */
	struct TestFailureInfo
	{
		String TestName;
		String Description;
		String File;
		long Line;
	};

	/** Information about a single test result. */
	struct TestResult
	{
		String Name;
		bool Passed = true;
		u64 DurationUs = 0;
		Vector<TestFailureInfo> Failures;
	};

	/** Information about a test suite result. */
	struct TestSuiteResult
	{
		String Name;
		u32 TotalTestCount = 0;
		u32 PassedTestCount = 0;
		u32 FailedTestCount = 0;
		u64 DurationUs = 0;
		Vector<TestResult> Tests;
	};

	/**
	 * Pure result accumulation without I/O. Implements TestOutput to collect test results
	 * that can later be written to various formats.
	 */
	class B3D_EXPORT TestResultCollector : public TestOutput
	{
	public:
		void DoOnSuiteStart(const String& suiteName) override;
		void DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount,
			u32 failedTestCount, u64 durationUs) override;
		void DoOnTestStart(const String& testName) override;
		void DoOnTestEnd(const String& testName, bool passed, u64 durationUs) override;
		void DoOnOutputFail(const String& description, const String& function,
			const String& file, long line) override;
		void DoOnOutputSuccess(const String& testName) override;

		/** Returns all collected test suite results. */
		const Vector<TestSuiteResult>& GetResults() const { return mResults; }

		/** Returns true if any test has failed. */
		bool HasFailures() const { return mHadFailures; }

		/** Returns 0 if all tests passed, 1 if any failed. */
		i32 GetExitCode() const { return mHadFailures ? 1 : 0; }

	private:
		Vector<TestSuiteResult> mResults;
		TestSuiteResult mCurrentSuite;
		TestResult mCurrentTest;
		String mCurrentTestName;
		bool mHadFailures = false;
	};

	/** @} */
} // namespace b3d
