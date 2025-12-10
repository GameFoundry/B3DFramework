//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuDevice.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Tracks the bound/use state of a single suballocation within a buffer. */
		struct SuballocationTrackingState
		{
			u32 BoundCount = 0;  /**< Number of command buffers this suballocation is bound to. */
			u32 UseCount = 0;    /**< Number of submitted command buffers using this suballocation. */
		};
#endif

		/** Wrapper around a Vulkan buffer object that manages its usage and lifetime. */
		class VulkanBuffer : public VulkanResource
		{
		public:
			/**
			 * @param	owner		Manager that takes care of tracking and releasing of this object.
			 * @param	type		Type of the buffer being created.
			 * @param	flags		Flags that specify how is the buffer intended to be used.
			 * @param	buffer		Actual low-level Vulkan buffer handle.
			 * @param	allocation	Information about memory mapped to the buffer.
			 * @param	rowPitch	If buffer maps to an image sub-resource, length of a single row (in elements).
			 * @param	slicePitch	If buffer maps to an image sub-resource, size of a single 2D surface (in elements).
			 * @param	name		Optional name of the resource, for debugging purposes.
			 */
			VulkanBuffer(VulkanResourceManager* owner, GpuBufferType type, GpuBufferFlags flags, VkBuffer buffer, VmaAllocation allocation, u32 rowPitch = 0, u32 slicePitch = 0, const StringView& name = "");
			~VulkanBuffer();

			/** Returns the internal handle to the Vulkan object. */
			VkBuffer GetVulkanHandle() const { return mBuffer; }

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
			 * Creates a new view of this buffer or returns an existing view if one of this format was already created. Views
			 * must be freed by calling freeView() when doing using them. Only UNIFORM_TEXEL and STORAGE_TEXEL buffer types
			 * support buffer views.
			 */
			VkBufferView GetOrCreateView(VkFormat format);

#if B3D_BUILD_TYPE_DEVELOPMENT
			// --- Suballocation Tracking (Debug Only) ---

			/**
			 * Initializes suballocation tracking for the specified count. Called during buffer creation.
			 * Only needs to be called for buffers with more than one suballocation.
			 *
			 * @param suballocationCount	Number of suballocations in the buffer.
			 * @param suballocationSize		Size of each suballocation in bytes.
			 */
			void InitializeSuballocationTracking(u32 suballocationCount, u32 suballocationSize);

			// --- Notification methods (called by VulkanResourceTracker) ---

			/** Notifies that a suballocation is bound to a command buffer. */
			void NotifySuballocationBound(u32 suballocationIndex);

			/** Notifies that a suballocation is used (command buffer submitted). */
			void NotifySuballocationUsed(u32 suballocationIndex);

			/** Notifies that a suballocation is done being used (command buffer completed). */
			void NotifySuballocationDone(u32 suballocationIndex);

			/** Notifies that a suballocation is unbound (command buffer destroyed without submit). */
			void NotifySuballocationUnbound(u32 suballocationIndex);

			// --- Query methods (called during Map/Write validation) ---

			/** Checks if a suballocation is currently bound to any command buffer. */
			bool IsSuballocationBound(u32 suballocationIndex) const;

			/** Checks if a suballocation is currently in use on the GPU. */
			bool IsSuballocationInUse(u32 suballocationIndex) const;

			/** Checks if any suballocation overlapping the given byte range is bound. */
			bool IsRangeBound(u32 offset, u32 size) const;

			/** Checks if any suballocation overlapping the given byte range is in use. */
			bool IsRangeInUse(u32 offset, u32 size) const;

			/** Returns the suballocation index for the given byte offset. */
			u32 GetSuballocationIndexForOffset(u32 offset) const;
#endif

		private:
			/** Information about a view of this buffer. */
			struct ViewInformation
			{
				ViewInformation() = default;

				ViewInformation(VkFormat format, VkBufferView view)
					: Format(format), View(view)
				{}

				VkFormat Format = VK_FORMAT_UNDEFINED;
				VkBufferView View = VK_NULL_HANDLE;
			};

			GpuBufferType mType;
			GpuBufferFlags mFlags;
			VkBuffer mBuffer;
			TInlineArray<ViewInformation, 2> mViews;
			VmaAllocation mAllocation;

			u32 mRowPitch;
			u32 mSliceHeight;

			mutable VkDeviceSize mMappedOffset = 0;
			mutable VkDeviceSize mMappedSize = 0;
			mutable Mutex mViewsMutex;

#if B3D_BUILD_TYPE_DEVELOPMENT
			TInlineArray<SuballocationTrackingState, 2> mSuballocationStates;
			u32 mSuballocationSize = 0;  // Size of each suballocation (for range-to-index conversion)
#endif
		};

		/**	Vulkan-specific implementation of GpuBuffer. */
		class VulkanGpuBuffer : public GpuBuffer
		{
		public:
			VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation);
			~VulkanGpuBuffer();

			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;
			u32 GetBoundCount() const override { return mBuffer->GetBoundCount(); }
			u32 GetUseCount() const override { return mBuffer->GetUseCount(); }

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return mBuffer->IsRangeBound(offset, size); }
			bool IsRangeInUse(u32 offset, u32 size) const override { return mBuffer->IsRangeInUse(offset, size); }
#endif

			/** Gets the resource wrapping the buffer object. */
			VulkanBuffer* GetVulkanResource() const { return mBuffer; }

			/** Returns a view of the buffer object using the provided format. Only relevant for simple storage buffers. If Unknown format is provided, returns the default view. If the view was previously created, returns the existing buffer view. */
			VkBufferView GetOrCreateView(GpuBufferFormat format) const;

		protected:
			friend class VulanGpuDevice;

			/** Information about a created buffer view. */
			struct ViewInformation
			{
				ViewInformation(GpuBufferFormat format, VkBufferView view)
					: Format(format), View(view) {}

				GpuBufferFormat Format = BF_UNKNOWN;
				VkBufferView View = VK_NULL_HANDLE;
			};

			void Initialize() override;

			void* Map(u32 offset, u32 length, GpuLockOptions options) override;
			void Unmap() override;
			void RecreateInternalBuffer() override;

			/** Creates a new buffer for the specified device, matching the current buffer properties. */
			VulkanBuffer* CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable);

			/** Gets the GPU device the buffer is created on. */
			VulkanGpuDevice& GetVulkanDevice() const { return static_cast<VulkanGpuDevice&>(mDevice); }

			VulkanBuffer* mBuffer = nullptr;

			VkBufferCreateInfo mBufferCI;
			VkBufferUsageFlags mUsageFlags;
			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
			bool mIsMapped : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d

namespace b3d
{
	B3D_IMPLEMENT_GLOBAL_POOL(render::VulkanGpuBuffer, 32)
}
