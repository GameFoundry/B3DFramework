//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

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
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** Represents a single GPU device usable by DirectX 12. */
		class D3D12GpuDevice : public GpuDevice
		{
		public:
			static constexpr const char* kGpuProgramLanguageName = "hlsl";

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
			SPtr<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const override;

			u32 GetQueueCount(GpuQueueUsage usage) const override { return (u32)mQueueInfos[(u32)usage].Queues.size(); }
			SPtr<GpuQueue> GetQueue(GpuQueueUsage usage, u32 index) const override;
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, u32 syncMask = 0xFFFFFFFF) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;
			void EndFrame() override;

			SPtr<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			SPtr<Texture> CreateTexture(const TextureCreateInformation& createInformation, bool deferredInitialize) override;
			SPtr<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, bool deferredInitialize = false) override;
			SPtr<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			SPtr<EventQuery> CreateEventQuery() override;
			SPtr<TimerQuery> CreateTimerQuery() override;
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool isBinary) override;
			SPtr<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, bool deferredInitialize = false) override;
			SPtr<GpuParameterSet> CreateGpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex = 0, bool deferredInitialize = false) override;
			SPtr<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, bool deferredInitialize) override;
			SPtr<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, bool deferredInitialize) override;
			SPtr<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			SPtr<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription) override;

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

			/** Returns the memory allocator for creating GPU resources. */
			D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

			/** Returns the GPU timestamp frequency for this device. */
			u64 GetTimestampFrequency() const { return mTimestampFrequency; }

		private:
			friend class D3D12RenderAPI;
			friend class b3d::D3D12GpuBackend;

			SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, bool deferredInitialize = false) override;

			/** Initializes the capabilities of the device. */
			void InitializeCapabilities();

			/** Marks the device as a primary device. */
			void SetIsPrimary() { mIsPrimary = true; }

			ComPtr<ID3D12Device> mDevice;
			ComPtr<IDXGIAdapter4> mAdapter;
			bool mIsPrimary = false;

			D3D12DescriptorManager* mDescriptorManager = nullptr;
			D3D12MA::Allocator* mAllocator = nullptr;
			u64 mTimestampFrequency = 0;

			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				Vector<SPtr<D3D12GpuQueue>> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			SPtr<VideoModeInfo> mVideoModeInfo;
		};

		/** @} */
	} // namespace render
} // namespace b3d
