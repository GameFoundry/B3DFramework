//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLCommandBuffer.h"

namespace bs { namespace ct
{
	GLCommandBuffer::GLCommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary)
		: CommandBuffer(type, deviceIdx, queueIdx, secondary)
	{
		if (deviceIdx != 0)
			BS_EXCEPT(InvalidParametersException, "Only a single device supported on OpenGL.");
	}

	void GLCommandBuffer::QueueCommand(const std::function<void()> command)
	{
#if BS_DEBUG_MODE
		if (getState() == CommandBufferState::Executing)
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

	void GLCommandBuffer::ExecuteCommands()
	{
#if BS_DEBUG_MODE
		if (mIsSecondary)
		{
			BS_LOG(Error, RenderBackend, "Cannot execute commands on a secondary buffer.");
			return;
		}

		if (getState() == CommandBufferState::Executing)
		{
			BS_LOG(Error, RenderBackend, "Cannot submit a command buffer that's still executing.");
			return;
		}
#endif

		if (glIsSync(mFence))
			glDeleteSync(mFence);

		glFlush();
		mFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		mIsSubmitted = true;
	}

	CommandBufferState GLCommandBuffer::GetState() const
	{
		if (mIsSubmitted)
			return IsComplete() ? CommandBufferState::Done : CommandBufferState::Executing;

		return mCommandQueued ? CommandBufferState::Recording : CommandBufferState::Empty;
	}

	void GLCommandBuffer::Reset()
	{
		mCommandQueued = false;
		mIsSubmitted = false;
	}

	bool GLCommandBuffer::IsComplete() const
	{
		GLenum result = glClientWaitSync(mFence, 0, 0);
		return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
	}

}}
