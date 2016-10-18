//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderAPI.h"
#include "BsCoreThread.h"
#include "BsRenderStats.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	VulkanRenderAPI::VulkanRenderAPI()
	{ }

	VulkanRenderAPI::~VulkanRenderAPI()
	{

	}

	const StringID& VulkanRenderAPI::getName() const
	{
		static StringID strName("VulkanRenderAPI");
		return strName;
	}

	const String& VulkanRenderAPI::getShadingLanguageName() const
	{
		static String strName("glsl");
		return strName;
	}

	void VulkanRenderAPI::initializePrepare()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderAPICore::initializePrepare();
	}

	void VulkanRenderAPI::initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow)
	{
		RenderAPICore::initializeFinalize(primaryWindow);
	}

    void VulkanRenderAPI::destroyCore()
	{
		THROW_IF_NOT_CORE_THREAD;

		RenderAPICore::destroyCore();
	}

	void VulkanRenderAPI::setGraphicsPipeline(const SPtr<GpuPipelineStateCore>& pipelineState,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setComputePipeline(const SPtr<GpuProgramCore>& computeProgram,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPipelineStateChanges);
	}

	void VulkanRenderAPI::setGpuParams(const SPtr<GpuParamsCore>& gpuParams, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumGpuParamBinds);
	}

	void VulkanRenderAPI::beginFrame(const SPtr<CommandBuffer>& commandBuffer)
	{

	}

	void VulkanRenderAPI::endFrame(const SPtr<CommandBuffer>& commandBuffer)
	{

	}

	void VulkanRenderAPI::setViewport(const Rect2& vp, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumVertexBufferBinds);
	}

	void VulkanRenderAPI::setIndexBuffer(const SPtr<IndexBufferCore>& buffer, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumIndexBufferBinds);
	}

	void VulkanRenderAPI::setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
		UINT32 instanceCount, const SPtr<CommandBuffer>& commandBuffer)
	{
		UINT32 primCount = 0;

		BS_INC_RENDER_STAT(NumDrawCalls);
		BS_ADD_RENDER_STAT(NumVertices, vertexCount);
		BS_ADD_RENDER_STAT(NumPrimitives, primCount);
	}

	void VulkanRenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumComputeCalls);
	}

	void VulkanRenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer)
	{
		
	}

	void VulkanRenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, UINT8 targetMask,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, UINT16 stencil,
		UINT8 targetMask, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumClears);
	}

	void VulkanRenderAPI::setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil,
		const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumRenderTargetChanges);
	}

	void VulkanRenderAPI::swapBuffers(const SPtr<RenderTargetCore>& target, const SPtr<CommandBuffer>& commandBuffer)
	{
		BS_INC_RENDER_STAT(NumPresents);
	}

	void VulkanRenderAPI::addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary)
	{

	}

	void VulkanRenderAPI::executeCommands(const SPtr<CommandBuffer>& commandBuffer)
	{

	}
	
	void VulkanRenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{

	}

	const RenderAPIInfo& VulkanRenderAPI::getAPIInfo() const
	{
		static RenderAPIInfo info(0.0f, 0.0f, 0.0f, 1.0f, VET_COLOR_ABGR, false, true, false, false);

		return info;
	}

	GpuParamBlockDesc VulkanRenderAPI::generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
	{
		GpuParamBlockDesc block;
		
		return block;
	}
}