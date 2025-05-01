//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"

namespace bs
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
	class B3D_UTILITY_EXPORT TestSuite
	{
	public:
		typedef void (TestSuite::*Func)();

	private:
		/** Contains data about a single unit test. */
		struct TestEntry
		{
			TestEntry(Func test, const String& name);

			Func Test;
			String Name;
		};

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
			static_assert((std::is_base_of<TestSuite, T>::value), "Invalid test suite type. It needs to derive from bs::TestSuite.");

			return std::static_pointer_cast<TestSuite>(B3DMakeShared<T>());
		}

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Reports success or failure depending on the result of an expression.
		 *
		 * @param[in]	success		If true success is reported, otherwise failure.
		 * @param[in]	desc		Message describing the nature of the failure.
		 * @param[in]	file		Name of the source code file the assert originates from.
		 * @param[in]	line		Line number at which the assert was triggered at.
		 */
		void Assertment(bool success, const String& desc, const String& file, long line);

		/** @} */

	protected:
		TestSuite() = default;

		/** Called right before any tests are ran. */
		virtual void StartUp() {}

		/**	Called after all tests and child suite's tests are ran. */
		virtual void ShutDown() {}

		/**
		 * Register a new unit test.
		 *
		 * @param[in]	test	Function to call in order to execute the test.
		 * @param[in]	name	Name of the test we can use for referencing it later.
		 */
		void AddTest(Func test, const String& name);

		Vector<TestEntry> mTests;
		Vector<SPtr<TestSuite>> mSuites;

		// Transient
		TestOutput* mOutput = nullptr;
		String mActiveTestName;
	};

/** Registers a new unit test within an implementation of TestSuite. */
#define B3D_ADD_TEST(func) AddTest(static_cast<Func>(&func), #func);

	/** @} */
} // namespace bs
