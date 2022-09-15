//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsTestSuite.h"
#include "Testing/BsTestOutput.h"

namespace bs
{
	TestSuite::TestEntry::TestEntry(Func test, const String& name)
		:test(test), name(name)
	{ }

	void TestSuite::Run(TestOutput& output)
	{
		mOutput = &output;

		StartUp();

		for (auto& testEntry : mTests)
		{
			mActiveTestName = testEntry.name;
			
			(this->*(testEntry.test))();
		}

		for (auto& suite : mSuites)
		{
			suite->Run(output);
		}

		ShutDown();
	}

	void TestSuite::Add(const SPtr<TestSuite>& suite)
	{
		mSuites.push_back(suite);
	}

	void TestSuite::AddTest(Func test, const String& name)
	{
		mTests.push_back(TestEntry(test, name));
	}

	void TestSuite::Assertment(bool success, const String& desc, const String& file, long line)
	{
		if (!success)
			mOutput->OutputFail(desc, mActiveTestName, file, line);
	}
}
