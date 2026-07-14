//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanAllocatorTestSuite.h"
#include "B3DVulkanBarrierTestSuite.h"
#include "Testing/B3DTestSuiteRegistry.h"
#include "Prerequisites/B3DPlatformDefines.h"

/** Plugin test DLL entry point that's located by the unit test runner. */
extern "C" B3D_PLUGIN_EXPORT void RegisterTestSuites()
{
	b3d::TestSuiteRegistry::Instance().RegisterSuite(b3d::TestSuite::Create<b3d::VulkanAllocatorTestSuite>());
	b3d::TestSuiteRegistry::Instance().RegisterSuite(b3d::TestSuite::Create<b3d::VulkanBarrierTestSuite>());
}
