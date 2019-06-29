//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderAPI.h"

#include "CoreThread/BsCoreThread.h"
#include "RenderAPI/BsViewport.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsGpuPipelineState.h"

using namespace std::placeholders;

namespace bs 
{
	void RenderAPI::setGpuParams(const SPtr<GpuParams>& gpuParams)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setGpuParams, ct::RenderAPI::instancePtr(), gpuParams->getCore(),
			nullptr));
	}

	void RenderAPI::setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setGraphicsPipeline, ct::RenderAPI::instancePtr(),
			pipelineState->getCore(), nullptr));
	}

	void RenderAPI::setComputePipeline(const SPtr<ComputePipelineState>& pipelineState)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setComputePipeline, ct::RenderAPI::instancePtr(),
							  pipelineState->getCore(), nullptr));
	}

	void RenderAPI::setVertexBuffers(UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers)
	{
		Vector<SPtr<ct::VertexBuffer>> coreBuffers(buffers.size());
		for (UINT32 i = 0; i < (UINT32)buffers.size(); i++)
			coreBuffers[i] = buffers[i] != nullptr ? buffers[i]->getCore() : nullptr;

		std::function<void(ct::RenderAPI*, UINT32, const Vector<SPtr<ct::VertexBuffer>>&)> resizeFunc =
			[](ct::RenderAPI* rs, UINT32 idx, const Vector<SPtr<ct::VertexBuffer>>& _buffers)
		{
			rs->setVertexBuffers(idx, (SPtr<ct::VertexBuffer>*)_buffers.data(), (UINT32)_buffers.size());
		};

		gCoreThread().queueCommand(std::bind(resizeFunc, ct::RenderAPI::instancePtr(), index, coreBuffers));
	}

	void RenderAPI::setIndexBuffer(const SPtr<IndexBuffer>& buffer)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setIndexBuffer, ct::RenderAPI::instancePtr(), buffer->getCore(),
			nullptr));
	}

	void RenderAPI::setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setVertexDeclaration, ct::RenderAPI::instancePtr(),
			vertexDeclaration->getCore(), nullptr));
	}

	void RenderAPI::setViewport(const Rect2& vp)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setViewport, ct::RenderAPI::instancePtr(), vp, nullptr));
	}

	void RenderAPI::setStencilRef(UINT32 value)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setStencilRef, ct::RenderAPI::instancePtr(), value, nullptr));
	}

	void RenderAPI::setDrawOperation(DrawOperationType op)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setDrawOperation, ct::RenderAPI::instancePtr(), op,
			nullptr));
	}

	void RenderAPI::setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setScissorRect, ct::RenderAPI::instancePtr(), left, top, right, bottom,
			nullptr));
	}

	void RenderAPI::setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags,
									RenderSurfaceMask loadMask)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::setRenderTarget,
			ct::RenderAPI::instancePtr(), target->getCore(), readOnlyFlags, loadMask, nullptr));
	}

	void RenderAPI::clearRenderTarget(UINT32 buffers, const Color& color, float depth, 
		UINT16 stencil, UINT8 targetMask)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::clearRenderTarget, ct::RenderAPI::instancePtr(), buffers, color,
			depth, stencil, targetMask, nullptr));
	}

	void RenderAPI::clearViewport(UINT32 buffers, const Color& color, float depth, UINT16 stencil, 
		UINT8 targetMask)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::clearViewport, ct::RenderAPI::instancePtr(), buffers, color, depth,
			stencil, targetMask, nullptr));
	}

	void RenderAPI::swapBuffers(const SPtr<RenderTarget>& target)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::swapBuffers, ct::RenderAPI::instancePtr(), target->getCore(), 1));
	}

	void RenderAPI::draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::draw, ct::RenderAPI::instancePtr(), vertexOffset,
			vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, 
		UINT32 vertexCount, UINT32 instanceCount)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::drawIndexed, ct::RenderAPI::instancePtr(), startIndex, indexCount,
			vertexOffset, vertexCount, instanceCount, nullptr));
	}

	void RenderAPI::dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY, UINT32 numGroupsZ)
	{
		gCoreThread().queueCommand(std::bind(&ct::RenderAPI::dispatchCompute, ct::RenderAPI::instancePtr(), numGroupsX,
			numGroupsY, numGroupsZ, nullptr));
	}

	const VideoModeInfo& RenderAPI::getVideoModeInfo()
	{
		return ct::RenderAPI::instance().getVideoModeInfo();
	}

	void RenderAPI::convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
	{
		ct::RenderAPI::instance().convertProjectionMatrix(matrix, dest);
	}

	namespace ct
	{
	RenderAPI::RenderAPI()
		: mCurrentCapabilities(nullptr), mNumDevices(0)
	{
	}

	RenderAPI::~RenderAPI()
	{
		// Base classes need to call virtual destroy_internal method instead of a destructor

		bs_deleteN(mCurrentCapabilities, mNumDevices);
		mCurrentCapabilities = nullptr;
	}

	SPtr<bs::RenderWindow> RenderAPI::initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
	{
		gCoreThread().queueCommand(std::bind((void(RenderAPI::*)())&RenderAPI::initialize, this), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);

		RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
		SPtr<bs::RenderWindow> renderWindow = bs::RenderWindow::create(windowDesc, nullptr);

		// Make sure render window initialization is submitted to the internal queue
		gCoreThread().submitAll();

		gCoreThread().queueCommand(std::bind(&RenderAPI::initializeWithWindow, this, renderWindow->getCore()), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);

		return renderWindow;
	}

	void RenderAPI::initialize()
	{
		// Do nothing
	}

	void RenderAPI::initializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void RenderAPI::destroy()
	{
		gCoreThread().queueCommand(std::bind(&RenderAPI::destroyCore, this));
		gCoreThread().submitAll(true);
	}

	void RenderAPI::destroyCore()
	{
		mActiveRenderTarget = nullptr;
	}

	const RenderAPICapabilities& RenderAPI::getCapabilities(UINT32 deviceIdx) const
	{
		if(deviceIdx >= mNumDevices)
		{
			BS_LOG(Warning, RenderBackend, "Invalid device index provided: {0}. Valid range is: [0, {1}).", 
				deviceIdx, mNumDevices);
			return mCurrentCapabilities[0];
		}

		return mCurrentCapabilities[deviceIdx];
	}

	UINT32 RenderAPI::vertexCountToPrimCount(DrawOperationType type, UINT32 elementCount)
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
}