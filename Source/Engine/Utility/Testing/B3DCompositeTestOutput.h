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

	/**
	 * Broadcasts test events to multiple TestOutput instances. Can be used to output test results both to console
	 * and log it in a file for example.
	 */
	class CompositeTestOutput : public TestOutput
	{
	public:
		/** Adds an output to receive test events. Output must remain valid for the lifetime of this object. */
		void Add(TestOutput& output)
		{
			mOutputs.push_back(&output);
		}

		void DoOnSuiteStart(const String& suiteName) override
		{
			for (auto* output : mOutputs)
				output->DoOnSuiteStart(suiteName);
		}

		void DoOnSuiteEnd(const String& suiteName, u32 totalTestCount, u32 passedTestCount, u32 failedTestCount, u64 durationUs) override
		{
			for (auto* output : mOutputs)
				output->DoOnSuiteEnd(suiteName, totalTestCount, passedTestCount, failedTestCount, durationUs);
		}

		void DoOnTestStart(const String& testName) override
		{
			for (auto* output : mOutputs)
				output->DoOnTestStart(testName);
		}

		void DoOnTestEnd(const String& testName, bool passed, u64 durationUs) override
		{
			for (auto* output : mOutputs)
				output->DoOnTestEnd(testName, passed, durationUs);
		}

		void DoOnOutputFail(const String& description, const String& function,
			const String& file, long line) override
		{
			for (auto* output : mOutputs)
				output->DoOnOutputFail(description, function, file, line);
		}

		void DoOnOutputSuccess(const String& testName) override
		{
			for (auto* output : mOutputs)
				output->DoOnOutputSuccess(testName);
		}

	private:
		Vector<TestOutput*> mOutputs;
	};

	/** @} */
} // namespace b3d
