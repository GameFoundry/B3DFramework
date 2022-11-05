//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "BsUtilityTestSuite.h"

using namespace bs;

int main()
{
	SPtr<TestSuite> tests = UtilityTestSuite::Create<UtilityTestSuite>();

	ConsoleTestOutput testOutput;
	tests->Run(testOutput);

	return 0;
}
