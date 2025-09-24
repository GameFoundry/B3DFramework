//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DConsoleTestOutput.h"
#include "B3DUtilityTestSuite.h"

using namespace b3d;

int main()
{
	SPtr<TestSuite> tests = UtilityTestSuite::Create<UtilityTestSuite>();

	ConsoleTestOutput testOutput;
	tests->Run(testOutput);

	return 0;
}
