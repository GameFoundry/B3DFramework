//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

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
			static constexpr const char* kGpuProgramLanguageName = "nullsl";

			NullGpuDevice();
			~NullGpuDevice();

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
			void SubmitTransferCommandBuffers(bool wait = false) override {}
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override {}
			void WaitUntilIdle() override {}
			void BeginFrame() override {}
			void EndFrame() override {}

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

			void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) override;
			GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) override;
			float ConvertTimestampToMilliseconds(u64 timestamp) override { return 0.0f; }

			/** @} */

		private:
			/** Contains data about a set of queues of a specific type. */
			struct QueueInfo
			{
				u32 FamilyIndex = ~0u;
				TArray<SPtr<GpuQueue>> Queues;
			};

			SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) override;

			/** Initializes capabilities with reasonable defaults for a null backend. */
			void InitializeCapabilities();

			bool mIsInitialized = false;
			QueueInfo mQueueInfos[GQT_COUNT];
			GpuDeviceCapabilities mCapabilities;
			SPtr<VideoModeInfo> mVideoModeInfo;
		};

		/** @} */
	} // namespace render
} // namespace b3d
