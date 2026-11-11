//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests the Vulkan objects behind acceleration structures. The tests run only on devices with acceleration structures. */
	class VulkanRayTracingTestSuite : public TestSuite
	{
	public:
		VulkanRayTracingTestSuite();

	private:
		/**
		 * Checks the Vulkan usage of buffers by type and flags: acceleration structure storage, build inputs, and device addresses for
		 * buffers that builds read or use as scratch memory, and for no other.
		 */
		void TestBufferUsage();

		/**
		 * Checks that buffers that can be scratch memory have device addresses aligned to the scratch offset alignment, instance buffers
		 * to 16 bytes, and acceleration structures to 256 bytes, and that only acceleration structure buffers have a native structure.
		 */
		void TestDeviceAddresses();

		/**
		 * Checks that instances are written as VkAccelerationStructureInstanceKHR, with the address of their bottom level structure, and
		 * with Vulkan's facing flipped unless the instance flips the engine's.
		 */
		void TestInstanceLayout();
	};
} // namespace b3d
