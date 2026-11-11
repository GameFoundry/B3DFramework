//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for acceleration structures and ray tracing. On devices without the capabilities, the tests verify the errors instead. */
	class GpuRayTracingTestSuite : public TestSuite
	{
	public:
		GpuRayTracingTestSuite();

	private:
		/**
		 * Verifies acceleration structure buffers: the creation rules, the resting state, barriers in and out of an acceleration structure
		 * build, and the rejection of host access, storage buffer bindings and stages that cannot access them. Without
		 * RSC_ACCELERATION_STRUCTURES, verifies that creation fails.
		 */
		void TestAccelerationStructureBuffers();
	};
} // namespace b3d
