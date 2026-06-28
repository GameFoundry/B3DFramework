//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuDevice.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanGpuBuffer;

		/** Descriptor used to create a VulkanBuffer. */
		struct VulkanBufferCreateInformation
		{
			VkBufferCreateInfo VkCreateInfo{}; /**< Vulkan-level descriptor used to create the underlying VkBuffer. */
			GpuBufferType Type = GpuBufferType::Vertex; /**< Type of the buffer being created. */
			GpuBufferFlags Flags; /**< Flags that specify how the buffer is intended to be used. */
			StringView DebugName; /**< Optional name of the resource, for debugging purposes. */
		};

		/** Wrapper around a Vulkan buffer object that manages its usage and lifetime. */
		class VulkanBuffer : public TVulkanResource<IGpuBufferResource>
		{
		public:
			/**
			 * @param	owner				Manager that takes care of tracking and releasing of this object.
			 * @param	createInformation	Describes the buffer being wrapped.
			 * @param	buffer				Internal Vulkan buffer handle that the wrapper takes ownership of.
			 * @param	allocation			Memory binding for this buffer.
			 * @param	parent				High-level VulkanTexture proxy that owns this wrapper, or nullptr for transient staging images / unowned wrappers. 
			 *								Required for the wrapper to participate in defragmentation.
			 */
			VulkanBuffer(VulkanResourceManager* owner, const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, VulkanAllocationResult allocation, VulkanGpuBuffer* parent = nullptr);
			~VulkanBuffer();

			IGpuResource* MoveAllocation(render::GpuCommandBuffer& commandBuffer, const GpuResourceLocation& newLocation) override;

			/** Returns the internal handle to the Vulkan object. */
			VkBuffer GetVulkanHandle() const { return mBuffer; }

			/** Assigns an name to the buffer, primarily used for easier debugging. */
			void SetName(const StringView& name);

			/** Returns a pointer to persistently mapped memory of the buffer, or null pointer if the buffer is not mappable. */
			void* GetMappedMemory() const { return mMappedMemory; }

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

			/** Flushes any CPU writes to the buffer to make them visible to the GPU. Only relevant for non-coherent memory. */
			void Flush(VkDeviceSize offset, VkDeviceSize size);

			/** Invalidates any GPU writes to the buffer to make them visible to the CPU. Only relevant for non-coherent memory. */
			void Invalidate(VkDeviceSize offset, VkDeviceSize size);

			/**
			 * Creates a new view of this buffer or returns an existing view if one of this format was already created. Views
			 * must be freed by calling freeView() when doing using them. Only UNIFORM_TEXEL and STORAGE_TEXEL buffer types
			 * support buffer views.
			 */
			VkBufferView GetOrCreateView(VkFormat format);

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
			VulkanAllocationResult mAllocation;
			VulkanGpuBuffer* mParent = nullptr;
			void* mMappedMemory = nullptr;

			mutable VkDeviceSize mMappedOffset = 0;
			mutable VkDeviceSize mMappedSize = 0;
			mutable Mutex mViewsMutex;
		};

		/**	Vulkan-specific implementation of GpuBuffer. */
		class VulkanGpuBuffer : public GpuBuffer
		{
		public:
			/**
			 * @param	device				Device the buffer is created on.
			 * @param	createInformation	Describes the buffer to create.
			 * @param	allocator			Allocator backing the buffer's memory, resolved up front for the buffer's
			 *								memory type (see VulkanGpuDevice::PickBufferMemoryType). The buffer
			 *								allocates through it for its whole lifetime, including internal recreation.
			 *								Must outlive the buffer.
			 */
			VulkanGpuBuffer(VulkanGpuDevice& device, const GpuBufferCreateInformation& createInformation, IGpuAllocator& allocator);
			~VulkanGpuBuffer();

			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;
			u32 GetBoundCount() const override { return mBuffer->GetBoundCount(); }
			u32 GetUseCount() const override { return mBuffer->GetUseCount(); }

			void Flush(u32 offset, u32 size) override;
			void Invalidate(u32 offset, u32 size) override;

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return mBuffer->IsRangeBound(offset, size); }
			bool IsRangeInUse(u32 offset, u32 size) const override { return mBuffer->IsRangeInUse(offset, size); }
#endif

			/** Gets the resource wrapping the buffer object. */
			VulkanBuffer* GetVulkanResource() const { return mBuffer; }

			/** Returns a view of the buffer object using the provided format. Only relevant for simple storage buffers. If Unknown format is provided, returns the default view. If the view was previously created, returns the existing buffer view. */
			VkBufferView GetOrCreateView(GpuBufferFormat format) const;

			/** Maps a buffer's engine usage (@p createInformation Type + Flags) to its Vulkan buffer usage bits. */
			static VkBufferUsageFlags GetVkBufferUsageFlags(const GpuBufferCreateInformation& createInformation);

			/**
			 * Maps a buffer's engine usage (@p createInformation Type + Flags) to the (required, preferred)
			 * memory-property flags that drive memory-type selection.
			 */
			static void GetVkMemoryPropertyFlags(const GpuBufferCreateInformation& createInformation, VkMemoryPropertyFlags& requiredFlags, VkMemoryPropertyFlags& preferredFlags);

		protected:
			friend class VulanGpuDevice;
			friend class VulkanBuffer;

			/** Information about a created buffer view. */
			struct ViewInformation
			{
				ViewInformation(GpuBufferFormat format, VkBufferView view)
					: Format(format), View(view) {}

				GpuBufferFormat Format = BF_UNKNOWN;
				VkBufferView View = VK_NULL_HANDLE;
			};

			void Initialize() override;
			void RecreateInternalBuffer() override;

			/** Creates a new buffer for the specified device, matching the current buffer properties. */
			VulkanBuffer* CreateBuffer(VulkanGpuDevice& device, u32 size, bool staging, bool readable, const VulkanAllocationResult* preAllocatedMemory = nullptr);

			/**
			 * Recreates this proxy's internal VulkanBuffer at the provided pre-reserved allocation slot,
			 * records a GPU-side copy from the current buffer into the new one on @p commandBuffer. The caller
			 * is responsible for queuing the old wrapper for destroy.
			 */
			VulkanBuffer* RelocateInternalBuffer(const VulkanAllocationResult& preReserved, render::GpuCommandBuffer& commandBuffer);

			/** Gets the GPU device the buffer is created on. */
			VulkanGpuDevice& GetVulkanDevice() const { return static_cast<VulkanGpuDevice&>(mDevice); }

			VulkanBuffer* mBuffer = nullptr;

			/** Allocator the buffer's memory is suballocated from. Resolved once at creation, for the buffer's memory type. */
			IGpuAllocator& mAllocator;

			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d

namespace b3d
{
	B3D_IMPLEMENT_GLOBAL_POOL(render::VulkanGpuBuffer, 32)
}
