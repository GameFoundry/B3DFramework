//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuiteRegistry.h"

using namespace b3d;

void TestSuiteRegistry::RegisterSuite(const SPtr<TestSuite>& suite)
{
	mSuites.push_back(suite);
}
