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
		class MetalGpuDevice : public GpuDevice
		{
		public:
			/**
			 * The engine's BSL compiler emits shader source for the Metal backend in the @c mvksl
			 * form: Vulkan-flavored GLSL with a @c METAL define, matching what MoltenVK accepts. The
			 * Metal backend runs that source through glslang (SPIR-V) and then SPIRV-Cross (MSL)
			 * before compiling the resulting MSL into an @c MTLLibrary. The canonical literal lives
			 * in @c B3DGpuBackend.h.
			 */
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageMvksl;

			MetalGpuDevice();
			~MetalGpuDevice();

#ifdef __OBJC__
			/** Returns the underlying MTLDevice. Only available to Objective-C++ translation units. */
			id<MTLDevice> GetMetalDevice() const;

			/** Returns the Metal command queue used for the given queue type. */
			id<MTLCommandQueue> GetMetalQueue(GpuQueueType type) const;

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
			 * time. Used by the query pool to gate timestamp-pool creation.
			 */
			id<MTLCounterSet> GetTimestampCounterSet() const;

			/**
			 * Returns the read-only MTLBinaryArchive loaded from the on-disk pipeline cache, or nil when
			 * nothing was loaded (first launch, iOS simulator, load failure, pre-macOS 11 / pre-iOS 14).
			 *
			 * URL-loaded binary archives are immutable: you can use them as a lookup source in a pipeline
			 * descriptor's @c binaryArchives but you cannot @c addRenderPipelineFunctionsWithDescriptor:
			 * to them. Pipeline state objects therefore attach *both* this archive (for fast-path lookup
			 * of pipelines compiled in a previous run) and @c GetMutableBinaryArchive() (to receive this
			 * session's fresh compiles).
			 *
			 * The archives are created lazily on the first call (one-shot init under an internal mutex)
			 * so device @c Initialize performs zero filesystem I/O for the pipeline cache; the cost is
			 * only paid when the first PSO actually compiles.
			 */
			id<MTLBinaryArchive> GetLoadedBinaryArchive();

			/**
			 * Returns the writable MTLBinaryArchive that receives this session's freshly-compiled
			 * pipelines, or nil when the platform / runtime does not support binary archives.
			 *
			 * Pipeline state objects add compiled variants back into this archive via
			 * @c addRenderPipelineFunctionsWithDescriptor: / @c addComputePipelineFunctionsWithDescriptor:
			 * so the next launch picks them up. Serialized to disk in the destructor; the loaded archive
			 * is *not* serialized (it already exists on disk and URL-loaded archives are immutable).
			 *
			 * The archives are created lazily on the first call — see @c GetLoadedBinaryArchive for
			 * rationale.
			 */
			id<MTLBinaryArchive> GetMutableBinaryArchive();
#endif

			/**
			 * Returns the device-owned heap allocator that sub-allocates @c MTLTexture /
			 * @c MTLBuffer instances out of a pool of @c MTLHeap objects bucketed by storage mode.
			 * Backends that create raw Metal resources on hot paths (texture / buffer creation
			 * during streaming) should route through the allocator to avoid paying the per-resource
			 * driver-side allocation cost which dominates on Apple Silicon. The allocator falls back
			 * to direct device allocation for resources above its internal large-resource threshold.
			 * Remains valid for the lifetime of the device — created in @c Initialize and torn down
			 * after the deferred-release queue has been drained in the destructor.
			 */
			MetalHeapAllocator& GetHeapAllocator() const { return *mHeapAllocator; }

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
			 * @name Per-encoder timestamp-sampling support flags
			 *
			 * @c sampleCountersInBuffer:atSampleIndex:withBarrier: on a render / compute / blit encoder
			 * requires the device to advertise the matching @c MTLCounterSamplingPoint (Draw / Dispatch
			 * / Blit boundary). Apple Silicon typically advertises only the stage boundary, so the
			 * per-encoder calls would fail validation there. These flags are probed once at init and
			 * consulted by @c MetalGpuCommandBuffer::WriteTimestamp to pick a supported encoder (or
			 * drop the sample with a warn-once log if none qualify).
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
			SPtr<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const override;

			u32 GetQueueCount(GpuQueueType type) const override;
			SPtr<GpuQueue> GetQueue(GpuQueueType type, u32 index) const override;
			void SubmitTransferCommandBuffers(bool wait = false) override;
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

			SPtr<render::GpuCommandBufferPool> CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation) override;
			SPtr<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			SPtr<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			SPtr<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			SPtr<EventQuery> CreateEventQuery() override;
			SPtr<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			SPtr<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			SPtr<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			SPtr<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			SPtr<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription) override;
			UPtr<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			SPtr<GpuTimelineFence> CreateTimelineFence() override;

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override;

			/** @} */

		private:
			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				TArray<SPtr<GpuQueue>> Queues;
			};

			SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes capabilities by querying the underlying MTLDevice. */
			void InitializeCapabilities();

#ifdef __OBJC__
			/**
			 * Lazy one-shot construction of the loaded / mutable pipeline binary archives. The first
			 * call resolves the on-disk archive path, creates the caches folder if necessary, loads any
			 * existing archive (immutable) and always creates a fresh empty mutable archive for this
			 * session. Subsequent calls are cheap guarded reads. Kept private so only the archive
			 * accessors trigger it.
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
			UPtr<Impl> mImpl;
			UPtr<MetalHeapAllocator> mHeapAllocator;
			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			SPtr<VideoModeInfo> mVideoModeInfo;
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
