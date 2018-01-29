//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Testing/BsConsoleTestOutput.h"
#include "Private/UnitTests/BsUtilityTestSuite.h"

using namespace bs;

int main()
{
	SPtr<TestSuite> tests = UtilityTestSuite::create<UtilityTestSuite>();

	ConsoleTestOutput testOutput;
	tests->run(testOutput);

	return 0;
}