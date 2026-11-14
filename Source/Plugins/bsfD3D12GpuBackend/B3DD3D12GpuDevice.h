//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12HeapBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
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
			void WaitUntilIdle() override;
			void BeginFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			GpuMemoryRequirements GetMemoryRequirements(const TextureCreateInformation& createInformation) const override;
			GpuMemoryRequirements GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const override;
			IGpuAllocator& GetPersistentAllocator(u32 memoryType) override;
			void GetPersistentAllocators(TArray<IGpuAllocator*>& outAllocators) override;
			TUnique<IGpuAllocator> CreateScratchAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker) override;
			IGpuTransientHeapPool* GetTransientHeapPool(u32 memoryType) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex) override;
			u32 GetParameterSlot(ShaderRegisterClass registerClass, u32 registerIndex) const override;
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

			/** Returns the adapter of the device. */
			IDXGIAdapter4* GetDXGIAdapter() const { return mAdapter.Get(); }

			/** Returns the descriptor manager that can be used for allocating descriptors. */
			D3D12DescriptorManager& GetDescriptorManager() const { return *mDescriptorManager; }

			/** Returns a manager that owns the lifetime of manager-allocated D3D12 GPU resources. */
			D3D12ResourceManager& GetResourceManager() const { return *mResourceManager; }

			/**
			 * Returns true if buffers use tight alignment. Without it, every buffer is at least 64 KB in size and aligned to 64 KB, since each
			 * buffer is a placed resource of its own.
			 */
			bool IsTightAlignmentSupported() const { return mIsTightAlignmentSupported; }

			/**
			 * Returns the class of heaps a buffer of @p type, @p memory and @p flags is placed in, following GpuBackendUtility::GetBufferPlacement().
			 * Mapped GPU memory is the memory of a GPU upload heap: GPU memory with Resizable BAR, and system memory on unified memory. Mapped
			 * system memory and staging buffers are in the memory of upload and readback heaps, which unlike those heaps GPU copies can both
			 * read and write. Once GPU memory is close to its budget, mapped GPU memory holding GpuBufferMemory::Device buffers falls back to
			 * default memory, and mapped GPU memory holding GpuBufferMemory::Mapped buffers to the memory of an upload heap (see
			 * D3D12HeapBackend::CreateHeap()), so the memory of a buffer can differ.
			 */
			D3D12HeapClass GetBufferHeapClass(GpuBufferType type, GpuBufferMemory memory, GpuBufferFlags flags) const;

			/** Returns the class of heaps a texture described by @p resourceDesc is placed in. */
			static D3D12HeapClass GetTextureHeapClass(const D3D12_RESOURCE_DESC& resourceDesc);

			/**
			 * Returns true if the CPU can write the GPU's memory directly while the GPU still reads it at full speed: GPU upload heaps on
			 * discrete GPUs (Resizable BAR), cache-coherent unified memory (or any unified memory, with gpu.ForceDirectGpuBufferWrites).
			 */
			bool HasFullSpeedMappableGpuMemory() const { return mHasFullSpeedMappableGpuMemory; }

			/**
			 * Creates a resource in a heap of @p heapClass. When valid, @p outAllocation owns the suballocation and must be released with
			 * FreeMemory after the native resource is destroyed.
			 */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, D3D12HeapClass heapClass, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuAllocation& outAllocation);

			/**
			 * Creates a resource placed at @p requestedAllocation. A pending allocation suballocates from its allocator, which must
			 * serve the resource's heap pool. An allocation with memory is used as is. @p outAllocation receives the memory
			 * the resource is placed at, and must be released with FreeMemory after the native resource is destroyed.
			 */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, const GpuAllocation& requestedAllocation, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuAllocation& outAllocation);

			/** Releases a suballocation if it is owned. The native resource using it must already have been destroyed. */
			void FreeMemory(GpuAllocation& allocation);

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

			void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) override;
			void RefreshCompletionState(GpuQueue& queue, bool forceWait, u64 lastFenceValue) override;
			u64 GetLastSubmittedFenceValue(const GpuQueue& queue) const override;
			void ExecuteWaitUntilIdle() override;
			void ExecuteWaitUntilIdle(GpuQueue& queue) override;

			/** @} */

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			GpuVertexInput* CreateVertexInput(u32 id, const GpuVertexInputLayout& layout) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			using GpuMemoryAllocator = TGpuTlsfAllocator<D3D12HeapBackend>;
			using LinearPagePool = TGpuLinearPagePool<D3D12HeapBackend>;
			using ScratchAllocator = TGpuLinearAllocator<D3D12HeapBackend>;

			/** Returns how heaps of @p heapClass are created. */
			D3D12HeapCreateInformation GetHeapCreateInformation(D3D12HeapClass heapClass) const;

			/** Lazily creates and returns the allocator of heaps of @p heapClass. */
			GpuMemoryAllocator& GetOrCreateGpuMemoryAllocator(D3D12HeapClass heapClass);

			/** Lazily creates and returns the scratch page pool of heaps of @p heapClass, which must be a buffer class. */
			LinearPagePool& GetOrCreateLinearPagePool(D3D12HeapClass heapClass);

			ComPtr<ID3D12Device> mDevice;
			ComPtr<ID3D12Device10> mEnhancedDevice;
			ComPtr<IDXGIAdapter4> mAdapter;
			bool mIsPrimary = false;
			bool mIsTightAlignmentSupported = false;
			bool mSupportsBindlessResources = false;
			bool mHasFullSpeedMappableGpuMemory = false;
			D3D12HeapMemory mDefaultHeapMemory; /**< Memory of a default heap. */
			D3D12HeapMemory mGpuUploadHeapMemory; /**< Memory of a GPU upload heap: GPU memory the CPU writes, or system memory on unified memory. */
			D3D12HeapMemory mUploadHeapMemory; /**< Memory of an upload heap. */
			D3D12HeapMemory mReadbackHeapMemory; /**< Memory of a readback heap. */

			D3D12DescriptorManager* mDescriptorManager = nullptr;
			D3D12ResourceManager* mResourceManager = nullptr;
			TUnique<D3D12HeapBackend> mHeapBackend;
			TUnique<GpuMemoryAllocator> mGpuMemoryAllocators[(u32)D3D12HeapClass::Count];
			TUnique<LinearPagePool> mLinearPagePools[(u32)D3D12HeapClass::Count];

			/** Guards lazy creation of persistent allocators and scratch page pools. */
			Mutex mGpuMemoryAllocatorMutex;

			/** Transient heap pool of every GPU-only buffer and texture memory type, owned by GpuDevice. */
			IGpuTransientHeapPool* mTransientHeapPool = nullptr;
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
