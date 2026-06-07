//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanBuiltinResources.h"
#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanHeapBackend.h"
#include "Managers/B3DVulkanDescriptorManager.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"

namespace b3d
{
	class VulkanGpuBackend;
	class VulkanGpuTimelineFence;

	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		class VulkanBuffer;
		class VulkanGpuBuffer;
		class VulkanImage;
		class VulkanTexture;
		struct VulkanBufferCreateInformation;
		struct VulkanImageCreateInformation;

		/** Contains format describing a Vulkan surface. */
		struct SurfaceFormat
		{
			VkFormat ColorFormat;
			VkFormat DepthFormat;
			VkColorSpaceKHR ColorSpace;
		};

		/**
		 * Result from the allocation functions. Wraps a GPU location (offset within a VkDeviceMemory heap)
		 * plus an optional persistent map. MappedMemory is non-null when the allocation lives in a
		 * persistently-mapped, host-visible heap and points to the start of the allocation's memory range.
		 */
		struct VulkanAllocationResult
		{
			GpuResourceLocation Location; /**< Allocator slot — heap, offset, size, allocator-private bookkeeping. */
			void* MappedMemory = nullptr; /**< Heap.Mapped + Location.Offset for host-visible heaps; null otherwise. */
			bool IsTransient = false; /**< True if produced by the linear (bump) allocator rather than the TLSF allocator; controls which allocator frees it. */

			/** Returns true once the allocator has populated this result with a live slot. */
			bool IsValid() const { return Location.IsValid(); }
		};

		/** Represents a single GPU device usable by Vulkan. */
		class VulkanGpuDevice : public GpuDevice
		{
		public:
#if B3D_PLATFORM_MACOS
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageMvksl;
#else
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageVksl;
#endif


			VulkanGpuDevice(VkPhysicalDevice device);
			~VulkanGpuDevice();

			/**
			 * @name GpuDevice Interface
			 * @{
			 */

			bool IsInitialized() const override { return true; }
			bool Initialize() override { return true; } // Initialized on construction

			const GpuDeviceCapabilities& GetCapabilities() const override { return mCapabilities; }
			const VideoModeInfo& GetVideoModeInfo() const override { return *mVideoModeInfo; }

			bool IsGpuProgramLanguageSupported(const StringView& language) const override { return language == kGpuProgramLanguageName; }
			TShared<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const override;

			u32 GetQueueCount(GpuQueueType type) const override { return (u32)mQueueInfos[(u32)type].Queues.size(); }
			TShared<GpuQueue> GetQueue(GpuQueueType type, u32 index) const override;
			void SubmitTransferCommandBuffers(bool wait = false) override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			TShared<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription) override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override;

			/** @} */

			/** Returns an object describing the physical properties of the device. */
			VkPhysicalDevice GetPhysical() const { return mPhysicalDevice; }

			/** Returns an object describing the logical properties of the device. */
			VkDevice GetLogical() const { return mLogicalDevice; }

			/** Returns true if the device is one of the primary GPU's. */
			bool IsPrimary() const { return mIsPrimary; }

			/** Returns a set of properties describing the physical device. */
			const VkPhysicalDeviceProperties& GetDeviceProperties() const { return mDeviceProperties; }

			/** Returns a set of features that the application can use to check if a specific feature is supported. */
			const VkPhysicalDeviceFeatures& GetDeviceFeatures() const { return mDeviceFeatures; }

			/** Returns a set of properties describing the memory of the physical device. */
			const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return mMemoryProperties; }

			/**
			 * Returns index of the queue family for the specified queue type. Returns -1 if no queues for the specified type
			 * exist. There will always be a queue family for the graphics type.
			 */
			u32 GetQueueFamily(GpuQueueType type) const { return mQueueInfos[(int)type].FamilyIndex; }

			/** Perform an operation for each queue on the device. */
			void DoForEachQueue(const std::function<void(VulkanGpuQueue&)>&& callback) const;

			/** Returns the best matching surface format according to the provided parameters. */
			SurfaceFormat GetSurfaceFormat(const VkSurfaceKHR& surface, bool useHardwareSRGB) const;

			/** Returns a manager that can be used for allocating descriptor layouts and sets. */
			VulkanDescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that can be used for allocating Vulkan objects wrapped as managed resources. */
			VulkanResourceManager& GetResourceManager() const { return *mResourceManager; }

			/** Returns a set of resources that are always available. */
			const VulkanBuiltinResources& GetBuiltinResources() const { return mBuiltinResources;  }

			/**
			 * Returns a set of command buffer semaphores depending on the provided sync mask.
			 *
			 * @param	syncMask		Mask that has a bit enabled for each queue to retrieve the semaphore for.
			 *							If a command buffer on a queue is not currently executing, semaphore won't be returned.
			 * @param	outSemaphores	Array into which all required semaphores will be appended to. 
			 *
			 * @note	Submit thread only.
			 */
			void GetSyncSemaphores(GpuQueueMask syncMask, TInlineArray<VulkanSemaphore*, 8> outSemaphores) const;

			/**
			 * @name Resource Creation
			 * @{
			 */

			/**
			 * Creates a VkBuffer described by @p info, suballocates compatible memory and binds the two 
			 * together, and wraps the result in a VulkanBuffer. Memory-type selection picks the best match 
			 * satisfying @p requiredFlags with a preference scoring against @p preferredFlags. 

			 * Provide @p parent so the buffer can participate in defragmentation - the parent will be notified
			 * when it needs to re-allocate the buffer in the new destination.
			 *
			 * Thread safe.
			 */
			VulkanBuffer* CreateBuffer(const VulkanBufferCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, VulkanGpuBuffer* parent);

			/**
			 * Same as the other overload, but binds the VkBuffer to externally allocated @p allocation slot instead of allocating new memory.
			 *
			 * Thread safe.
			 */
			VulkanBuffer* CreateBuffer(const VulkanBufferCreateInformation& createInformation, const VulkanAllocationResult& allocation, VulkanGpuBuffer* parent);

			/**
			 * Creates a VkImage described by @p info, suballocates compatible memory and binds the 
			 * two together, and wraps the result in a VulkanImage. @p kind controls buffer-image 
			 * granularity placement (Non-linear for optimally-tiled images, Linear for linearly-tiled).
			 *
			 * Provide @p parent so the buffer can participate in defragmentation - the parent will be notified
			 * when it needs to re-allocate the image in the new destination.
			 *
			 * Thread safe.
			 */
			VulkanImage* CreateImage(const VulkanImageCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, GpuResourceKind kind, VulkanTexture* parent);

			/**
			 * Same as the other overload, but binds the VkImage to externally allocated @p allocation slot instead of allocating new memory.
			 *
			 * Thread safe.
			 */
			VulkanImage* CreateImage(const VulkanImageCreateInformation& createInformation, const VulkanAllocationResult& allocation, VulkanTexture* parent);

			/**
			 * Returns @p allocation to its allocator's free pool synchronously. The slot becomes
			 * immediately available for reuse.
			 *
			 * Caller must guarantee the GPU is no longer using the underlying memory range.
			 *
			 * Thread safe.
			 */
			void FreeMemory(VulkanAllocationResult& allocation);

			/**
			 * Returns the persistent CPU pointer for @p allocation, offset by @p offset bytes. The allocation must
			 * live in a host-visible memory type (heaps for those types are persistently mapped on creation).
			 *
			 * Thread safe.
			 */
			u8* MapMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset = 0) const;

			/**
			 * No-op for persistently-mapped heaps; retained for symmetry with MapMemory.
			 *
			 * Thread safe.
			 */
			void UnmapMemory(const VulkanAllocationResult& allocation) const;

			/**
			 * Invalidates @p [offset, offset+size) within @p allocation so subsequent CPU reads observe GPU writes.
			 * Only relevant for non-coherent memory.
			 *
			 * Thread safe.
			 */
			void InvalidateMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

			/**
			 * Flushes @p [offset, offset+size) within @p allocation so subsequent GPU reads observe CPU writes.
			 * Only relevant for non-coherent memory.
			 *
			 * Thread safe.
			 */
			void FlushMemory(const VulkanAllocationResult& allocation, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

			/** @} */

			/** Returns true if timeline semaphores are available on this device. */
			bool SupportsTimelineSemaphores() const { return mSupportsTimelineSemaphore; }

			/** Returns the device heap backend. */
			VulkanHeapBackend& GetHeapBackend() const { return *mHeapBackend; }

		private:
			friend class b3d::VulkanGpuBackend;

			static constexpr u32 kQueueUsageCombinationCount = 8; // 3^2, as there are three usage types in CommandBufferUsageFlag

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/**
			 * Allocates a memory slot for @p image. Picks the best memory type satisfying @p requiredFlags and
			 * (where possible) the @p preferredFlags hint, then suballocates from the per-memory-type allocator.
			 * @p kind controls buffer-image granularity placement. The caller is responsible for binding via
			 * vkBindImageMemory; this method does not bind.
			 *
			 * Internal — invoked only by CreateImage. External callers route through CreateImage so the wrapper
			 * registration with the allocator (owner stamping for defragmentation) cannot be skipped.
			 */
			VulkanAllocationResult AllocateMemory(VkImage image, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, GpuResourceKind kind);

			/**
			 * Allocates a memory slot for @p buffer. Picks the best memory type satisfying @p requiredFlags and
			 * (where possible) the @p preferredFlags hint, then suballocates from the per-memory-type allocator.
			 * The caller is responsible for binding via vkBindBufferMemory; this method does not bind.
			 *
			 * When @p transient is true the slot is taken from the per-memory-type linear (bump) allocator instead of the
			 * general-purpose TLSF allocator.
			 *
			 * Internal — invoked only by CreateBuffer.
			 */
			VulkanAllocationResult AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, bool transient);

			/**
			 * Common bind-and-wrap helper for buffers. Binds @p buffer to @p allocation, constructs the
			 * VulkanBuffer wrapper, and stamps the allocator owner when @p parent is non-null.
			 */
			VulkanBuffer* BindBufferToAllocation(const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, VulkanAllocationResult allocation, VulkanGpuBuffer* parent);

			/**
			 * Common bind-and-wrap helper for images. Binds @p image to @p allocation, constructs the
			 * VulkanImage wrapper, and stamps the allocator owner when @p parent is non-null. 
			 */
			VulkanImage* BindBufferToAllocation(const VulkanImageCreateInformation& info, VkImage image, VulkanAllocationResult allocation, VulkanTexture* parent);

			/**
			 * Associates a IGpuResource owner onto an existing allocation, so the allocation participates
			 * in defragmentation. Untracked wrappers (parent == nullptr) skip this call and stay ineligible.
			 */
			void SetAllocationOwner(const VulkanAllocationResult& allocation, IGpuResource* owner);

			/**
			 * Picks a memory-type index satisfying @p typeBits and the @p required flags, with a preference
			 * scoring against @p preferred. Returns VK_MAX_MEMORY_TYPES on failure.
			 */
			u32 PickMemoryTypeIndex(u32 typeBits, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const;

			/**
			 * Returns the GPU memory allocator backing memory type @p memoryTypeIndex, lazily creating it on
			 * first use. Each allocator owns its own VkDeviceMemory heaps via the shared VulkanHeapBackend
			 * and is locked to a single memory-type index.
			 */
			TGpuTlsfAllocator<VulkanHeapBackend>& GetOrCreateGpuMemoryAllocator(u32 memoryTypeIndex);

			/**
			 * Returns the linear (bump) allocator backing memory type @p memoryTypeIndex, lazily creating it on first use.
			 * Used for transient buffer allocations (GpuBufferFlag::Transient). Pages retire against the device frame fence
			 * and are reclaimed in bulk via EndFrame/WaitUntilIdle rather than per-allocation.
			 */
			TGpuLinearAllocator<VulkanHeapBackend>& GetOrCreateGpuLinearAllocator(u32 memoryTypeIndex);

			/**
			 * Runs an opportunistic defragmentation pass across every GPU allocators that support it.
			 *
			 * No-op when mDefragEnabled is false. Bounded by mDefragBudgetBytes / mDefragBudgetAllocations
			 * so a single frame can't stall on copying many GBs of memory.
			 */
			void RunDefragPass();

			/**
			 * Reclaims memory held by the per-memory-type linear (transient) allocators. Retires each allocator's
			 * active page against the current frame index, then drains every page whose frame is no longer in flight
			 * on the GPU. Called once per frame from EndFrame (on the render thread); because the linear allocators
			 * are shared and thread-safe this reclaims transient buffers allocated by any thread during the frame.
			 */
			void ReclaimLinearAllocators();

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			VkPhysicalDevice mPhysicalDevice;
			VkDevice mLogicalDevice = nullptr;
			bool mIsPrimary = false;

			VulkanDescriptorManager* mDescriptorManager;
			VulkanResourceManager* mResourceManager;
			VulkanBuiltinResources mBuiltinResources;

			VkPhysicalDeviceProperties mDeviceProperties;
			VkPhysicalDeviceFeatures mDeviceFeatures;
			VkPhysicalDeviceMemoryProperties mMemoryProperties;

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				Vector<TShared<VulkanGpuQueue>> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

			bool mSupportsTimelineSemaphore = false;
			TUnique<VulkanHeapBackend> mHeapBackend;

			/** Per-memory-type TLSF allocator pool. Slots are lazily populated on first allocation. */
			TUnique<TGpuTlsfAllocator<VulkanHeapBackend>> mGpuMemoryAllocators[VK_MAX_MEMORY_TYPES];

			/** Guards lazy creation of mTlsfAllocators entries. */
			mutable Mutex mGpuMemoryAllocatorMutex;

			/** Per-memory-type linear (bump) allocator pool for transient buffers. Slots are lazily populated on first transient allocation. */
			TUnique<TGpuLinearAllocator<VulkanHeapBackend>> mGpuLinearAllocators[VK_MAX_MEMORY_TYPES];

			/** Guards lazy creation of mGpuLinearAllocators entries. */
			mutable Mutex mGpuLinearAllocatorMutex;

			u64 mDefragBudgetBytes = 8ull * 1024 * 1024;
			u32 mDefragBudgetAllocations = 8;
			bool mDefragEnabled = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
