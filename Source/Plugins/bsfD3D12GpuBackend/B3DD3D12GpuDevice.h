//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12HeapBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
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
			 * Returns true if placed buffers use tight alignment. Buffers are then placed resources of their own in persistent
			 * memory too, rather than slices of buffer pages.
			 */
			bool IsTightAlignmentSupported() const { return mIsTightAlignmentSupported; }

			/**
			 * Returns the properties of the heap a buffer of @p type, @p memory and @p flags is placed in, following GpuBackendUtility::GetBufferPlacement().
			 * Mapped GPU memory is a GPU upload heap with Resizable BAR, and a custom heap on unified memory. Mapped system memory is a
			 * write-combined custom heap, which GPU copies can write, unlike an upload heap. Staging buffers are in custom heaps with the
			 * properties of upload and readback heaps, for the same reason. Once GPU memory is close to its budget, a GPU upload heap holding GpuBufferMemory::Device buffers is a default heap, and
			 * one holding GpuBufferMemory::Mapped buffers a write-combined custom heap (see D3D12HeapBackend::CreateHeap()), so the heap of
			 * a buffer can differ. Mapped heaps have no buffer pages for unordered access
			 * views, so their memory requirements report mapped buffers the GPU writes as unsupported.
			 */
			D3D12HeapProperties GetBufferHeapProperties(GpuBufferType type, GpuBufferMemory memory, GpuBufferFlags flags) const;

			/**
			 * Returns true if the CPU can write the GPU's memory directly while the GPU still reads it at full speed: GPU upload heaps on
			 * discrete GPUs (Resizable BAR), cache-coherent unified memory (or any unified memory, with gpu.ForceDirectGpuBufferWrites).
			 */
			bool HasFullSpeedMappableGpuMemory() const { return mHasFullSpeedMappableGpuMemory; }

			/**
			 * Creates a resource using the heap pool matching @p heapProperties. When valid, @p outAllocation owns the suballocation and must be
			 * released with FreeMemory after the native resource is destroyed.
			 */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12HeapProperties& heapProperties, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuAllocation& outAllocation);

			/** Creates a resource in a heap of @p heapType using the matching heap pool. See the overload taking a D3D12HeapProperties. */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, D3D12_HEAP_TYPE heapType, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuAllocation& outAllocation);

			/**
			 * Creates a resource placed at @p requestedAllocation. A pending allocation suballocates from its allocator, which must
			 * serve the resource's heap pool. An allocation with memory is used as is. @p outAllocation receives the memory
			 * the resource is placed at, and must be released with FreeMemory after the native resource is destroyed.
			 */
			HRESULT CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, const GpuAllocation& requestedAllocation, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ComPtr<ID3D12Resource>& outResource, GpuAllocation& outAllocation);

			/** Releases a TLSF suballocation if it is owned. The native resource using it must already have been destroyed. */
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

			/** Returns the class of heaps that hold the specified resource in a heap with @p heapProperties, or Count when unsupported. */
			static D3D12HeapClass GetHeapClass(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12HeapProperties& heapProperties);

			/** Lazily creates and returns the allocator of heaps of @p heapClass. */
			GpuMemoryAllocator& GetOrCreateGpuMemoryAllocator(D3D12HeapClass heapClass);

			ComPtr<ID3D12Device> mDevice;
			ComPtr<ID3D12Device10> mEnhancedDevice;
			ComPtr<IDXGIAdapter4> mAdapter;
			bool mIsPrimary = false;
			bool mIsTightAlignmentSupported = false;
			bool mSupportsBindlessResources = false;
			bool mHasFullSpeedMappableGpuMemory = false;
			D3D12_CPU_PAGE_PROPERTY mMappedGpuPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; /**< CPU page property of the custom heaps holding mapped GPU memory on unified memory. */
			D3D12_CPU_PAGE_PROPERTY mStagingWritePageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE; /**< CPU page property of the custom heaps holding staging buffers the CPU writes, that of an upload heap. */
			D3D12_CPU_PAGE_PROPERTY mStagingReadPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; /**< CPU page property of the custom heaps holding staging buffers the CPU reads, that of a readback heap. */

			D3D12DescriptorManager* mDescriptorManager = nullptr;
			D3D12ResourceManager* mResourceManager = nullptr;
			TUnique<D3D12HeapBackend> mHeapBackend;
			TUnique<D3D12BufferPool> mBufferPool;
			TUnique<GpuMemoryAllocator> mGpuMemoryAllocators[(u32)D3D12HeapClass::Count];
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
