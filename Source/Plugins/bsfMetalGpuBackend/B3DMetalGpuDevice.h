//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"

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
		class MetalGpuDevice : public GpuDevice, private IGpuSubmitThreadBackend
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageMsl;

			MetalGpuDevice();
			~MetalGpuDevice();

			/** Returns the underlying MTLDevice. */
			id<MTLDevice> GetMetalDevice() const { return mMetalDevice; }

			/** Returns the Metal command queue used for the given queue type. */
			id<MTLCommandQueue> GetMetalQueue(GpuQueueType type) const
			{
				B3D_ASSERT((u32)type < GQT_COUNT);
				return mCommandQueues[(u32)type];
			}

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
			 * Returns the resolved timestamp counter set used by @c MetalGpuQueryPool to allocate
			 * @c MTLCounterSampleBuffer instances, or nil when the device did not advertise one at init
			 * time. A non-null set implies all command-boundary sampling points required by the generic
			 * timestamp-query contract are also supported.
			 */
			id<MTLCounterSet> GetTimestampCounterSet() const { return mTimestampCounterSet; }

			/**
			 * Returns the device-wide listener that blocking CPU waits register their notification
			 * blocks with (@c MetalGpuTimelineFence::WaitInternal, @c MetalGpuQueryPool::TryResolve).
			 * A listener is not bound to any particular event, so this single instance serves every
			 * @c MTLSharedEvent the backend creates. Its blocks are delivered on a dedicated concurrent
			 * dispatch queue rather than the main queue the default initializer would use, so a
			 * main-thread wait cannot deadlock on a notification that could only run on main.
			 * Created in @c Initialize; nil if creation failed, in which case callers poll instead.
			 */
			MTLSharedEventListener* GetSharedEventListener() const { return mSharedEventListener; }

			/**
			 * Returns the device-owned heap allocator that sub-allocates @c MTLTexture /
			 * @c MTLBuffer instances out of a pool of @c MTLHeap objects bucketed by storage mode.
			 * Backends that create raw Metal resources on hot paths (texture / buffer creation
			 * during streaming) should route through the allocator to avoid paying the per-resource
			 * driver-side allocation cost which dominates on Apple Silicon. Oversize allocations use
			 * dedicated placement heaps rather than fragmenting pooled heaps.
			 */
			MetalHeapAllocator& GetHeapAllocator() const { return *mHeapAllocator; }

			/**
			 * Returns the device-owned resource manager that mints and tracks the lifetime of the
			 * low-level GPU resource wrappers.
			 */
			MetalResourceManager& GetResourceManager() const { B3D_ASSERT(mResourceManager != nullptr); return *mResourceManager; }

			/**
			 * Returns the device-owned internal clear pipeline, used by
			 * @c MetalGpuCommandBuffer::ClearViewport to clear a sub-rect of the render target - Metal's
			 * only native clear is the render pass load action, which always covers a whole attachment.
			 */
			MetalClearPipeline& GetClearPipeline() const { B3D_ASSERT(mClearPipeline != nullptr); return *mClearPipeline; }

			/**
			 * Returns true while the device's GpuSubmitThread is alive (from the end of Initialize()
			 * to the start of destruction). Used by MetalGpuQueue::WaitUntilIdle to fall back to the
			 * native wait during construction / teardown windows.
			 */
			bool HasSubmitThread() const { return mSubmitThread != nullptr; }

			/**
			 * @name Per-encoder timestamp-sampling support flags
			 *
			 * @c sampleCountersInBuffer:atSampleIndex:withBarrier: on a render / compute / blit encoder
			 * requires the device to advertise the matching @c MTLCounterSamplingPoint (Draw / Dispatch
			 * / Blit boundary). Apple Silicon typically advertises only the stage boundary, so the
			 * per-encoder calls would fail validation there. The backend advertises timer queries only
			 * when all three flags are true.
			 *  @{
			 */
			bool SupportsRenderEncoderTimestamps() const { return mSupportsRenderEncoderTimestamps; }
			bool SupportsComputeEncoderTimestamps() const { return mSupportsComputeEncoderTimestamps; }
			bool SupportsBlitEncoderTimestamps() const { return mSupportsBlitEncoderTimestamps; }
			/** @} */

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
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

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

			void NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer) override;
			void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) override;
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, u64 lastFenceValue) override;
			u64 GetLastSubmittedFenceValue(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;

			/** Initializes capabilities by querying the underlying MTLDevice. */
			void InitializeCapabilities();

			bool mIsInitialized = false;
			id<MTLDevice> mMetalDevice = nil;
			id<MTLCommandQueue> mCommandQueues[GQT_COUNT] = { nil };
			id<MTLSharedEvent> mQueueEvents[GQT_COUNT] = { nil };

			// Device-wide MTLSharedEventListener and the dispatch queue its notification blocks run on
			dispatch_queue_t mListenerDispatchQueue = nullptr;
			MTLSharedEventListener* mSharedEventListener = nil;

			id<MTLBuffer> mNullVertexBuffer = nil;
			id<MTLBuffer> mDummyArgumentBuffer = nil;

			id<MTLCounterSet> mTimestampCounterSet = nil;

			bool mSupportsRenderEncoderTimestamps = false;
			bool mSupportsComputeEncoderTimestamps = false;
			bool mSupportsBlitEncoderTimestamps = false;

			// First CPU/GPU timestamp pair captured at device init
			MTLTimestamp mFirstCpuTimestamp = 0;
			MTLTimestamp mFirstGpuTimestamp = 0;
			bool mFirstTimestampPairCaptured = false;
			std::atomic<bool> mTimestampCalibrationDone{ false };
			Mutex mTimestampCalibrationMutex;

			TUnique<MetalHeapAllocator> mHeapAllocator;

			TUnique<MetalResourceManager> mResourceManager;
			TUnique<MetalClearPipeline> mClearPipeline;

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

			// CPU / GPU timestamp calibration captured once at init via [MTLDevice sampleTimestamps:]. Used
			// by ConvertTimestampToMilliseconds to translate raw GPU ticks into engine-wallclock time. Zero
			// on devices that do not advertise RSC_TIMER_QUERIES.
			u64 mCpuBaseTimestamp = 0;
			u64 mGpuBaseTimestamp = 0;
			double mGpuTicksPerNanosecond = 0.0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
