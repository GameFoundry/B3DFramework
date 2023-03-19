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
#include "RenderAPI/BsGpuPipelineState.h"

using namespace std::placeholders;

using namespace bs;

void RenderAPI::SetGpuParams(const SPtr<GpuParams>& gpuParams)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetGpuParams, ct::RenderAPI::InstancePtr(), gpuParams->GetCore(), nullptr));
}

void RenderAPI::SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetGraphicsPipeline, ct::RenderAPI::InstancePtr(), pipelineState->GetCore(), nullptr));
}

void RenderAPI::SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetComputePipeline, ct::RenderAPI::InstancePtr(), pipelineState->GetCore(), nullptr));
}

void RenderAPI::SetVertexBuffers(u32 index, const Vector<SPtr<GpuBuffer>>& buffers)
{
	Vector<SPtr<ct::GpuBuffer>> coreBuffers(buffers.size());
	for(u32 i = 0; i < (u32)buffers.size(); i++)
		coreBuffers[i] = buffers[i] != nullptr ? buffers[i]->GetCore() : nullptr;

	std::function<void(ct::RenderAPI*, u32, const Vector<SPtr<ct::GpuBuffer>>&)> resizeFunc =
		[](ct::RenderAPI* rs, u32 idx, const Vector<SPtr<ct::GpuBuffer>>& _buffers)
	{
		rs->SetVertexBuffers(idx, (SPtr<ct::GpuBuffer>*)_buffers.data(), (u32)_buffers.size());
	};

	GetCoreThread().QueueCommand(std::bind(resizeFunc, ct::RenderAPI::InstancePtr(), index, coreBuffers));
}

void RenderAPI::SetIndexBuffer(const SPtr<GpuBuffer>& buffer)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetIndexBuffer, ct::RenderAPI::InstancePtr(), buffer->GetCore(), nullptr));
}

void RenderAPI::SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetVertexDeclaration, ct::RenderAPI::InstancePtr(), vertexDeclaration->GetCore(), nullptr));
}

void RenderAPI::SetViewport(const Rect2& vp)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetViewport, ct::RenderAPI::InstancePtr(), vp, nullptr));
}

void RenderAPI::SetStencilRef(u32 value)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetStencilRef, ct::RenderAPI::InstancePtr(), value, nullptr));
}

void RenderAPI::SetDrawOperation(DrawOperationType op)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetDrawOperation, ct::RenderAPI::InstancePtr(), op, nullptr));
}

void RenderAPI::SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SetRenderTarget, ct::RenderAPI::InstancePtr(), target->GetCore(), readOnlyFlags, loadMask, nullptr));
}

void RenderAPI::ClearRenderTarget(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::ClearRenderTarget, ct::RenderAPI::InstancePtr(), buffers, color, depth, stencil, targetMask, nullptr));
}

void RenderAPI::ClearViewport(u32 buffers, const Color& color, float depth, u16 stencil, u8 targetMask)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::ClearViewport, ct::RenderAPI::InstancePtr(), buffers, color, depth, stencil, targetMask, nullptr));
}

void RenderAPI::SwapBuffers(const SPtr<RenderTarget>& target)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::SwapBuffers, ct::RenderAPI::InstancePtr(), target->GetCore(), 1));
}

void RenderAPI::Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::Draw, ct::RenderAPI::InstancePtr(), vertexOffset, vertexCount, instanceCount, 0, nullptr));
}

void RenderAPI::DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::DrawIndexed, ct::RenderAPI::InstancePtr(), startIndex, indexCount, vertexOffset, vertexCount, instanceCount, 0, nullptr));
}

void RenderAPI::DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ)
{
	GetCoreThread().QueueCommand(std::bind(&ct::RenderAPI::DispatchCompute, ct::RenderAPI::InstancePtr(), numGroupsX, numGroupsY, numGroupsZ, nullptr));
}

void RenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	ct::RenderAPI::Instance().ConvertProjectionMatrix(matrix, dest);
}

namespace bs { namespace ct
{
RenderAPI::RenderAPI()
{
}

RenderAPI::~RenderAPI()
{
	// Base classes need to call virtual destroy_internal method instead of a destructor
}

SPtr<bs::RenderWindow> RenderAPI::Initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
{
	GetCoreThread().QueueCommand(std::bind((void(RenderAPI::*)()) & RenderAPI::Initialize, this), CTQF_InternalQueue | CTQF_BlockUntilComplete);

	RENDER_WINDOW_DESC windowDesc = primaryWindowDesc;
	SPtr<bs::RenderWindow> renderWindow = bs::RenderWindow::Create(windowDesc, nullptr);

	// Make sure render window initialization is submitted to the internal queue
	GetCoreThread().SubmitAll();

	GetCoreThread().QueueCommand(std::bind(&RenderAPI::InitializeWithWindow, this, renderWindow->GetCore()), CTQF_InternalQueue | CTQF_BlockUntilComplete);

	return renderWindow;
}

void RenderAPI::Initialize()
{
	// Do nothing
}

void RenderAPI::InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
{
	THROW_IF_NOT_CORE_THREAD;
}

void RenderAPI::Destroy()
{
	GetCoreThread().QueueCommand(std::bind(&RenderAPI::DestroyCore, this));
	GetCoreThread().SubmitAll(true);
}

void RenderAPI::DestroyCore()
{
	mActiveRenderTarget = nullptr;
}

u32 RenderAPI::VertexCountToPrimCount(DrawOperationType type, u32 elementCount)
{
	u32 primCount = 0;
	switch(type)
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

}}
