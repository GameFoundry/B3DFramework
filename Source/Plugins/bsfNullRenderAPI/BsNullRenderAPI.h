//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs { namespace ct
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
		const StringID& getName() const override;
		
		/** @copydoc RenderAPI::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setComputePipeline */
		void setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setGpuParams */
		void setGpuParams(const SPtr<GpuParams>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags, 
			RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setViewport */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBuffer>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::setDrawOperation */
		void setDrawOperation(DrawOperationType op,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override { }

		/** @copydoc RenderAPI::swapBuffers() */
		void swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask = 0xFFFFFFFF) override { }

		/** @copydoc RenderAPI::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override { }

		/** @copydoc RenderAPI::submitCommandBuffer() */
		void submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask = 0xFFFFFFFF) override { }

		/** @copydoc RenderAPI::convertProjectionMatrix */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPI::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override ;

	protected:
		friend class NullRenderAPIFactory;

		/** @copydoc RenderAPI::initialize */
		void initialize() override;

		/** @copydoc RenderAPI::initializeWithWindow */
		void initializeWithWindow(const SPtr<RenderWindow>& primaryWindow) override;

		/** @copydoc RenderAPI::destroyCore */
		void destroyCore() override;

		NullProgramFactory* mNullProgramFactory = nullptr;
	};

	/** @} */
}}