//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"



namespace BansheeEngine
{
	/** @addtogroup Testing
	 *  @{
	 */

/** Tests if condition is true, and reports unit test failure if it fails. */
#define BS_TEST_ASSERT(expr) assertment((expr), __FUNCTION__, __FILE__, __LINE__); 

/** Tests if condition is true, and reports unit test failure with a message if it fails. */
#define BS_TEST_ASSERT_MSG(expr, msg) assertment((expr), msg, __FILE__, __LINE__); 

	/**
	 * Primary class for unit testing. Override and register unit tests in constructor then run the tests using the 
	 * desired method of output.
	 */
	class BS_UTILITY_EXPORT TestSuite
	{
	public:
		typedef void(TestSuite::*Func)();

	private:
		/** Contains data about a single unit test. */
		struct TestEntry
		{
			TestEntry(Func test, const String& name);

			Func test;
			String name;
		};

	public:
		virtual ~TestSuite() {}

		/** Runs all the tests in the suite (and sub-suites). Tests results are reported to the provided output class. */
		void run(TestOutput& output);

		/** Adds a new child suite to this suite. This method allows you to group suites and execute them all at once. */
		void add(const SPtr<TestSuite>& suite);

		/**	Creates a new suite of a particular type. */
		template <class T>
		static SPtr<TestSuite> create()
		{
			static_assert((std::is_base_of<TestSuite, T>::value), "Invalid test suite type. It needs to derive from BansheeEngine::TestSuite.");

			return std::static_pointer_cast<TestSuite>(bs_shared_ptr_new<T>());
		}

	protected:
		TestSuite();

		/** Called right before any tests are ran. */
		virtual void startUp() {}

		/**	Called after all tests and child suite's tests are ran. */
		virtual void shutDown() {}

		/**
		 * Register a new unit test.
		 *
		 * @param[in]	test	Function to call in order to execute the test.
		 * @param[in]	name	Name of the test we can use for referencing it later.
		 */
		void addTest(Func test, const String& name);

		/**
		 * Reports success or failure depending on the result of an expression.
		 *
		 * @param[in]	success		If true success is reported, otherwise failure.
		 * @param[in]	desc		Message describing the nature of the failure.
		 * @param[in]	file		Name of the source code file the assertment originates from.
		 * @param[in]	line		Line number at which the assertment was triggered at.
		 */
		void assertment(bool success, const String& desc, const String& file, long line);

		Vector<TestEntry> mTests;
		Vector<SPtr<TestSuite>> mSuites;

		// Transient
		TestOutput* mOutput;
		String mActiveTestName;
	};

/** Registers a new unit test within an implementation of TestSuite. */
#define BS_ADD_TEST(func) addTest(static_cast<Func>(&func), #func);

	/** @} */
}
