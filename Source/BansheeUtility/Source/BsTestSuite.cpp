//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTestSuite.h"
#include "BsTestOutput.h"

namespace BansheeEngine
{
	TestSuite::TestEntry::TestEntry(Func test, const String& name)
		:test(test), name(name)
	{ }

	TestSuite::TestSuite()
		: mOutput(nullptr)
	{ }

	void TestSuite::run(TestOutput& output)
	{
		mOutput = &output;

		startUp();

		for (auto& testEntry : mTests)
		{
			mActiveTestName = testEntry.name;
			
			(this->*(testEntry.test))();
		}

		for (auto& suite : mSuites)
		{
			suite->run(output);
		}

		shutDown();
	}

	void TestSuite::add(const SPtr<TestSuite>& suite)
	{
		mSuites.push_back(suite);
	}

	void TestSuite::addTest(Func test, const String& name)
	{
		mTests.push_back(TestEntry(test, name));
	}

	void TestSuite::assertment(bool success, const String& desc, const String& file, long line)
	{
		if (!success)
			mOutput->outputFail(desc, mActiveTestName, file, line);
	}
}
