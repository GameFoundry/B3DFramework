//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12HeapBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

#if B3D_BUILD_TYPE_DEVELOPMENT
#include <atomic>
#include <thread>
#endif

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

			/** Creates a D3D12 device for @p adapter. */
			D3D12GpuDevice(IDXGIAdapter4* adapter);
			~D3D12GpuDevice() override;

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
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			TShared<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, IGpuAllocator& allocator, GpuObjectCreateFlags flags) override;
			u32 PickBufferMemoryType(const GpuBufferCreateInformation& createInformation) const override;
			TUnique<IGpuAllocator> CreateTransientAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			u32 GetUniformBufferParameterSlot(u32 registerIndex) const override;
			TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
			TShared<GpuTimelineFence> CreateTimelineFence() override;

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& outMatrix) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override;

			/** @} */

			/** Returns the D3D12 device object. */
			ID3D12Device* GetD3D12Device() const { return mDevice.Get(); }

			/** Returns the interface used for enhanced-layout resource creation. */
			ID3D12Device10* GetD3D12Device10() const { return mEnhancedDevice.Get(); }

			/** Returns true if the device is the primary GPU. */
			bool IsPrimary() const { return mIsPrimary; }

			/** Returns the descriptor manager that can be used for allocating descriptors. */
			D3D12DescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that owns the lifetime of manager-allocated D3D12 GPU resources. */
			D3D12ResourceManager& GetResourceManager() const { return *mResourceManager; }

			/** Returns true if the submit thread is currently running. */
			bool HasSubmitThread() const { return mSubmitThread != nullptr; }

			/**
			 * Creates a resource using the matching heap pool. When valid, @p outAllocation owns the suballocation and 
			 * must be released with FreeMemory after the native resource is destroyed.
			 */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuResourceLocation& outAllocation);

			/** Releases a TLSF suballocation. The native resource using it must already have been destroyed. */
			void FreeMemory(GpuResourceLocation& allocation);

			/**
			 * Drains any warnings/errors stored in the D3D12 debug layer's info queue into the engine log. No-op
			 * when the debug layer is disabled. Thread safe (the info queue is internally synchronized).
			 */
			void LogDebugLayerMessages();

			/**
			 * Logs DRED (Device Removed Extended Data) auto-breadcrumbs after a device removal: which command list
			 * hung, at which command, plus a page-fault GPU VA when one was recorded. Requires DRED to have been
			 * enabled before device creation.
			 */
			void LogDeviceRemovalBreadcrumbs();

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
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, u32 lastSubmitIndex) override;
			u32 GetLastSubmitIndex(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			enum class MemoryPoolType : u32
			{
				DefaultBuffer,      /**< GPU-local buffers. */
				DefaultTexture,     /**< GPU-local, non-multisampled textures. */
				DefaultMsaaTexture, /**< GPU-local multisampled textures. */
				UploadBuffer,       /**< CPU-writable upload buffers. */
				ReadbackBuffer,     /**< CPU-readable readback buffers. */
				Count               /**< Number of allocatable memory pool types. */
			};

			using GpuMemoryAllocator = TGpuTlsfAllocator<D3D12HeapBackend>;

			/** Returns the heap pool compatible with the specified resource, or Count when unsupported. */
			static MemoryPoolType GetMemoryPoolType(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType);

			/** Lazily creates and returns the allocator for a compatible D3D12 heap class. */
			GpuMemoryAllocator& GetOrCreateGpuMemoryAllocator(MemoryPoolType poolType);

			ComPtr<ID3D12Device> mDevice;
			ComPtr<ID3D12Device10> mEnhancedDevice;
			ComPtr<IDXGIAdapter4> mAdapter;
			bool mIsPrimary = false;

			D3D12DescriptorManager* mDescriptorManager = nullptr;
			D3D12ResourceManager* mResourceManager = nullptr;
			TUnique<D3D12HeapBackend> mHeapBackend;
			TUnique<D3D12BufferPool> mBufferPool;
			TUnique<GpuMemoryAllocator> mGpuMemoryAllocators[(u32)MemoryPoolType::Count];
			Mutex mGpuMemoryAllocatorMutex;
			u64 mTimestampFrequency = 0;
			bool mLoggedDeviceRemoval = false; /**< Ensures DRED breadcrumbs are logged only once per removal. */

#if B3D_BUILD_TYPE_DEVELOPMENT
			std::thread mDeviceRemovalWatchdog; /**< Polls for device removal and dumps the DRED breadcrumbs. */
			std::atomic<bool> mWatchdogShouldExit{false};
#endif

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				Vector<TShared<D3D12GpuQueue>> Queues; /**< Queues exposed for one queue type. */
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

		};

		/** @} */
	} // namespace render
} // namespace b3d
