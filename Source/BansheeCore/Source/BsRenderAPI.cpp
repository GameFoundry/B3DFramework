//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderAPI.h"

#include "BsCoreThread.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsRenderWindow.h"
#include "BsMesh.h"
#include "BsGpuParams.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsGpuBuffer.h"
#include "BsGpuPipelineState.h"

using namespace std::placeholders;

namespace bs 
{
	void RenderAPI::setGpuParams(const SPtr<GpuParams>& gpuParams)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setGpuParams, RenderAPICore::instancePtr(), gpuParams->getCore(), 
			nullptr));
	}

	void RenderAPI::setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setGraphicsPipeline, RenderAPICore::instancePtr(),
			pipelineState->getCore(), nullptr));
	}

	void RenderAPI::setComputePipeline(const SPtr<ComputePipelineState>& pipelineState)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setComputePipeline, RenderAPICore::instancePtr(),
							  pipelineState->getCore(), nullptr));
	}

	void RenderAPI::setVertexBuffers(UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers)
	{
		Vector<SPtr<VertexBufferCore>> coreBuffers(buffers.size());
		for (UINT32 i = 0; i < (UINT32)buffers.size(); i++)
			coreBuffers[i] = buffers[i] != nullptr ? buffers[i]->getCore() : nullptr;

		std::function<void(RenderAPICore*, UINT32, const Vector<SPtr<VertexBufferCore>>&)> resizeFunc =
			[](RenderAPICore* rs, UINT32 idx, const Vector<SPtr<VertexBufferCore>>& _buffers)
		{
			rs->setVertexBuffers(idx, (SPtr<VertexBufferCore>*)_buffers.data(), (UINT32)_buffers.size());
		};

		gCoreThread().queueCommand(std::bind(resizeFunc, RenderAPICore::instancePtr(), index, coreBuffers));
	}

	void RenderAPI::setIndexBuffer(const SPtr<IndexBuffer>& buffer)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setIndexBuffer, RenderAPICore::instancePtr(), buffer->getCore(), 
			nullptr));
	}

	void RenderAPI::setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setVertexDeclaration, RenderAPICore::instancePtr(), 
			vertexDeclaration->getCore(), nullptr));
	}

	void RenderAPI::setViewport(const Rect2& vp)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setViewport, RenderAPICore::instancePtr(), vp, nullptr));
	}

	void RenderAPI::setStencilRef(UINT32 value)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setStencilRef, RenderAPICore::instancePtr(), value, nullptr));
	}

	void RenderAPI::setDrawOperation(DrawOperationType op)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setDrawOperation, RenderAPICore::instancePtr(), op, 
			nullptr));
	}

	void RenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setScissorRect, RenderAPICore::instancePtr(), left, top, right, bottom, 
			nullptr));
	}

	void RenderAPI::setRenderTarget(const SPtr<RenderTarget>& target, bool readOnlyDepthStencil,
									RenderSurfaceMask loadMask)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::setRenderTarget, 
			RenderAPICore::instancePtr(), target->getCore(), readOnlyDepthStencil, loadMask, nullptr));
	}

	void RenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, 
		UINT16 stencil, UINT8 targetMask)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::clearRenderTarget, RenderAPICore::instancePtr(), buffers, color,
			depth, stencil, targetMask, nullptr));
	}

	void RenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, 
		UINT8 targetMask)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::clearViewport, RenderAPICore::instancePtr(), buffers, color, depth,
			stencil, targetMask, nullptr));
	}

	void RenderAPI::swapBuffers(const SPtr<RenderTarget>& target)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::swapBuffers, RenderAPICore::instancePtr(), target->getCore(), 1));
	}

	void RenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::draw, RenderAPICore::instancePtr(), vertexOffset, 
			vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, 
		UINT32 vertexCount, UINT32 instanceCount)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::drawIndexed, RenderAPICore::instancePtr(), startIndex, indexCount, 
			vertexOffset, vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::dispatchCompute, RenderAPICore::instancePtr(), numGroupsX, 
			numGroupsY, numGroupsZ, nullptr));
	}

	const VideoModeInfo& RenderAPI::getVideoModeInfo()
	{
		return RenderAPICore::instance().getVideoModeInfo();
	}

	void RenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		RenderAPICore::instance().convertProjectionMatrix(matrix, dest);
	}

	const RenderAPIInfo& RenderAPI::getAPIInfo()
	{
		return RenderAPICore::instance().getAPIInfo();
	}

    RenderAPICore::RenderAPICore()
        : mCurrentCapabilities(nullptr), mNumDevices(0)
    {
    }

    RenderAPICore::~RenderAPICore()
    {
		// Base classes need to call virtual destroy_internal method instead of a destructor

		bs_deleteN(mCurrentCapabilities, mNumDevices);
		mCurrentCapabilities = nullptr;
    }

	SPtr<RenderWindow> RenderAPICore::initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
	{
		gCoreThread().queueCommand(std::bind((void(RenderAPICore::*)())&RenderAPICore::initialize, this), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);

		RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
		SPtr<RenderWindow> renderWindow = RenderWindow::create(windowDesc, nullptr);

		gCoreThread().queueCommand(std::bind(&RenderAPICore::initializeWithWindow, this, renderWindow->getCore()), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);

		return renderWindow;
	}

	void RenderAPICore::initialize()
	{
		// Do nothing
	}

	void RenderAPICore::initializeWithWindow(const SPtr<RenderWindowCore>& primaryWindow)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void RenderAPICore::destroy()
	{
		gCoreThread().queueCommand(std::bind(&RenderAPICore::destroyCore, this));
		gCoreThread().submitAll(true);
	}

	void RenderAPICore::destroyCore()
	{
		mActiveRenderTarget = nullptr;
	}

	const RenderAPICapabilities& RenderAPICore::getCapabilities(UINT32 deviceIdx) const
	{
		if(deviceIdx >= mNumDevices)
		{
			LOGWRN("Invalid device index provided: " + toString(deviceIdx) + ". Valid range is: [0, " + toString(mNumDevices) + ").");
			return mCurrentCapabilities[0];
		}

		return mCurrentCapabilities[deviceIdx];
	}

	UINT32 RenderAPICore::vertexCountToPrimCount(DrawOperationType type, UINT32 elementCount)
	{
		UINT32 primCount = 0;
		switch (type)
		{
		case DOT_POINT_LIST:
			primCount = elementCount;
			break;

		case DOT_LINE_LIST:
			primCount = elementCount / 2;
			break;

		case DOT_LINE_STRIP:
			primCount = elementCount - 1;
			break;

		case DOT_TRIANGLE_LIST:
			primCount = elementCount / 3;
			break;

		case DOT_TRIANGLE_STRIP:
			primCount = elementCount - 2;
			break;

		case DOT_TRIANGLE_FAN:
			primCount = elementCount - 2;
			break;
		}

		return primCount;
	}
}