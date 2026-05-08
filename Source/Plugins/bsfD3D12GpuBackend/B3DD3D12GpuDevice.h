//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

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
			TShared<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const override;

			u32 GetQueueCount(GpuQueueUsage usage) const override { return (u32)mQueueInfos[(u32)usage].Queues.size(); }
			TShared<GpuQueue> GetQueue(GpuQueueUsage usage, u32 index) const override;
			void PresentRenderWindow(const TShared<RenderWindow>& renderWindow, u32 syncMask = 0xFFFFFFFF) override;
			void WaitUntilIdle() override;
			void BeginFrame() override;

			TShared<GpuCommandBufferPool> CreateGpuCommandBufferPool(const GpuCommandBufferPoolCreateInformation& createInformation) override;
			TShared<Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags) override;
			TShared<GpuQueryPool> CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation) override;
			TShared<EventQuery> CreateEventQuery() override;
			TShared<TimerQuery> CreateTimerQuery() override;
			TShared<OcclusionQuery> CreateOcclusionQuery(bool isBinary) override;
			TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;
			TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) override;
			TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription) override;
			UPtr<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) override;
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

			/** Returns the memory allocator for creating GPU resources. */
			D3D12MA::Allocator* GetAllocator() const { return mAllocator; }

			/** Returns the GPU timestamp frequency for this device. */
			u64 GetTimestampFrequency() const { return mTimestampFrequency; }

		private:
			friend class b3d::D3D12GpuBackend;

			TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

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
				Vector<TShared<D3D12GpuQueue>> Queues;
			};

			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			TShared<VideoModeInfo> mVideoModeInfo;
		};

		/** @} */
	} // namespace render
} // namespace b3d
