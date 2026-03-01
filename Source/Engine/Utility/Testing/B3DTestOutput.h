//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Abstract interface used for outputting unit test results. */
	class B3D_EXPORT TestOutput
	{
	public:
		virtual ~TestOutput() {}

		/**
		 * Triggered when a unit test fails.
		 *
		 * @param	description	Reason why the unit test failed.
		 * @param	function	Name of the function the test failed in.
		 * @param	file		File the unit test failed in.
		 * @param	line		Line of code the unit test failed on.
		 */
		virtual void DoOnOutputFail(const String& description, const String& function, const String& file, long line) = 0;

		/**
		 * Triggered when a unit test succeeds (optional to implement).
		 *
		 * @param	testName	Name of the test that succeeded.
		 */
		virtual void DoOnOutputSuccess(const String& testName) {}

		/**
		 * Triggered when a test suite starts executing.
		 *
		 * @param	suiteName	Name of the test suite being executed.
		 */
		virtual void DoOnSuiteStart(const String& suiteName) {}

		/**
		 * Triggered when a test suite finishes executing.
		 *
		 * @param	suiteName			Name of the test suite that finished.
		 * @param	totalTestCount		Total number of tests in the suite.
		 * @param	passedTestCount		Number of tests that passed.
		 * @param	failedTestCount		Number of tests that failed.
		 * @param	durationUs			Total execution time in microseconds.
		 */
		virtual void DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs) {}

		/**
		 * Triggered when an individual test starts executing.
		 *
		 * @param	testName	Name of the test being executed.
		 */
		virtual void DoOnTestStart(const String& testName) {}

		/**
		 * Triggered when an individual test finishes executing.
		 *
		 * @param	testName	Name of the test that finished.
		 * @param	passed		True if the test passed, false if it failed.
		 * @param	durationUs	Execution time in microseconds.
		 */
		virtual void DoOnTestEnd(const String& testName, bool passed, u64 durationUs) {}
	};

	/** @} */
} // namespace b3d
