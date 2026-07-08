//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"

namespace D3D12MA
{
	class Allocator;
	class Allocation;
}

namespace b3d
{
	class D3D12GpuBackend;

	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Represents a single GPU device usable by DirectX 12. */
		class D3D12GpuDevice : public GpuDevice, private IGpuSubmitThreadBackend
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageHlsl;

			D3D12GpuDevice(IDXGIAdapter4* adapter);
			~D3D12GpuDevice();

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
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			TShared<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override;

			/** @} */

			/** Returns the D3D12 device object. */
			ID3D12Device* GetD3D12Device() const { return mDevice.Get(); }

			/** Returns the DXGI adapter. */
			IDXGIAdapter4* GetDXGIAdapter() const { return mAdapter.Get(); }

			/** Returns true if the device is the primary GPU. */
			bool IsPrimary() const { return mIsPrimary; }

			/** Returns the descriptor manager that can be used for allocating descriptors. */
			D3D12DescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that owns the lifetime of manager-allocated D3D12 GPU resources. */
			D3D12ResourceManager& GetResourceManager() const { return *mResourceManager; }

			/** Returns the memory allocator for creating GPU resources. */
			D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

			/** Returns true if the submit thread is currently running. */
			bool HasSubmitThread() const { return mSubmitThread != nullptr; }

			/**
			 * Queues a native object (and optionally its D3D12MA allocation) for release once the GPU can no longer
			 * be referencing it. Deferred objects are released two frame boundaries later (matching the submit
			 * thread's two-frames-in-flight pacing), or when the device goes idle.
			 *
			 * Thread safe.
			 */
			void DeferNativeRelease(ComPtr<IUnknown> object, D3D12MA::Allocation* allocation = nullptr);

			/**
			 * Drains any warnings/errors stored in the D3D12 debug layer's info queue into the engine log. No-op
			 * when the debug layer is disabled. Thread safe (the info queue is internally synchronized).
			 */
			void LogDebugLayerMessages();

			/**
			 * Render-thread work context used internally by this backend's texture/buffer read/write
			 * paths, created lazily on first use. Borrows the device's frame completion tracker.
			 *
			 * TODO: Remove once the engine upload/readback surface threads a caller-provided
			 *		 GpuWorkContext through these paths (this backend is currently non-functional).
			 */
			GpuWorkContext& GetInternalWorkContext();

			/**
			 * Releases the internal work context, submitting and waiting on any of its outstanding transfer work. Must
			 * be executed on the render thread (which the context's command buffer pools are bound to), while the
			 * submit thread is still running.
			 */
			void ReleaseInternalWorkContext() { mInternalWorkContext = nullptr; }

			/** Returns the GPU timestamp frequency for this device. */
			u64 GetTimestampFrequency() const { return mTimestampFrequency; }

		private:
			friend class b3d::D3D12GpuBackend;

			/**
			 * @name IGpuSubmitThreadBackend implementation
			 * @{
			 */

			void NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer) override;
			void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) override;
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, bool queueEmpty, u32 lastSubmitIndex) override;
			u32 GetLastSubmitIndex(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			/**
			 * Starts the submit thread that all queue submission and present operations are routed through. Requires
			 * the device queues and the task scheduler to be available. Only started for the primary device.
			 */
			void StartSubmitThread();

			/** Stops the submit thread. All GPU work must have completed before calling this. */
			void StopSubmitThread();

			/** Releases every deferred native object whose safety window has passed. See DeferNativeRelease(). */
			void DrainDeferredReleases(bool releaseAll);

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			ComPtr<ID3D12Device> mDevice;
			ComPtr<IDXGIAdapter4> mAdapter;
			bool mIsPrimary = false;

			D3D12DescriptorManager* mDescriptorManager = nullptr;
			D3D12ResourceManager* mResourceManager = nullptr;
			D3D12MA::Allocator* mAllocator = nullptr;
			u64 mTimestampFrequency = 0;
			TShared<GpuWorkContext> mInternalWorkContext; /**< See GetInternalWorkContext(). */

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				Vector<TShared<D3D12GpuQueue>> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

			/** A native object (and optionally its allocation) whose release has been deferred. See DeferNativeRelease(). */
			struct DeferredRelease
			{
				ComPtr<IUnknown> Object;
				D3D12MA::Allocation* Allocation = nullptr;
			};

			/**
			 * Ring of deferred-release lists: one open list objects are appended to, plus two lists aging out
			 * (covering the submit thread's two frames in flight). Advanced at every EndFrame().
			 */
			static constexpr u32 kDeferredReleaseFrameCount = 3;
			Vector<DeferredRelease> mDeferredReleases[kDeferredReleaseFrameCount];
			u32 mCurrentDeferredReleaseFrame = 0;
			Mutex mDeferredReleaseMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
