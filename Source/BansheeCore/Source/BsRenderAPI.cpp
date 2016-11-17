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

namespace BansheeEngine 
{
	void RenderAPI::setGpuParams(CoreAccessor& accessor, const SPtr<GpuParams>& gpuParams)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setGpuParams, RenderAPICore::instancePtr(), gpuParams->getCore(), 
			nullptr));
	}

	void RenderAPI::setGraphicsPipeline(CoreAccessor& accessor, const SPtr<GraphicsPipelineState>& pipelineState)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setGraphicsPipeline, RenderAPICore::instancePtr(),
			pipelineState->getCore(), nullptr));
	}

	void RenderAPI::setComputePipeline(CoreAccessor& accessor, const SPtr<GpuProgram>& computeProgram)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setComputePipeline, RenderAPICore::instancePtr(),
			computeProgram->getCore(), nullptr));
	}

	void RenderAPI::setVertexBuffers(CoreAccessor& accessor, UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers)
	{
		Vector<SPtr<VertexBufferCore>> coreBuffers(buffers.size());
		for (UINT32 i = 0; i < (UINT32)buffers.size(); i++)
			coreBuffers[i] = buffers[i] != nullptr ? buffers[i]->getCore() : nullptr;

		std::function<void(RenderAPICore*, UINT32, const Vector<SPtr<VertexBufferCore>>&)> resizeFunc =
			[](RenderAPICore* rs, UINT32 idx, const Vector<SPtr<VertexBufferCore>>& _buffers)
		{
			rs->setVertexBuffers(idx, (SPtr<VertexBufferCore>*)_buffers.data(), (UINT32)_buffers.size());
		};

		accessor.queueCommand(std::bind(resizeFunc, RenderAPICore::instancePtr(), index, coreBuffers));
	}

	void RenderAPI::setIndexBuffer(CoreAccessor& accessor, const SPtr<IndexBuffer>& buffer)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setIndexBuffer, RenderAPICore::instancePtr(), buffer->getCore(), 
			nullptr));
	}

	void RenderAPI::setVertexDeclaration(CoreAccessor& accessor, const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setVertexDeclaration, RenderAPICore::instancePtr(), 
			vertexDeclaration->getCore(), nullptr));
	}

	void RenderAPI::setViewport(CoreAccessor& accessor, const Rect2& vp)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setViewport, RenderAPICore::instancePtr(), vp, nullptr));
	}

	void RenderAPI::setStencilRef(CoreAccessor& accessor, UINT32 value)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setStencilRef, RenderAPICore::instancePtr(), value, nullptr));
	}

	void RenderAPI::setDrawOperation(CoreAccessor& accessor, DrawOperationType op)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setDrawOperation, RenderAPICore::instancePtr(), op, 
			nullptr));
	}

	void RenderAPI::setScissorRect(CoreAccessor& accessor, UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setScissorRect, RenderAPICore::instancePtr(), left, top, right, bottom, 
			nullptr));
	}

	void RenderAPI::setRenderTarget(CoreAccessor& accessor, const SPtr<RenderTarget>& target, bool readOnlyDepthStencil)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::setRenderTarget, 
			RenderAPICore::instancePtr(), target->getCore(), readOnlyDepthStencil, nullptr));
	}

	void RenderAPI::beginRender(CoreAccessor& accessor)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::beginFrame, RenderAPICore::instancePtr(), nullptr));
	}

	void RenderAPI::endRender(CoreAccessor& accessor)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::endFrame, RenderAPICore::instancePtr(), nullptr));
	}

	void RenderAPI::clearRenderTarget(CoreAccessor& accessor, UINT32 buffers, const Color& color, float depth, 
		UINT16 stencil, UINT8 targetMask)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::clearRenderTarget, RenderAPICore::instancePtr(), buffers, color,
			depth, stencil, targetMask, nullptr));
	}

	void RenderAPI::clearViewport(CoreAccessor& accessor, UINT32 buffers, const Color& color, float depth, UINT16 stencil, 
		UINT8 targetMask)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::clearViewport, RenderAPICore::instancePtr(), buffers, color, depth,
			stencil, targetMask, nullptr));
	}

	void RenderAPI::swapBuffers(CoreAccessor& accessor, const SPtr<RenderTarget>& target)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::swapBuffers, RenderAPICore::instancePtr(), target->getCore(), 1));
	}

	void RenderAPI::draw(CoreAccessor& accessor, UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::draw, RenderAPICore::instancePtr(), vertexOffset, 
			vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::drawIndexed(CoreAccessor& accessor, UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, 
		UINT32 vertexCount, UINT32 instanceCount)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::drawIndexed, RenderAPICore::instancePtr(), startIndex, indexCount, 
			vertexOffset, vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::dispatchCompute(CoreAccessor& accessor, UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		accessor.queueCommand(std::bind(&RenderAPICore::dispatchCompute, RenderAPICore::instancePtr(), numGroupsX, 
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
		gCoreThread().queueCommand(std::bind((void(RenderAPICore::*)())&RenderAPICore::initialize, this), true);

		RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
		SPtr<RenderWindow> renderWindow = RenderWindow::create(windowDesc, nullptr);

		gCoreThread().queueCommand(std::bind(&RenderAPICore::initializeWithWindow, this, renderWindow->getCore()), true);

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
		gCoreAccessor().queueCommand(std::bind(&RenderAPICore::destroyCore, this));
		gCoreThread().submitAccessors(true);
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