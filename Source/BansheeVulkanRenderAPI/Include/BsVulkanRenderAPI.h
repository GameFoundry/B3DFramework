//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderAPI.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Implementation of a render system using Vulkan. Provides abstracted access to various low level Vulkan methods. */
	class VulkanRenderAPI : public RenderAPICore
	{
	public:
		VulkanRenderAPI();
		~VulkanRenderAPI();

		/** @copydoc RenderAPICore::getName */
		const StringID& getName() const override;
		
		/** @copydoc RenderAPICore::getShadingLanguageName */
		const String& getShadingLanguageName() const override;

		/** @copydoc RenderAPICore::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GpuPipelineStateCore>& pipelineState, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setComputePipeline */
		void setComputePipeline(const SPtr<GpuProgramCore>& computeProgram,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setGpuParams */
		void setGpuParams(const SPtr<GpuParamsCore>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::beginFrame */
		void beginFrame(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::endFrame */
		void endFrame(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setViewport */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBufferCore>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setDrawOperation */
		void setDrawOperation(DrawOperationType op,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::swapBuffers() */
		void swapBuffers(const SPtr<RenderTargetCore>& target,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;

		/** @copydoc RenderAPICore::executeCommands() */
		void executeCommands(const SPtr<CommandBuffer>& commandBuffer) override;

		/** @copydoc RenderAPICore::convertProjectionMatrix */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPICore::getAPIInfo */
		const RenderAPIInfo& getAPIInfo() const override;

		/** @copydoc RenderAPICore::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

	protected:
		friend class VulkanRenderAPIFactory;

		/** @copydoc RenderAPICore::initializePrepare */
		void initializePrepare() override;

		/** @copydoc RenderAPICore::initializeFinalize */
		void initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow) override;

		/** @copydoc RenderAPICore::destroyCore */
		void destroyCore() override;

	private:
	};

	/** @} */
}