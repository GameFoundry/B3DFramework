//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	DirectX 11 implementation of a generic GPU buffer. */
		class VulkanGpuBuffer : public GpuBuffer
		{
		public:
			~VulkanGpuBuffer();

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

			VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);
			VulkanGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

			void Initialize() override;
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void Unmap() override;

			/** Updates buffer views if they are missing or don't match the buffers. */
			void UpdateViews();

		private:
			VkBuffer mCachedBuffers[BS_MAX_DEVICES]{};
			VkBufferView mBufferViews[BS_MAX_DEVICES]{};
		};

		/** @} */
	} // namespace ct
} // namespace bs
