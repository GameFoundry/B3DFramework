//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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

	public:
		/**
		 * RAII scope for managing expected/ignored logs during a test section.
		 * When the scope exits, expected logs are verified and all registrations are cleaned up.
		 */
		class B3D_EXPORT LoggingScope
		{
		public:
			LoggingScope(TestSuite& suite);
			~LoggingScope();

			// Non-copyable, non-movable
			LoggingScope(const LoggingScope&) = delete;
			LoggingScope& operator=(const LoggingScope&) = delete;

			/**
			 * Declare that a log with given verbosity and pattern MUST occur.
			 * Test fails if the expected log is not found when scope exits.
			 */
			void ExpectLog(LogVerbosity verbosity, const String& pattern);

			/** Convenience for expecting warnings, see ExpectLog. */
			void ExpectWarning(const String& pattern);

			/** Convenience for expecting errors, see ExpectLog. */
			void ExpectError(const String& pattern);

			/**
			 * Declare that a log with given verbosity and pattern MAY occur.
			 * Test passes whether or not the log actually occurs.
			 */
			void IgnoreLog(LogVerbosity verbosity, const String& pattern);

			/** Convenience for ignoring warnings, see IgnoreLog. */
			void IgnoreWarning(const String& pattern);

			/** Convenience for ignoring errors, see IgnoreLog. */
			void IgnoreError(const String& pattern);

		private:
			friend class TestSuite;

			struct ExpectedEntry
			{
				LogVerbosity Verbosity;
				String Pattern;
				bool WasFound = false;
			};

			struct IgnoredEntry
			{
				LogVerbosity Verbosity;
				String Pattern;
			};

			TestSuite& mSuite;
			Vector<ExpectedEntry> mExpected;
			Vector<IgnoredEntry> mIgnored;
		};

	private:
		/** Contains data about a single unit test. */
		struct TestEntry
		{
			TestEntry(Func test, const String& name);

			Func Test;
			String Name;
		};

		/** Log callback handler. */
		bool OnLogEntry(const String& message, LogVerbosity verbosity, const char* categoryName);

		/** Verify any unhandled warnings/errors at end of test. */
		void VerifyUnhandledLogs();

		/** Register/unregister log scopes (called by LoggingScope). */
		void RegisterLogScope(LoggingScope* scope);
		void UnregisterLogScope(LoggingScope* scope);

		friend class LoggingScope;

	public:
		virtual ~TestSuite() = default;

		/** Runs all the tests in the suite (and sub-suites). Tests results are reported to the provided output class. */
		void Run(TestOutput& output);

		/** Adds a new child suite to this suite. This method allows you to group suites and execute them all at once. */
		void Add(const TShared<TestSuite>& suite);

		/**	Creates a new suite of a particular type. */
		template <class T>
		static TShared<TestSuite> Create()
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
		 * Register a new unit test.
		 *
		 * @param	test	Function to call in order to execute the test.
		 * @param	name	Name of the test we can use for referencing it later.
		 */
		void AddTest(Func test, const String& name);

	private:
		String mSuiteName;
		Vector<TestEntry> mTests;
		Vector<TShared<TestSuite>> mSuites;

		// Transient
		TestOutput* mOutput = nullptr;
		String mActiveTestName;
		u32 mFailureCount = 0;

		Vector<LogEntry> mCapturedLogs; /**< Logs captured during current test. */
		Vector<LoggingScope*> mActiveLogScopes; /**< Active log scopes (stack - most recent first). */

		bool mLogCaptureActive = false; /**< True if log capture is currently active. */
	};

/** Registers a new unit test within an implementation of TestSuite. */
#define B3D_ADD_TEST(func) AddTest(static_cast<Func>(&func), #func);

	/** @} */
} // namespace b3d
