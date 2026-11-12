//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan acceleration structure and the buffer that holds its memory, which manages their usage and lifetime. */
		class VulkanAccelerationStructure : public VulkanResource
		{
		public:
			/**
			 * @param	owner				Manager that takes care of tracking and releasing of this object.
			 * @param	buffer				Buffer bound to memory that stores the structure, created with acceleration structure storage and
			 *								device address usage and at least the structure's size. The structure takes ownership of it.
			 * @param	createInformation	Level, size and name of the structure.
			 */
			VulkanAccelerationStructure(VulkanResourceManager* owner, VulkanBuffer& buffer, const GpuAccelerationStructureCreateInformation& createInformation);
			~VulkanAccelerationStructure();

			/** Returns the internal handle to the Vulkan object. */
			VkAccelerationStructureKHR GetVulkanHandle() const { return mAccelerationStructure; }

			/** Returns the address builds and top level instances refer to the structure by. */
			VkDeviceAddress GetDeviceAddress() const { return mDeviceAddress; }

			/** Returns the buffer that stores the structure. Barriers of the structure apply to it. */
			VulkanBuffer& GetBuffer() const { return *mBuffer; }

		private:
			VulkanBuffer* mBuffer;
			VkAccelerationStructureKHR mAccelerationStructure = VK_NULL_HANDLE;
			VkDeviceAddress mDeviceAddress = 0;
		};

		/** Vulkan implementation of an acceleration structure. */
		class VulkanGpuAccelerationStructure : public GpuAccelerationStructure
		{
		public:
			/**
			 * @param	device					Device that created the structure.
			 * @param	createInformation		Description of the structure, which the device validated.
			 * @param	accelerationStructure	Native structure, which the base class destroys.
			 */
			VulkanGpuAccelerationStructure(VulkanGpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation, VulkanAccelerationStructure& accelerationStructure);

			/** Returns the native structure. */
			VulkanAccelerationStructure* GetVulkanResource() const { return mAccelerationStructure; }

		private:
			VulkanAccelerationStructure* mAccelerationStructure;
		};

		/** @} */
	} // namespace render
} // namespace b3d
