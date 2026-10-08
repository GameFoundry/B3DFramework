//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup NullGpuBackend
		 *  @{
		 */

		/** Heap created by NullHeapBackend. It has no backing memory. */
		struct NullGpuHeap : IGpuHeap
		{
			u64 Size = 0; /**< Size of the heap in bytes. */
		};

		/** Heap backend for the null device. Heaps only exist to give GPU allocations an identity. */
		class NullHeapBackend
		{
		public:
			using HeapHandle = IGpuHeap*;

			/** Initializer struct for CreateHeap(). The null backend has no heap properties. */
			struct HeapCreateInformation
			{ };

			/** Creates a heap of @p sizeInBytes bytes. */
			HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

			/** Destroys a heap previously created with CreateHeap(). */
			void DestroyHeap(HeapHandle handle);
		};

		/**
		 * Represents a single null GPU device.
		 *
		 * This device simulates a GPU without performing any actual rendering operations. It reports
		 * reasonable capabilities to allow content to be loaded and processed, but all draw calls and
		 * resource operations are no-ops. Useful for testing rendering logic without requiring GPU hardware.
		 */
		class NullGpuDevice : public GpuDevice
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = kGpuProgramLanguageNullsl;

			NullGpuDevice();
			~NullGpuDevice() override;

			/**
			 * @name GpuDevice Interface
			 *  @{
			 */

			bool IsInitialized() const override { return mIsInitialized; }
			bool Initialize() override;

			const GpuDeviceCapabilities& GetCapabilities() const override { return mCapabilities; }
			const VideoModeInfo& GetVideoModeInfo() const override { return *mVideoModeInfo; }

			bool IsGpuProgramLanguageSupported(const StringView& language) const override { return language == kGpuProgramLanguageName; }
			TShared<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const override;

			u32 GetQueueCount(GpuQueueType type) const override;
			TShared<GpuQueue> GetQueue(GpuQueueType type, u32 index) const override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override {}
			void WaitUntilIdle() override {}
			void BeginFrame() override {}

			TShared<render::GpuCommandBufferPool> CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation) override;
			GpuMemoryRequirements GetMemoryRequirements(const TextureCreateInformation& createInformation) const override;
			GpuMemoryRequirements GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const override;
			IGpuAllocator& GetPersistentAllocator(u32 memoryType) override;
			IGpuTransientHeapPool* GetTransientHeapPool(u32 memoryType) override;
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
			float ConvertTimestampToMilliseconds(u64 timestamp) override { return 0.0f; }

			/** @} */

		private:
			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				TArray<TShared<GpuQueue>> Queues;
			};

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) override;

			/** Initializes capabilities with reasonable defaults for a null backend. */
			void InitializeCapabilities();

			bool mIsInitialized = false;
			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;

			NullHeapBackend mHeapBackend;
			TUnique<TGpuTlsfAllocator<NullHeapBackend>> mPersistentAllocator; /**< Allocator for the device's only memory type. */
			IGpuTransientHeapPool* mTransientHeapPool = nullptr; /**< Transient heap pool of the device's only memory type, owned by GpuDevice. */
		};

		/** @} */
	} // namespace render
} // namespace b3d

B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(b3d::render::NullHeapBackend);
