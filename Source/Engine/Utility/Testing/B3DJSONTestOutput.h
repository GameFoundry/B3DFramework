//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Testing/B3DTestOutput.h"
#include "FileSystem/B3DPath.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Information about a single test failure for JSON output. */
	struct JSONTestFailureInfo
	{
		String TestName;
		String Description;
		String File;
		long Line;
	};

	/** Information about a single test for JSON output. */
	struct JSONTestInfo
	{
		String Name;
		bool Passed = true;
		u64 DurationUs = 0;
		Vector<JSONTestFailureInfo> Failures;
	};

	/** Information about a test suite for JSON output. */
	struct JSONTestSuiteInfo
	{
		String Name;
		u32 TotalTestCount = 0;
		u32 PassedTestCount = 0;
		u32 FailedTestCount = 0;
		u64 DurationUs = 0;
		Vector<JSONTestInfo> Tests;
	};

	/** Outputs unit test results to a JSON file for structured reporting. */
	class B3D_EXPORT JSONTestOutput : public TestOutput
	{
	public:
		/**
		 * Creates a JSON test output that will write results to the specified file.
		 *
		 * @param	outputPath	Path to the output JSON file.
		 */
		JSONTestOutput(const Path& outputPath);
		~JSONTestOutput();

		void DoOnSuiteStart(const String& suiteName) override;
		void DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs) override;
		void DoOnTestStart(const String& testName) override;
		void DoOnTestEnd(const String& testName, bool passed, u64 durationUs) override;
		void DoOnOutputFail(const String& description, const String& function, const String& file, long line) override;
		void DoOnOutputSuccess(const String& testName) override;

		/** Returns 0 if all tests passed, 1 if any failed. */
		i32 GetExitCode() const { return mHadFailures ? 1 : 0; }

	private:
		Path mOutputPath;
		Vector<JSONTestSuiteInfo> mSuites;
		JSONTestSuiteInfo mCurrentSuite;
		JSONTestInfo mCurrentTest;
		String mCurrentTestName;
		bool mHadFailures = false;
	};

	/** @} */
} // namespace b3d
