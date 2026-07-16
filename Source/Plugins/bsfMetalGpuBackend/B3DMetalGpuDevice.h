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

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Represents a single Metal GPU device.
		 *
		 * Wraps a real @c MTLDevice and owns one @c MTLCommandQueue per GpuQueueType. The Metal
		 * handles are stored in an opaque @c Impl struct so that non-Objective-C++ translation units
		 * can include this header without dragging in the Metal framework.
		 */
		class MetalGpuDevice : public GpuDevice, private IGpuSubmitThreadBackend
		{
		public:
			/**
			 * The engine's BSL compiler emits shader source for the native Metal backend in the
			 * @c msl form: Vulkan-flavored GLSL with a @c METAL define. The Metal backend runs that
			 * source through glslang (SPIR-V) and then SPIRV-Cross (MSL) before compiling the resulting
			 * MSL into an @c MTLLibrary. The canonical literal lives in @c B3DGpuBackend.h.
			 */
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageMsl;

			MetalGpuDevice();
			~MetalGpuDevice();

#ifdef __OBJC__
			/** Returns the underlying MTLDevice. Only available to Objective-C++ translation units. */
			id<MTLDevice> GetMetalDevice() const;

			/** Returns the Metal command queue used for the given queue type. */
			id<MTLCommandQueue> GetMetalQueue(GpuQueueType type) const;

			/**
			 * Returns the persistent zero-filled MTLBuffer bound at a vertex-input null stream's slot for
			 * shader inputs that have no matching vertex-buffer element. Created once at Initialize()
			 * (@c kMetalNullVertexStreamStride bytes, @c MTLResourceStorageModeShared, contents memset to
			 * zero) and released at teardown; the command buffer binds it via
			 * @c MetalGpuCommandBuffer::ResolveVertexInputForDraw. Never nil once the device is
			 * initialized on a valid MTLDevice.
			 */
			id<MTLBuffer> GetNullVertexBuffer() const;

			/**
			 * Queues an Obj-C Metal object (@c MTLTexture / @c MTLBuffer / ...) for release once
			 * @p originQueue has committed at least @p eventValue on its shared event. Drained on the
			 * next @c BeginFrame. Used by resource recreate paths to keep the prior backing alive until
			 * any in-flight command buffers that reference it have retired, without blocking the caller.
			 *
			 * @note	The watermark is currently the graphics queue's last-committed value; if a
			 *			texture / buffer is referenced only on a compute or transfer queue, the release
			 *			may over-retain by one frame. TODO: tighten by tracking the resource's actual
			 *			last-submit queue + value once per-resource submit tracking lands.
			 */
			void QueueMetalResourceForDeferredRelease(id resource, MetalGpuQueue* originQueue, u64 eventValue);

			/**
			 * Returns the resolved timestamp counter set used by @c MetalGpuQueryPool to allocate
			 * @c MTLCounterSampleBuffer instances, or nil when the device did not advertise one at init
			 * time. A non-null set implies all command-boundary sampling points required by the generic
			 * timestamp-query contract are also supported.
			 */
			id<MTLCounterSet> GetTimestampCounterSet() const;

			/**
			 * Returns the read-only MTLBinaryArchive loaded from the on-disk pipeline cache, or nil when
			 * nothing was loaded (first launch, load failure, or an incompatible archive).
			 *
			 * URL-loaded archives are immutable and are used only as pipeline lookup sources. Populating
			 * an archive is an offline/prewarm responsibility because runtime insertion recompiles a PSO.
			 *
			 * The archives are created lazily on the first call (one-shot init under an internal mutex)
			 * so device @c Initialize performs zero filesystem I/O for the pipeline cache; the cost is
			 * only paid when the first PSO actually compiles.
			 */
			id<MTLBinaryArchive> GetLoadedBinaryArchive();
#endif

			/**
			 * Returns the device-owned heap allocator that sub-allocates @c MTLTexture /
			 * @c MTLBuffer instances out of a pool of @c MTLHeap objects bucketed by storage mode.
			 * Backends that create raw Metal resources on hot paths (texture / buffer creation
			 * during streaming) should route through the allocator to avoid paying the per-resource
			 * driver-side allocation cost which dominates on Apple Silicon. Oversize allocations use
			 * dedicated placement heaps rather than fragmenting pooled heaps.
			 * Remains valid for the lifetime of the device — created in @c Initialize and torn down
			 * after the deferred-release queue has been drained in the destructor.
			 */
			MetalHeapAllocator& GetHeapAllocator() const { return *mHeapAllocator; }

			/**
			 * Returns the device-owned resource manager that mints and tracks the lifetime of the
			 * low-level tracked wrappers (@c MetalBuffer / @c MetalImage / @c MetalImageSubresource).
			 * Wrappers are created via @c GetResourceManager().Create<T>(...) and retired via
			 * @c IGpuResource::Destroy(); the manager defers the actual free until every command
			 * buffer referencing the wrapper has retired. Created in @c Initialize (after the heap
			 * allocator) and torn down in the destructor before @c mHeapAllocator.reset() so
			 * heap-backed wrappers release their spans before the heaps are destroyed.
			 */
			MetalResourceManager& GetResourceManager() const { B3D_ASSERT(mResourceManager != nullptr); return *mResourceManager; }

			/**
			 * Returns @c true once @c ~MetalGpuDevice has started executing. Late resource destructors
			 * (e.g. @c ~MetalTexture / @c ~MetalGpuBuffer triggered by engine-thread teardown that
			 * cascades through the device lifetime) consult this flag to take the synchronous release
			 * path instead of appending to the deferred-release list — queuing after the device's
			 * drain / heap-allocator teardown would leave dangling parent-heap refs and outlive the
			 * list itself. Never clears; the device is going away.
			 */
			bool IsShuttingDown() const { return mIsShuttingDown; }

			/**
			 * Returns true while the device's GpuSubmitThread is alive (from the end of Initialize()
			 * to the start of destruction). Used by MetalGpuQueue::WaitUntilIdle to fall back to the
			 * native wait during construction / teardown windows, mirroring the null-check inside
			 * VulkanGpuDevice::WaitUntilIdle.
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
			bool SupportsRenderEncoderTimestamps() const;
			bool SupportsComputeEncoderTimestamps() const;
			bool SupportsBlitEncoderTimestamps() const;
			/** @} */

			/**
			 * Returns the maximum anisotropic-filtering ratio supported by the underlying @c MTLDevice.
			 * All current Metal GPU families support up to 16; the value is cached at initialization time
			 * so callers do not have to re-probe feature sets.
			 */
			u32 GetMaxSamplerAnisotropy() const { return mMaxSamplerAnisotropy; }

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
			TShared<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation,
				IGpuAllocator& allocator, GpuObjectCreateFlags flags) override;
			u32 PickBufferMemoryType(const GpuBufferCreateInformation& createInformation) const override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;
			TUnique<IGpuAllocator> CreateTransientAllocator(u32 memoryType,
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
			 *
			 * Implemented privately, mirroring VulkanGpuDevice: the GpuSubmitThread constructed at the
			 * end of Initialize() is the only caller. Command buffer methods downcast to the Metal
			 * types; queue methods forward to MetalGpuQueue's submit-thread-facing half.
			 * @{
			 */

			void NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer) override;
			void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) override;
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, bool queueEmpty, u32 lastSubmitIndex) override;
			u32 GetLastSubmitIndex(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes capabilities by querying the underlying MTLDevice. */
			void InitializeCapabilities();

#ifdef __OBJC__
			/**
			 * Lazy one-shot construction of the loaded pipeline binary archive. The first call resolves
			 * the on-disk archive path, creates the caches folder if necessary, and loads an existing
			 * immutable archive. Subsequent calls are cheap guarded reads. Kept private so only the
			 * archive accessor triggers it.
			 */
			void EnsureBinaryArchives();
#endif

			/** Pimpl that holds Objective-C / Metal handles so they do not leak into plain C++ headers. */
			struct Impl;

			bool mIsInitialized = false;
			// Flipped to @c true at the top of @c ~MetalGpuDevice before any deferred-release drain or
			// @c mHeapAllocator.reset() runs. Consulted by @c IsShuttingDown so resource destructors
			// reached during device teardown skip deferred-release queuing and release their backing
			// Metal handles synchronously. Never cleared.
			bool mIsShuttingDown = false;
			TUnique<Impl> mImpl;
			TUnique<MetalHeapAllocator> mHeapAllocator;

			// Owns the lifetime of the Metal-side tracked IGpuResource wrappers minted by
			// GetResourceManager().Create<T>. Constructed after mHeapAllocator in Initialize and
			// reset before mHeapAllocator in teardown so heap-backed wrappers free their allocator
			// spans before the heaps that back them are destroyed.
			TUnique<MetalResourceManager> mResourceManager;

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;
			u32 mMaxSamplerAnisotropy = 1;

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
