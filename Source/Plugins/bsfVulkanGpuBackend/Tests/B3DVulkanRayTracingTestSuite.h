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
		 * Checks the Vulkan usage of buffers by type and flags: build inputs, and device addresses for buffers that builds read or use as
		 * scratch memory, and for no other. No buffer can store an acceleration structure.
		 */
		void TestBufferUsage();

		/**
		 * Checks that buffers that can be scratch memory have device addresses aligned to the scratch offset alignment, instance buffers
		 * to 16 bytes, and that acceleration structures have a native structure whose address is aligned to 256 bytes.
		 */
		void TestDeviceAddresses();

		/**
		 * Checks that instances are written as VkAccelerationStructureInstanceKHR, with the address of their bottom level structure, and
		 * with Vulkan's facing flipped unless the instance flips the engine's.
		 */
		void TestInstanceLayout();

		/**
		 * Checks the layout of a shader binding table against the ray tracing capabilities: region addresses aligned to the table alignment,
		 * strides to the record alignment, ray generation records that start at the table alignment each, records that hold the handles of
		 * their groups, and an empty hit group record of zeros.
		 */
		void TestShaderBindingTable();
	};
} // namespace b3d
