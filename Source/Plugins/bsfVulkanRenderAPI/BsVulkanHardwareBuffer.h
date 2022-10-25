//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan buffer object that manages its usage and lifetime. */
		class VulkanBuffer : public VulkanResource
		{
		public:
			/**
			 * @param[in]	owner		Manager that takes care of tracking and releasing of this object.
			 * @param[in]	buffer		Actual low-level Vulkan buffer handle.
			 * @param[in]	allocation	Information about memory mapped to the buffer.
			 * @param[in]	rowPitch	If buffer maps to an image sub-resource, length of a single row (in elements).
			 * @param[in]	slicePitch	If buffer maps to an image sub-resource, size of a single 2D surface (in elements).
			 */
			VulkanBuffer(VulkanResourceManager* owner, VkBuffer buffer, VmaAllocation allocation, u32 rowPitch = 0, u32 slicePitch = 0);
			~VulkanBuffer();

			/** Returns the internal handle to the Vulkan object. */
			VkBuffer GetHandle() const { return mBuffer; }

			/**
			 * If buffer represents an image sub-resource, this is the number of elements that separate one row of the
			 * sub-resource from another (if no padding, it is equal to image width).
			 */
			u32 GetRowPitch() const { return mRowPitch; }

			/**
			 * If buffer represents an image sub-resource, this is the number of elements that separate one column of the
			 * sub-resource from another (if no padding, it is equal to image height). Only relevant for 3D images.
			 */
			u32 GetSliceHeight() const { return mSliceHeight; }

			/**
			 * Returns a pointer to internal buffer memory. Must be followed by unmap(). Caller must ensure the buffer was
			 * created in CPU readable memory, and that buffer isn't currently being written to by the GPU.
			 */
			u8* Map(VkDeviceSize offset, VkDeviceSize length) const;

			/** Unmaps a buffer previously mapped with map(). */
			void Unmap();

			/**
			 * Queues a command on the provided command buffer. The command copies the contents of the current buffer to
			 * the destination buffer. Caller must ensure the provided offsets and length are within valid bounds of
			 * both buffers.
			 */
			void Copy(VulkanCmdBuffer* cb, VulkanBuffer* destination, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize length);

			/**
			 * Queues a command on the provided command buffer. The command copies the contents of the current buffer to
			 * the destination image subresource.
			 */
			void Copy(VulkanCmdBuffer* cb, VulkanImage* destination, const VkExtent3D& extent, const VkImageSubresourceLayers& range, VkImageLayout layout);

			/**
			 * Queues a command on the provided command buffer. The command copies the contents of the provided memory location
			 * the destination buffer. Caller must ensure the provided offset and length are within valid bounds of
			 * both buffers. Caller must ensure the offset and size is a multiple of 4, and size is equal to or less then 65536.
			 */
			void Update(VulkanCmdBuffer* cb, u8* data, VkDeviceSize offset, VkDeviceSize length);

			/** @copydoc VulkanResource::notifyDone */
			void NotifyDone(u32 globalQueueIdx, VulkanAccessFlags useFlags);

			/** @copydoc VulkanResource::notifyUnbound */
			void NotifyUnbound();

			/**
			 * Creates a new view of this buffer or returns an existing view if one of this format was already created. Views
			 * must be freed by calling freeView() when doing using them. Only UNIFORM_TEXEL and STORAGE_TEXEL buffer types
			 * support buffer views.
			 */
			VkBufferView GetView(VkFormat format);

			/**
			 * Frees a previously allocated buffer view. Calling this is optional as all buffer views will be deallocated
			 * when the buffer is destroyed.
			 */
			void FreeView(VkBufferView view);

		private:
			/** Information about a view of this buffer. */
			struct ViewInfo
			{
				ViewInfo() = default;

				ViewInfo(VkFormat format, VkBufferView view)
					: Format(format), View(view), UseCount(1)
				{}

				VkFormat Format = VK_FORMAT_UNDEFINED;
				VkBufferView View = VK_NULL_HANDLE;
				u32 UseCount = 0;
			};

			/**
			 * Destroys any buffer views are currently not being used. This must only be called after the buffer is done
			 * being used on a command buffer.
			 */
			void DestroyUnusedViews();

			VkBuffer mBuffer;
			Vector<ViewInfo> mViews;
			VmaAllocation mAllocation;

			u32 mRowPitch;
			u32 mSliceHeight;
		};

		/**	Class containing common functionality for all Vulkan hardware buffers. */
		class VulkanHardwareBuffer : public HardwareBuffer
		{
		public:
			/**	Available types of Vulkan buffers. */
			enum BufferType
			{
				/** Contains geometry vertices and their properties. */
				BT_VERTEX,
				/** Contains triangle to vertex mapping. */
				BT_INDEX,
				/** Contains GPU program parameters. */
				BT_UNIFORM,
				/** Generic GPU buffer containing non-formatted data. */
				BT_GENERIC,
				/** Generic GPU buffer containing structured data. */
				BT_STRUCTURED
			};

			VulkanHardwareBuffer(BufferType type, GpuBufferFormat format, GpuBufferUsage usage, u32 size, GpuDeviceFlags deviceMask = GDF_DEFAULT);
			~VulkanHardwareBuffer();

			/** @copydoc HardwareBuffer::readData */
			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0);

			/** @copydoc HardwareBuffer::writeData */
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0);

			/** @copydoc HardwareBuffer::copyData */
			void CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/**
			 * Gets the resource wrapping the buffer object, on the specified device. If hardware buffer device mask doesn't
			 * include the provided device, null is returned.
			 */
			VulkanBuffer* GetResource(u32 deviceIdx) const { return mBuffers[deviceIdx]; }

		protected:
			/** @copydoc HardwareBuffer::map */
			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx);

			/** @copydoc HardwareBuffer::unmap */
			void Unmap();

			/** Creates a new buffer for the specified device, matching the current buffer properties. */
			VulkanBuffer* CreateBuffer(VulkanDevice& device, u32 size, bool staging, bool readable);

			VulkanBuffer* mBuffers[BS_MAX_DEVICES];

			VulkanBuffer* mStagingBuffer;
			u8* mStagingMemory;
			u32 mMappedDeviceIdx;
			u32 mMappedGlobalQueueIdx;
			u32 mMappedOffset;
			u32 mMappedSize;
			GpuLockOptions mMappedLockOptions;

			VkBufferCreateInfo mBufferCI;
			VkBufferUsageFlags mUsageFlags;
			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
			bool mIsMapped : 1;
		};

		/** @} */
	} // namespace ct
} // namespace bs

namespace bs
{
	IMPLEMENT_GLOBAL_POOL(ct::VulkanHardwareBuffer, 32)
}
