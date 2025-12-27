//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestOutput.h"
#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Outputs unit test results to stdout with detailed statistics and colored output. */
	class B3D_EXPORT ConsoleTestOutput : public TestOutput
	{
	public:
		ConsoleTestOutput();
		~ConsoleTestOutput() override;

		void DoOnOutputFail(const String& description, const String& function, const String& file, long line) override;
		void DoOnOutputSuccess(const String& testName) override;

		void DoOnSuiteStart(const String& suiteName) override;
		void DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs) override;
		void DoOnTestStart(const String& testName) override;
		void DoOnTestEnd(const String& testName, bool passed, u64 durationUs) override;

		/** Returns 0 if all tests passed, 1 if any failed. */
		int GetExitCode() const { return mHadFailures ? 1 : 0; }

	private:
		/** Console color codes for colored output. */
		enum class ConsoleColor
		{
			Default,
			Green,
			Red,
			Yellow,
			Cyan
		};

		/** Information about a single test failure. */
		struct FailureInfo
		{
			String TestName;
			String Description;
			String File;
			long Line;
		};

		/** Print text to console with specified color. */
		void PrintColored(const String& text, ConsoleColor color);

		/** Print separator line (========). */
		void PrintSeparator();

		/** Set console text color. */
		void SetConsoleColor(ConsoleColor color);

		/** Reset console to default color. */
		void ResetConsoleColor();

		Vector<FailureInfo> mFailures; /**< List of all failures encountered during test execution. */
		bool mHadFailures = false; /**< True if any test has failed. */

		String mCurrentTest; /**< Name of the currently executing test. */
		u32 mCurrentTestIndex = 0; /**< Index of current test within the suite. */

		u32 mTotalTestsInSuite = 0; /**< Total number of tests in current suite. */
		bool mSupportsColor = true; /** True if console supports colored output. */

#if B3D_PLATFORM_WIN32
		/** Windows console handle (HANDLE). */
		void* mConsoleHandle = nullptr;

		/** Default console text attributes. */
		u16 mDefaultAttributes = 0;
#endif
	};

	/** @} */
} // namespace b3d
