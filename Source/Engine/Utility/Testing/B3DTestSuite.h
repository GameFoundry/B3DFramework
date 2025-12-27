//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	/** @addtogroup Testing
	 *  @{
	 */

/** Tests if condition is true, and reports unit test failure if it fails. */
#define B3D_TEST_ASSERT(expr) Assertment((expr), __FUNCTION__, __FILE__, __LINE__);

/** Tests if condition is true, and reports unit test failure with a message if it fails. */
#define B3D_TEST_ASSERT_MSG(expr, msg) Assertment((expr), msg, __FILE__, __LINE__);

/** Tests if condition is true, and reports unit test failure if it fails. Expects a reference to the TestSuite. */
#define B3D_TEST_ASSERT_EXTERNAL(TestSuite, Expr) TestSuite.Assertment((Expr), __FUNCTION__, __FILE__, __LINE__);

	/**
	 * Primary class for unit testing. Override and register unit tests in constructor then run the tests using the
	 * desired method of output.
	 */
	class B3D_EXPORT TestSuite
	{
	public:
		typedef void (TestSuite::*Func)();

		/** Controls how logs are handled during test execution. */
		enum class LogMode
		{
			/** Capture and verify logs, fail on unexpected warnings/errors. */
			Strict,

			/** Capture logs but don't fail on warnings/errors. */
			Permissive,

			/** Disable log capture entirely. */
			Disabled
		};

	private:
		/** Contains data about a single unit test. */
		struct TestEntry
		{
			TestEntry(Func test, const String& name);

			Func Test;
			String Name;
		};

		/** Information about an ignored log entry (may or may not occur). */
		struct IgnoredLogEntry
		{
			LogVerbosity Verbosity;
			String Pattern;
			bool WasMatched = false;
		};

		/** Information about an expected log entry (MUST occur). */
		struct ExpectedLogEntry
		{
			LogVerbosity Verbosity;
			String Pattern;
			bool WasFound = false;
		};

		/** Log callback handler. */
		bool OnLogEntry(const String& message, LogVerbosity verbosity, const char* categoryName);

		/** Verify logs at end of test. */
		void VerifyLogs();

	public:
		virtual ~TestSuite() = default;

		/** Runs all the tests in the suite (and sub-suites). Tests results are reported to the provided output class. */
		void Run(TestOutput& output);

		/** Adds a new child suite to this suite. This method allows you to group suites and execute them all at once. */
		void Add(const SPtr<TestSuite>& suite);

		/**	Creates a new suite of a particular type. */
		template <class T>
		static SPtr<TestSuite> Create()
		{
			static_assert((std::is_base_of<TestSuite, T>::value), "Invalid test suite type. It needs to derive from b3d::TestSuite.");

			return std::static_pointer_cast<TestSuite>(B3DMakeShared<T>());
		}

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Reports success or failure depending on the result of an expression.
		 *
		 * @param	success	If true success is reported, otherwise failure.
		 * @param	desc	Message describing the nature of the failure.
		 * @param	file	Name of the source code file the assert originates from.
		 * @param	line	Line number at which the assert was triggered at.
		 */
		void Assertment(bool success, const String& desc, const String& file, long line);

		/** @} */

	protected:
		TestSuite(const String& name = "TestSuite")
			: mSuiteName(name) {}

		/** Called right before any tests are ran. */
		virtual void StartUp() {}

		/**	Called after all tests and child suite's tests are ran. */
		virtual void ShutDown() {}

		/**
		 * Override to specify log handling mode for this suite.
		 * Default is Strict (fail on unexpected warnings/errors).
		 */
		virtual LogMode GetLogMode() const { return LogMode::Strict; }

		/**
		 * Declare that a specific warning/error may occur and should not fail the test.
		 * Test passes whether or not the log actually occurs.
		 *
		 * @param	verbosity	Log level to ignore (Warning, Error, etc.)
		 * @param	pattern		Substring to match in the log message.
		 */
		void IgnoreLog(LogVerbosity verbosity, const String& pattern);

		/** Convenience for ignoring warnings. */
		void IgnoreWarning(const String& pattern);

		/** Convenience for ignoring errors. */
		void IgnoreError(const String& pattern);

		/**
		 * Declare that a warning/error is expected and MUST occur.
		 * Test fails if the expected log is not found.
		 *
		 * @param	verbosity	Log level that must occur (Warning, Error, etc.)
		 * @param	pattern		Substring to match in the log message.
		 */
		void ExpectLog(LogVerbosity verbosity, const String& pattern);

		/** Convenience for expecting warnings. */
		void ExpectWarning(const String& pattern);

		/** Convenience for expecting errors. */
		void ExpectError(const String& pattern);

		/**
		 * Register a new unit test.
		 *
		 * @param	test	Function to call in order to execute the test.
		 * @param	name	Name of the test we can use for referencing it later.
		 */
		void AddTest(Func test, const String& name);

	private:
		String mSuiteName;
		Vector<TestEntry> mTests;
		Vector<SPtr<TestSuite>> mSuites;

		// Transient
		TestOutput* mOutput = nullptr;
		String mActiveTestName;
		u32 mFailureCount = 0;

		Vector<LogEntry> mCapturedLogs; /**< Logs captured during current test. */
		Vector<IgnoredLogEntry> mIgnoredLogs; /**< Logs that may occur and should be ignored. */
		Vector<ExpectedLogEntry> mExpectedLogs; /**< Logs that MUST occur for test to pass. */

		bool mLogCaptureActive = false; /**< True if log capture is currently active. */
	};

/** Registers a new unit test within an implementation of TestSuite. */
#define B3D_ADD_TEST(func) AddTest(static_cast<Func>(&func), #func);

	/** @} */
} // namespace b3d
