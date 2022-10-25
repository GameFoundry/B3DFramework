//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		class NullProgramFactory;

		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/** Implementation of a render system that has no backend and performs no operations internally. */
		class NullRenderAPI final : public RenderAPI
		{
		public:
			/** @copydoc RenderAPI::getName */
			const StringID& GetName() const;

			/** @copydoc RenderAPI::setGraphicsPipeline */
			void SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setComputePipeline */
			void SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setGpuParams */
			void SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::clearRenderTarget */
			void ClearRenderTarget(u32 buffers, const Color& color = Color::Black, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::clearViewport */
			void ClearViewport(u32 buffers, const Color& color = Color::Black, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setRenderTarget */
			void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setViewport */
			void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setScissorRect */
			void SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr) override {}

			/** @copydoc RenderAPI::setStencilRef */
			void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setVertexBuffers */
			void SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setIndexBuffer */
			void SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setVertexDeclaration */
			void SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::setDrawOperation */
			void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::draw */
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::drawIndexed */
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::dispatchCompute */
			void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr) {}

			/** @copydoc RenderAPI::swapBuffers() */
			void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF) override {}

			/** @copydoc RenderAPI::addCommands() */
			void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) {}

			/** @copydoc RenderAPI::submitCommandBuffer() */
			void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask = 0xFFFFFFFF) {}

			/** @copydoc RenderAPI::convertProjectionMatrix */
			void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

			/** @copydoc RenderAPI::generateParamBlockDesc() */
			GpuParamBlockDesc GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params);

		protected:
			friend class NullRenderAPIFactory;

			/** @copydoc RenderAPI::initialize */
			void Initialize() override;

			/** @copydoc RenderAPI::initializeWithWindow */
			void InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow);

			/** @copydoc RenderAPI::destroyCore */
			void DestroyCore() override;

		public:
			SPtr<CommandBuffer> GetMainCommandBuffer() const { return nullptr; }

		protected:
			NullProgramFactory* mNullProgramFactory = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
