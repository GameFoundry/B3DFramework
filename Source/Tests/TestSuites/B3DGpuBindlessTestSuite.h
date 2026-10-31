//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for the bindless resource and sampler heaps. */
	class GpuBindlessTestSuite : public TestSuite
	{
	public:
		GpuBindlessTestSuite();

	private:
		/**
		 * Verifies slot allocation on a native heap stub: the invalid slot, capacity errors, reuse of released slots only once their frame
		 * completes, pins that outlive their release, statistics and, in development builds, the errors of mismatched releases.
		 */
		void TestHeapAllocation();

		/**
		 * Verifies the bindless indices of textures, buffers and samplers: default and additional views, the global samplers, the release of
		 * slots on destruction and the rejection of resources that can't be bindless.
		 */
		void TestResourceIndices();

		/**
		 * Dispatches a compute program that reads a structured and a typed buffer and a texture through ResourceDescriptorHeap, sampled
		 * with a cached and a global sampler through SamplerDescriptorHeap, and verifies the values it writes. Runs on devices that support
		 * bindless resources.
		 */
		void TestHeapAccess();
	};
} // namespace b3d
