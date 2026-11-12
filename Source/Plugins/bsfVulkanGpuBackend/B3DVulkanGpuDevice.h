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
#include "B3DVulkanResource.h"
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
		 * Memory a native resource is bound to. Wraps a GPU allocation (offset within a VkDeviceMemory heap)
		 * plus an optional persistent map. MappedMemory is non-null when the allocation lives in a
		 * persistently-mapped, host-visible heap and points to the start of the allocation's memory range.
		 */
		struct VulkanAllocationResult
		{
			GpuAllocation Allocation; /**< Allocator slot — heap, offset, size, owning allocator, allocator-private bookkeeping. */
			void* MappedMemory = nullptr; /**< Heap.Mapped + Allocation.Offset for host-visible heaps; null otherwise. */
		};

		/** Represents a single GPU device usable by Vulkan. */
		class VulkanGpuDevice : public GpuDevice, private IGpuSubmitThreadBackend
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageVksl;

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

			u32 GetQueueCount(GpuQueueType type) const override { return (u32)mQueueInfos[(u32)type].Queues.size(); }
			TShared<GpuQueue> GetQueue(GpuQueueType type, u32 index) const override;
			void WaitUntilIdle() override;
			void BeginFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			GpuMemoryRequirements GetMemoryRequirements(const TextureCreateInformation& createInformation) const override;
			GpuMemoryRequirements GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const override;
			bool GetAccelerationStructureSizes(const GpuAccelerationStructureInputs& inputs, GpuAccelerationStructureSizes& outSizes) const override;
			bool WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance> instances, void* destination) const override;
			IGpuAllocator& GetPersistentAllocator(u32 memoryType) override;
			void GetPersistentAllocators(TArray<IGpuAllocator*>& outAllocators) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;
			TShared<GpuSplitBarrier> CreateSplitBarrier(TOptional<GpuQueueType> releaseQueue, TOptional<GpuQueueType> acquireQueue) override;
			TUnique<IGpuAllocator> CreateScratchAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker) override;
			IGpuTransientHeapPool* GetTransientHeapPool(u32 memoryType) override;

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

			/**
			 * Returns the distinct queue families of every queue type the device has. Resources shared between queues use
			 * concurrent sharing with these families, if there is more than one.
			 */
			const TInlineArray<u32, GQT_COUNT>& GetQueueFamilies() const { return mQueueFamilies; }

			/** Returns the best matching surface format according to the provided parameters. */
			SurfaceFormat GetSurfaceFormat(const VkSurfaceKHR& surface, bool useHardwareSRGB) const;

			/** Returns a manager that can be used for allocating descriptor layouts and sets. */
			VulkanDescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that can be used for allocating Vulkan objects wrapped as managed resources. */
			VulkanResourceManager& GetResourceManager() const { return *mResourceManager; }

			/** Returns a set of resources that are always available. */
			const VulkanBuiltinResources& GetBuiltinResources() const { return mBuiltinResources;  }

			/** Returns the layout of the bindless heap set, or null if the device doesn't support bindless resources. */
			VulkanDescriptorLayout* GetBindlessHeapLayout() const { return mBindlessHeapLayout; }

			/** Returns true if the bufferDeviceAddress feature is enabled. */
			bool IsBufferDeviceAddressEnabled() const { return mIsBufferDeviceAddressEnabled; }

			/**
			 * Returns the descriptor types the resource binding of the bindless heap set can hold. Includes acceleration structures if
			 * GpuRayTracingCapabilities::SupportsBindlessAccelerationStructures is true. Empty if the device doesn't support bindless resources.
			 */
			TArrayView<const VkDescriptorType> GetBindlessResourceTypes() const { return TArrayView<const VkDescriptorType>(mBindlessResourceTypes.Data(), mBindlessResourceTypes.Size()); }

			/**
			 * @name Resource Creation
			 * @{
			 */

			/**
			 * Creates a VkBuffer described by @p createInformation, binds it to memory at @p requestedAllocation and wraps the result in a
			 * VulkanBuffer. A pending allocation suballocates the memory from its allocator, which must serve the
			 * buffer's memory type (see GetMemoryRequirements()). An allocation with memory binds the buffer to it; the
			 * buffer frees it on destruction only if the allocation is owned. Outputs the memory the buffer is in to @p outAllocation.
			 *
			 * Thread safe if the allocation's allocator is.
			 */
			VulkanBuffer* CreateBuffer(const VulkanBufferCreateInformation& createInformation, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation);

			/**
			 * Creates a VkBuffer described by @p createInformation, suballocates compatible memory from the persistent allocators and
			 * binds the two together, and wraps the result in a VulkanBuffer. For buffers the engine never sees as a GpuBuffer.
			 *
			 * Thread safe.
			 */
			VulkanBuffer* CreateBuffer(const VulkanBufferCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags);

			/**
			 * Creates a VkImage described by @p createInformation, suballocates compatible memory from the persistent allocators and
			 * binds the two together, and wraps the result in a VulkanImage.
			 *
			 * Thread safe.
			 */
			VulkanImage* CreateImage(const VulkanImageCreateInformation& createInformation, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags);

			/**
			 * Creates a VkImage described by @p createInformation, binds it to memory at @p requestedAllocation and wraps the result in a
			 * VulkanImage. Allocation rules match CreateBuffer(const VulkanBufferCreateInformation&, const GpuAllocation&, GpuAllocation&).
			 * Outputs the memory the image is in to @p outAllocation.
			 *
			 * Thread safe if the allocation's allocator is.
			 */
			VulkanImage* CreateImage(const VulkanImageCreateInformation& createInformation, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation);

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

			/** Returns the device heap backend. */
			VulkanHeapBackend& GetHeapBackend() const { return *mHeapBackend; }

		private:
			friend class b3d::VulkanGpuBackend;

			static constexpr u32 kQueueUsageCombinationCount = 8; // 3^2, as there are three usage types in CommandBufferUsageFlag

			/**
			 * @name IGpuSubmitThreadBackend implementation
			 * @{
			 */

			void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) override;
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, u64 lastFenceValue) override;
			u64 GetLastSubmittedFenceValue(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuAccelerationStructure> CreateGpuAccelerationStructureInternal(const GpuAccelerationStructureCreateInformation& createInformation) override;
			GpuVertexInput* CreateVertexInput(u32 id, const GpuVertexInputLayout& layout) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/**
			 * Resolves @p requestedAllocation into the memory a native resource with @p requirements is bound to. A pending allocation
			 * suballocates from its allocator; an allocation with memory is used as is.
			 */
			VulkanAllocationResult ResolveAllocation(const GpuAllocation& requestedAllocation, const VkMemoryRequirements& requirements) const;

			/** Common bind-and-wrap helper for buffers. Binds @p buffer to @p allocation and constructs the VulkanBuffer wrapper. */
			VulkanBuffer* BindBufferToAllocation(const VulkanBufferCreateInformation& createInformation, VkBuffer buffer, const VulkanAllocationResult& allocation);

			/** Common bind-and-wrap helper for images. Binds @p image to @p allocation and constructs the VulkanImage wrapper. */
			VulkanImage* BindBufferToAllocation(const VulkanImageCreateInformation& info, VkImage image, const VulkanAllocationResult& allocation);

			/**
			 * Picks a memory-type index satisfying @p typeBits and the @p required flags, with a preference
			 * scoring against @p preferred. Returns VK_MAX_MEMORY_TYPES on failure.
			 */
			u32 PickMemoryTypeIndex(u32 typeBits, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const;

			/** Number of values EncodeMemoryType() can return. */
			static constexpr u32 kEncodedMemoryTypeCount = VK_MAX_MEMORY_TYPES * 2;

			/**
			 * Returns the value reported as GpuMemoryRequirements::MemoryType for a resource in the Vulkan memory type
			 * @p memoryTypeIndex. Optimally tiled images report a different value than buffers and linearly tiled images of the same
			 * Vulkan memory type, so the two use different persistent allocators and never share a persistent heap, and
			 * VkPhysicalDeviceLimits::bufferImageGranularity does not apply to them. Both values map to one transient heap pool, which
			 * rounds its placements to that granularity.
			 */
			static u32 EncodeMemoryType(u32 memoryTypeIndex, bool isOptimallyTiled) { return memoryTypeIndex * 2 + (isOptimallyTiled ? 1 : 0); }

			/** Returns the Vulkan memory type index of @p memoryType, a value returned by EncodeMemoryType(). */
			static u32 GetMemoryTypeIndex(u32 memoryType) { return memoryType / 2; }

			/** Returns true if @p memoryType, a value returned by EncodeMemoryType(), holds optimally tiled images. */
			static bool IsOptimallyTiled(u32 memoryType) { return memoryType % 2 != 0; }

			/**
			 * Returns the GPU memory allocator backing @p memoryType, a value returned by EncodeMemoryType(), lazily creating it on
			 * first use. Each allocator owns its own VkDeviceMemory heaps via the shared VulkanHeapBackend
			 * and is locked to a single memory-type index.
			 */
			TGpuTlsfAllocator<VulkanHeapBackend>& GetOrCreateGpuMemoryAllocator(u32 memoryType);

			/**
			 * Returns the shared linear page pool backing memory type @p memoryTypeIndex, lazily creating it on
			 * first use. Every GpuWorkContext's transient (linear) allocator for this memory type draws pages
			 * from (and returns drained pages to) this device-owned, thread-safe pool, bounding the number of
			 * VkDeviceMemory heaps under bursty transient allocation. See CreateScratchAllocator.
			 */
			TGpuLinearPagePool<VulkanHeapBackend>& GetOrCreateLinearPagePool(u32 memoryTypeIndex);

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
			bool mSupportsBindlessResources = false;
			bool mSupportsNullDescriptor = false; /**< True if VK_EXT_robustness2 nullDescriptor is enabled. */
			bool mIsBufferDeviceAddressEnabled = false; /**< True if the bufferDeviceAddress feature is enabled. Acceleration structures need it. */
			bool mSupportsAccelerationStructures = false; /**< True if the accelerationStructure feature is enabled. */
			bool mSupportsRayQuery = false; /**< True if the rayQuery feature is enabled. */
			bool mSupportsRayTracingPipeline = false; /**< True if the rayTracingPipeline feature is enabled. */

			VulkanDescriptorLayout* mBindlessHeapLayout = nullptr;
			TInlineArray<VkDescriptorType, 4> mBindlessResourceTypes; /**< See GetBindlessResourceTypes(). */

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				Vector<TShared<VulkanGpuQueue>> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			TInlineArray<u32, GQT_COUNT> mQueueFamilies;
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

			TUnique<VulkanHeapBackend> mHeapBackend;

			/** TLSF allocator of each value returned by EncodeMemoryType(). Slots are lazily populated on first allocation. */
			TUnique<TGpuTlsfAllocator<VulkanHeapBackend>> mGpuMemoryAllocators[kEncodedMemoryTypeCount];

			/** Guards lazy creation of mTlsfAllocators entries. */
			mutable Mutex mGpuMemoryAllocatorMutex;

			/** Per-memory-type shared page pool feeding every context's transient linear allocators for that type. Lazily populated. */
			TUnique<TGpuLinearPagePool<VulkanHeapBackend>> mLinearPagePools[VK_MAX_MEMORY_TYPES];

			/** Guards lazy creation of mLinearPagePools entries. */
			mutable Mutex mLinearPagePoolMutex;

			/** Transient heap pool of each Vulkan memory type, owned by GpuDevice. Valid for indices below the memory type count. */
			IGpuTransientHeapPool* mTransientHeapPools[VK_MAX_MEMORY_TYPES];
		};

		/** @} */
	} // namespace render
} // namespace b3d
