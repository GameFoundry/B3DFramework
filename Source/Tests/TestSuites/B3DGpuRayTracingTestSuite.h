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
		 * Verifies acceleration structure creation: the creation errors, the type, size and bindless slot, the resting state, barriers in
		 * and out of an acceleration structure build, the stages that cannot access structures or buffers, and the rules for build input
		 * buffers. Without RSC_ACCELERATION_STRUCTURES, verifies that creation fails.
		 */
		void TestAccelerationStructureCreation();

		/**
		 * Verifies the size query and the instance writes, and the errors of invalid inputs, instances and builds. Without
		 * RSC_ACCELERATION_STRUCTURES, verifies that the queries, instance writes and builds fail.
		 */
		void TestAccelerationStructureQueries();

		/**
		 * Builds bottom level structures of known triangles and top level structures of their instances, and traces rays against them
		 * inline, with the top level structure bound to a set and taken from the bindless heap. Checks distances, barycentrics,
		 * instance, geometry and primitive indices, facing, instance masks, ray and instance flags, any-hit callbacks, and that a top
		 * level structure keeps its bottom levels alive. Runs only with RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestInlineRayTracing();
	};
} // namespace b3d
