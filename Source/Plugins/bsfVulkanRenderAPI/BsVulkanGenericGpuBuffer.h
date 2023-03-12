//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGenericGpuBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Vulkan implementation of a generic GPU buffer. */
		class VulkanGenericGpuBuffer : public GenericGpuBuffer
		{
		public:
			~VulkanGenericGpuBuffer();

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;

			/**
			 * Gets the resource wrapping the buffer object, on the specified device. If the object wasn't initialized for the
			 * specified device, null is returned.
			 */
			VulkanBuffer* GetResource(u32 deviceIdx) const;

			/**
			 * Returns a view of the buffer object using the format specified when creating this object, per device. If the
			 * object wasn't initialized for the specified device, null is returned.
			 */
			VkBufferView GetView(u32 deviceIdx) const;

		protected:
			friend class VulkanHardwareBufferManager;

			VulkanGenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask);
			VulkanGenericGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer);

			void Initialize() override;
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void Unmap() override;

			/** Updates buffer views if they are missing or don't match the buffers. */
			void UpdateViews();

		private:
			VkBuffer mCachedBuffers[B3D_MAX_DEVICES]{};
			VkBufferView mBufferViews[B3D_MAX_DEVICES]{};
		};

		/** @} */
	} // namespace ct
} // namespace bs
