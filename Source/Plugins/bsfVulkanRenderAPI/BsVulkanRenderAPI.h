//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanGpuBackend.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Implementation of a render system using Vulkan. Provides abstracted access to various low level Vulkan methods. */
		class VulkanRenderAPI : public RenderAPI
		{
		public:
			VulkanRenderAPI() = default;
			~VulkanRenderAPI() override = default;

			const StringID& GetName() const override;
			void SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetGpuParams(const SPtr<GpuParameters>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DisableScissorTest(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetIndexBuffer(const SPtr<GpuBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF) override;
			void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;
			void BeginLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void EndLabel(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void InsertLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void BeginFrame() override;
			void EndFrame() override;
			void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask = 0xFFFFFFFF) override;
			void WaitUntilIdle() const override;
			SPtr<CommandBuffer> GetMainCommandBuffer() const override;
			SPtr<GpuDevice> GetPrimaryGpuDevice() const override { return mPrimaryGpuDevice; }
			void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;
			GpuParameterBlockInformation GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params) override;

			/**
			 * @name Internal
			 * @{
			 */

			/** Returns the main command buffer, executing on the graphics queue. */
			VulkanCommandBuffer* GetMainVulkanCommandBuffer() const { return mMainCommandBuffer.get(); }

			/** @} */
		protected:
			friend class VulkanRenderAPIFactory;

			void Initialize() override;
			void DestroyCore() override;

			/**
			 * Returns a valid command buffer. Uses the provided buffer if not null. Otherwise returns the default command
			 * buffer.
			 */
			VulkanCommandBuffer* EnsureCommandBuffer(const SPtr<CommandBuffer>& buffer);

		private:
			/** Queued swap chain operation. */
			struct QueuedSwapOperation
			{
				QueuedSwapOperation(const SPtr<RenderTarget>& target = nullptr, const VulkanSwapChain* swapChain = nullptr, u32 syncMask = ~0u)
					: Target(target), SwapChain(swapChain), SyncMask(syncMask)
				{}

				SPtr<RenderTarget> Target;
				const VulkanSwapChain* SwapChain;
				u32 SyncMask = ~0u;
			};

			SPtr<VulkanCommandBuffer> mMainCommandBuffer;
			SPtr<GpuDevice> mPrimaryGpuDevice;
			mutable Vector<SPtr<CommandBuffer>> mSubmittedCommandBuffers;
		};

		/**	Provides easy access to the VulkanRenderAPI. */
		VulkanRenderAPI& GetVulkanRenderAPI();

		/** @} */
	} // namespace ct
} // namespace bs
