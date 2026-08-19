//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** End-to-end tests for Vulkan command-buffer boundary and queue handoff synchronization. */
	class VulkanBarrierTestSuite : public TestSuite
	{
	public:
		VulkanBarrierTestSuite();

	private:
		/** Copies data through a GPU-local buffer from graphics to compute with no caller-provided queue mask. */
		void TestGraphicsToComputeBufferHandoff();

		/** Copies data through a GPU-local buffer from compute to graphics with no caller-provided queue mask. */
		void TestComputeToGraphicsBufferHandoff();

		/** Copies data across queues after the source command buffer has completed and reset. */
		void TestCompletedGraphicsToComputeBufferHandoff();

		/** Waits on one completed graphics submission from two destination queues. */
		void TestCompletedQueueProgressFanOut();

		/** Copies data through a GPU-local buffer in consecutive graphics command buffers. */
		void TestSameQueueBufferBoundary();

		/** Checks that the concurrent-read texture hint enables concurrent Vulkan queue-family sharing when needed. */
		void TestConcurrentQueueReadTexture();

		/** Resolves a multisampled texture through the Vulkan transfer-native resolve path. */
		void TestMultisampleResolve();
	};
} // namespace b3d
