//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsGpuBuffer.h"
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

			/** Assigns an name to the buffer, primarily used for easier debugging. */
			void SetName(const StringView& name);

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
			 * Returns a pointer to internal buffer memory. Must be followed by Unmap(). Caller must ensure the buffer was
			 * created in CPU readable memory, and that buffer isn't currently being written to by the GPU.
			 *
			 * @param	offset					Offset into the allocation which to map from, in bytes.
			 * @param	size					Amount of bytes to map, starting with @p offset.
			 * @param	isInvalidateRequired	Ensures any GPU writes are made visible to the CPU before mapping. This is required for buffers
			 *									allocated in non-coherent memory and will be ignored for ones allocated in coherent memory.
			 */
			u8* Map(VkDeviceSize offset, VkDeviceSize size, bool isInvalidateRequired = false) const;

			/**
			 * Unmaps a buffer previously mapped with Map().
			 *
			 * @param	isFlushRequired			Ensures any CPU writes are made visible to the GPU after unmapping. This is required for buffers
			 *									allocated in non-coherent memory and will be ignored for ones allocated in coherent memory.
			 */
			void Unmap(bool isFlushRequired = false);

			/**
			 * Queues a command on the provided command buffer. The command copies the contents of the provided memory location
			 * the destination buffer. Caller must ensure the provided offset and length are within valid bounds of
			 * both buffers. Caller must ensure the offset and size is a multiple of 4, and size is equal to or less then 65536.
			 */
			void Update(VulkanInternalCommandBuffer* cb, u8* data, VkDeviceSize offset, VkDeviceSize length);

			void NotifyDone(u32 globalQueueIdx, VulkanAccessFlags useFlags) override;
			void NotifyUnbound() override;

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

			mutable VkDeviceSize mMappedOffset = 0;
			mutable VkDeviceSize mMappedSize = 0;
			mutable Mutex mViewsMutex;
		};

		/**	Vulkan-specific implementation of GpuBuffer. */
		class VulkanGpuBuffer : public GpuBuffer
		{
		public:
			VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation);
			~VulkanGpuBuffer();

			void SetName(const StringView& name) override;
			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;
			void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

			/**
			 * Gets the resource wrapping the buffer object, on the specified device. If hardware buffer device mask doesn't
			 * include the provided device, null is returned.
			 */
			VulkanBuffer* GetResource(u32 deviceIdx) const
			{
				B3D_ASSERT(deviceIdx == 0);
				return mBuffer;
			}

		protected:
			friend class VulanGpuDevice;

			void Initialize() override;

			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;
			void Unmap() override;

			/** Creates a new buffer for the specified device, matching the current buffer properties. */
			VulkanBuffer* CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable);

			VulkanGpuDevice& mDevice;
			VulkanBuffer* mBuffer = nullptr;

			VulkanBuffer* mStagingBuffer = nullptr;
			u8* mStagingMemory = nullptr;
			u32 mMappedGlobalQueueIdx = ~0u;
			u32 mMappedOffset = 0;
			u32 mMappedSize = 0;
			GpuLockOptions mMappedLockOptions = GBL_WRITE_ONLY;

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
	B3D_IMPLEMENT_GLOBAL_POOL(ct::VulkanGpuBuffer, 32)
}
