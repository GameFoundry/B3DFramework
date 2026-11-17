//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuShaderBindingTable.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**
		 * Vulkan implementation of a shader binding table. Its records are the shader group handles of its pipeline, in one host-visible
		 * buffer written once at creation.
		 */
		class B3D_VULKAN_EXPORT VulkanGpuShaderBindingTable : public GpuShaderBindingTable
		{
		public:
			/**
			 * Creates the buffer of the table and writes the records into it.
			 *
			 * @param	device				Device that creates the table.
			 * @param	createInformation	Records of the table, which the device validated. Its pipeline must have a native pipeline.
			 */
			VulkanGpuShaderBindingTable(VulkanGpuDevice& device, const GpuShaderBindingTableCreateInformation& createInformation);
			~VulkanGpuShaderBindingTable();

			/** Returns the buffer that holds the records. */
			VulkanBuffer* GetBuffer() const { return mBuffer; }

			/** Returns where the records are in the buffer. */
			const GpuShaderBindingTableIdentifierLayout& GetLayout() const { return mLayout; }

			/** Returns ray generation record @p recordIndex as the region vkCmdTraceRaysKHR() takes: the record alone, with its size equal to its stride. */
			VkStridedDeviceAddressRegionKHR GetRayGenerationRecordRegion(u32 recordIndex) const;

			/** Returns @p region of the table as the region vkCmdTraceRaysKHR() takes. Zeros if @p region is empty. */
			VkStridedDeviceAddressRegionKHR GetDeviceAddressRegion(const GpuShaderBindingTableRegion& region) const;

		private:
			GpuShaderBindingTableIdentifierLayout mLayout;
			VulkanBuffer* mBuffer = nullptr;
		};

		/** @} */
	} // namespace render
} // namespace b3d
