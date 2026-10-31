//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "Allocators/B3DRangeAllocator.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuQueue;
		class MetalHeapAllocator;
		class MetalResourceManager;
		class MetalClearPipeline;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Represents a single Metal GPU device. */
		class MetalGpuDevice : public GpuDevice, private IGpuSubmitThreadBackend, private IGpuBindlessHeap
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageMsl;

			/** Number of 8-byte slots of the visibility result buffer. Apple GPUs limit visibility result offsets to 256 KB. */
			static constexpr u32 kVisibilityResultSlotCount = 32 * 1024;

			/**
			 * Number of slots of each timestamp counter heap, the maximum Metal allows. Metal also limits a process to 32 live counter
			 * heaps, so timestamp query pools share them rather than each having its own.
			 */
			static constexpr u32 kCounterHeapSlotCount = 4096;

			MetalGpuDevice();
			~MetalGpuDevice();

			/** Returns the underlying MTLDevice. */
			id<MTLDevice> GetMetalDevice() const { return mMetalDevice; }

			/**
			 * Returns the persistent zero-filled MTLBuffer bound at a vertex-input null stream's slot for
			 * shader inputs that have no matching vertex-buffer element. 
			 */
			id<MTLBuffer> GetNullVertexBuffer() const { return mNullVertexBuffer; }

			/**
			 * Returns the persistent zero-filled MTLBuffer whose GPU address pre-fills unbound buffer
			 * slots in every parameter set's argument buffer, so shaders that read a slot the engine
			 * never bound load zeroes instead of faulting.
			 */
			id<MTLBuffer> GetDummyArgumentBuffer() const { return mDummyArgumentBuffer; }

			/**
			 * Returns the device-wide listener that blocking CPU waits register their notification blocks with. It is not bound
			 * to any particular event, so it serves every @c MTLSharedEvent the backend creates. Its blocks run on a dedicated
			 * concurrent dispatch queue rather than the main queue, so a main-thread wait cannot deadlock on a notification
			 * that could only run on main.
			 */
			MTLSharedEventListener* GetSharedEventListener() const { return mSharedEventListener; }

			/**
			 * Returns the listener that reports submission completion. Its blocks run in order on a serial dispatch queue, and
			 * must not block on GPU work.
			 */
			MTLSharedEventListener* GetCompletionListener() const { return mCompletionListener; }

			/**
			 * Returns the heap allocator that every buffer and texture the GPU accesses must be allocated through. Its residency
			 * set, attached to every queue of the device, is what makes their memory resident.
			 */
			MetalHeapAllocator& GetHeapAllocator() const { return *mHeapAllocator; }

			/**
			 * Returns the device-owned resource manager that mints and tracks the lifetime of the
			 * low-level GPU resource wrappers.
			 */
			MetalResourceManager& GetResourceManager() const { B3D_ASSERT(mResourceManager != nullptr); return *mResourceManager; }

			/**
			 * Returns the device-owned internal clear pipeline, which @c MetalGpuCommandBuffer::ClearRenderTarget and
			 * @c MetalGpuCommandBuffer::ClearViewport draw with. Metal's only native clear, the render pass load action, needs a new
			 * render encoder.
			 */
			MetalClearPipeline& GetClearPipeline() const { B3D_ASSERT(mClearPipeline != nullptr); return *mClearPipeline; }

			/**
			 * Returns the buffer the occlusion queries of every render pass write their visibility results into. Occlusion query pools own
			 * ranges of its slots, so a render encoder created with it can run the queries of any pool.
			 */
			id<MTLBuffer> GetVisibilityResultBuffer() const { return mVisibilityResultBuffer; }

			/**
			 * Returns the buffer of bindless heap @p heap, an array of 8-byte entries that shaders read as an argument buffer: the
			 * resource heap binds at buffer index kGpuBindlessParameterSet, and the sampler heap at kMetalSamplerHeapBufferIndex.
			 */
			id<MTLBuffer> GetBindlessHeapBuffer(GpuBindlessHeapType heap) const { return mBindlessHeapBuffers[(u32)heap]; }

			/**
			 * Reserves @p count consecutive slots of the visibility result buffer, and returns the first, or RangeAllocator::kInvalidIndex
			 * if no free range is large enough. Thread safe.
			 */
			u32 AllocateVisibilityResultSlots(u32 count);

			/** Releases slots reserved by AllocateVisibilityResultSlots(). Thread safe. */
			void FreeVisibilityResultSlots(u32 firstSlot, u32 count);

			/**
			 * Reserves @p count consecutive slots of a timestamp counter heap, creating a new heap if no existing one has enough free
			 * slots. Returns the first slot and outputs its heap in @p outCounterHeap, or returns RangeAllocator::kInvalidIndex if the
			 * slots cannot be reserved. Thread safe.
			 */
			u32 AllocateCounterHeapSlots(u32 count, __strong id<MTL4CounterHeap>& outCounterHeap);

			/** Releases slots reserved by AllocateCounterHeapSlots(). Thread safe. */
			void FreeCounterHeapSlots(id<MTL4CounterHeap> counterHeap, u32 firstSlot, u32 count);

			/**
			 * Returns true while the device's GpuSubmitThread is alive (from the end of Initialize()
			 * to the start of destruction). Used by MetalGpuQueue::WaitUntilIdle to fall back to the
			 * native wait during construction / teardown windows.
			 */
			bool HasSubmitThread() const { return mSubmitThread != nullptr; }

			/**
			 * @name GpuDevice Interface
			 *  @{
			 */

			bool IsInitialized() const override { return mIsInitialized; }
			bool Initialize() override;

			const GpuDeviceCapabilities& GetCapabilities() const override { return mCapabilities; }
			const VideoModeInfo& GetVideoModeInfo() const override { return *mVideoModeInfo; }

			bool IsGpuProgramLanguageSupported(const StringView& language) const override { return language == kGpuProgramLanguageName; }

			u32 GetQueueCount(GpuQueueType type) const override;
			TShared<GpuQueue> GetQueue(GpuQueueType type, u32 index) const override;
			void WaitUntilIdle() override;
			void BeginFrame() override;

			TShared<render::GpuCommandBufferPool> CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation) override;
			GpuMemoryRequirements GetMemoryRequirements(const TextureCreateInformation& createInformation) const override;
			GpuMemoryRequirements GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const override;
			IGpuAllocator& GetPersistentAllocator(u32 memoryType) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;
			TUnique<IGpuAllocator> CreateScratchAllocator(u32 memoryType,
				IGpuCompletionTracker& completionTracker) override;
			IGpuTransientHeapPool* GetTransientHeapPool(u32 memoryType) override;

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override;

			/** @} */

		private:
			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				TArray<TShared<GpuQueue>> Queues;
			};

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

			/**
			 * @name IGpuBindlessHeap implementation
			 * @{
			 */

			void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) override;
			void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) override;
			void ClearSlot(GpuBindlessHeapType heap, u32 slotIndex) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			GpuVertexInput* CreateVertexInput(u32 id, const GpuVertexInputLayout& layout) override;

			/** Initializes capabilities by querying the underlying MTLDevice. */
			void InitializeCapabilities();

			bool mIsInitialized = false;
			id<MTLDevice> mMetalDevice = nil;

			// Device-wide MTLSharedEventListener and the dispatch queue its notification blocks run on
			dispatch_queue_t mListenerDispatchQueue = nullptr;
			MTLSharedEventListener* mSharedEventListener = nil;

			// Serial listener reporting submission completion
			dispatch_queue_t mCompletionDispatchQueue = nullptr;
			MTLSharedEventListener* mCompletionListener = nil;

			id<MTLBuffer> mNullVertexBuffer = nil;
			GpuAllocation mNullVertexBufferAllocation;
			id<MTLBuffer> mDummyArgumentBuffer = nil;
			GpuAllocation mDummyArgumentBufferAllocation;
			id<MTLBuffer> mVisibilityResultBuffer = nil;
			GpuAllocation mVisibilityResultBufferAllocation;
			id<MTLBuffer> mBindlessHeapBuffers[(u32)GpuBindlessHeapType::Count] = {};
			GpuAllocation mBindlessHeapBufferAllocations[(u32)GpuBindlessHeapType::Count];

			/** Timestamp counter heap shared by query pools, and its unallocated slots. */
			struct CounterHeap
			{
				id<MTL4CounterHeap> Heap = nil;
				RangeAllocator Slots{ kCounterHeapSlotCount };
			};

			RangeAllocator mVisibilityResultSlots{ kVisibilityResultSlotCount };
			TArray<CounterHeap> mCounterHeaps;
			Mutex mQueryAllocationMutex; /**< Guards mVisibilityResultSlots and mCounterHeaps. */

			/** Frequency of GPU timestamps, in ticks per second. */
			u64 mTimestampFrequency = 0;

			TUnique<MetalHeapAllocator> mHeapAllocator;

			/** Transient heap pool of the private memory type, owned by GpuDevice. */
			IGpuTransientHeapPool* mTransientHeapPool = nullptr;

			TUnique<MetalResourceManager> mResourceManager;
			TUnique<MetalClearPipeline> mClearPipeline;

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;
		};

		/** @} */
	} // namespace render
} // namespace b3d
