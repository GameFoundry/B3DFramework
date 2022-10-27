//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11CommandBuffer.h"
#include "BsD3D11Device.h"
#include "BsD3D11EventQuery.h"

using namespace bs;
using namespace bs::ct;

D3D11CommandBuffer::D3D11CommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary)
	: CommandBuffer(type, deviceIdx, queueIdx, secondary)
{
	if(deviceIdx != 0)
		BS_EXCEPT(InvalidParametersException, "Only a single device supported on DX11.");
}

void D3D11CommandBuffer::QueueCommand(const std::function<void()> command)
{
#if BS_DEBUG_MODE
	if(GetState() == CommandBufferState::Executing)
	{
		BS_LOG(Error, RenderBackend, "Cannot modify a command buffer that's still executing.");
		return;
	}
#endif

	// We don't support command buffer queuing on DX11, so we just execute the command right away. This means
	// if caller uses a non-main command buffer the behaviour will likely be incorrect. To properly support
	// command queuing we'd need to remember state of GpuParams when first bound and handles updates to
	// buffers after they are bound (and potentially other things).
	command();
	mCommandQueued = true;
}

void D3D11CommandBuffer::ExecuteCommands()
{
#if BS_DEBUG_MODE
	if(mIsSecondary)
	{
		BS_LOG(Error, RenderBackend, "Cannot execute commands on a secondary buffer.");
		return;
	}

	if(GetState() == CommandBufferState::Executing)
	{
		BS_LOG(Error, RenderBackend, "Cannot submit a command buffer that's still executing.");
		return;
	}
#endif

	mFence = bs_shared_ptr_new<D3D11EventQuery>(mDeviceIdx);
	mFence->Begin();
	mIsSubmitted = true;
}

CommandBufferState D3D11CommandBuffer::GetState() const
{
	if(mIsSubmitted)
		return IsComplete() ? CommandBufferState::Done : CommandBufferState::Executing;

	return mCommandQueued ? CommandBufferState::Recording : CommandBufferState::Empty;
}

void D3D11CommandBuffer::Reset()
{
	mCommandQueued = false;
	mIsSubmitted = false;
}

bool D3D11CommandBuffer::IsComplete() const
{
	return mFence && mFence->IsReady();
}
