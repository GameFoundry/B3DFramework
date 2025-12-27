//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DPixelData.h"
#include "Threading/B3DAsyncOp.h"
#include "Debug/B3DLog.h"
#include "Utility/B3DCommandLine.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

	/** Configuration for snapshot test mode, parsed from command-line arguments. */
	struct SnapshotTestConfiguration
	{
		bool Enabled = false;			/**< --enable-test-snapshot flag */
		Path OutputPath;				/**< --test-output-path=<path> */
		String TestName;				/**< --test-name=<name> */
		u64 CaptureFrame = 0;			/**< --capture-frame=N (0 = last frame - 2) */

		/**
		 * Returns the effective test name for output files.
		 * Uses TestName if specified, otherwise extracts the executable name.
		 */
		String GetEffectiveTestName() const
		{
			if(!TestName.empty())
				return TestName;

			// Extract executable name without extension
			const Path& executablePath = CommandLine::GetExecutablePath();
			const String& filename = executablePath.GetFilename(false);

			return filename.empty() ? "test" : filename;
		}

		/** Parse test mode configuration from command-line arguments. */
		static SnapshotTestConfiguration ParseFromCommandLine()
		{
			SnapshotTestConfiguration configuration;

			configuration.Enabled = CommandLine::HasParameter("enable-test-snapshot");
			configuration.OutputPath = Path(CommandLine::GetParameterValue("test-output-path", "./test_output"));
			configuration.TestName = CommandLine::GetParameterValue("test-name", "");
			configuration.CaptureFrame = (u64)CommandLine::GetParameterValueAsInt("capture-frame", 0);

			return configuration;
		}
	};

	/** Status of a snapshot test result. */
	enum class SnapshotTestStatus
	{
		Passed,
		Failed,
		PassedWithWarnings
	};

	/** Captured log entry for snapshot test output. */
	struct SnapshotTestLogEntry
	{
		String Message;
		LogVerbosity Verbosity;
		String Category;
	};

	/** Results of a snapshot test execution. */
	struct SnapshotTestResult
	{
		SnapshotTestStatus Status = SnapshotTestStatus::Passed;
		String TestName;
		Path ScreenshotPath;
		u64 TotalFrames = 0;
		float ExecutionTimeSeconds = 0.0f;
		Vector<String> Errors;
		Vector<String> Warnings;
	};

	/**
	 * Captures screenshots and collects logs from an application run.
	 *
	 * Integrates with the Application main loop to:
	 * - Capture screenshots at a specific frame
	 * - Collect log output during execution
	 * - Output results as JSON and log files
	 */
	class B3D_EXPORT SnapshotTestRunner
	{
	public:
		/**
		 * Creates and initializes a snapshot test runner.
		 *
		 * @param configuration	Configuration from command-line arguments.
		 */
		SnapshotTestRunner(const SnapshotTestConfiguration& configuration);
		~SnapshotTestRunner();

		/** Called each frame from Application::PostUpdate(). */
		void Update();

		/** Called from Application::EndMainLoop() to finalize results. */
		void Finalize();

	private:
		/** Request a screenshot capture from the main camera. */
		void RequestScreenCapture();

		/** Save the captured screenshot to disk. */
		bool SaveScreenshot(const SPtr<PixelData>& pixelData);

		/** Write the result JSON file. */
		bool WriteResultJson();

		/** Write the log file. */
		bool WriteLogFile();

		/**
		 * Log callback to capture log entries.
		 * @return False to allow normal log processing to continue.
		 */
		bool OnLogEntry(const String& message, LogVerbosity verbosity);

		SnapshotTestConfiguration mConfiguration;
		SnapshotTestResult mResult;
		TAsyncOp<SPtr<PixelData>> mScreenCaptureOp;

		u64 mStartFrame = 0;
		u64 mExitAfterNFrames = 0;
		u64 mStartTimeUs = 0;
		bool mCaptureRequested = false;
		bool mFinalized = false;
	};

	/** @} */
} // namespace b3d
