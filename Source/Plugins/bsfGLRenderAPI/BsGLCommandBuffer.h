//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "BsGLRenderAPI.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Command buffer implementation for OpenGL, which doesn't support multi-threaded command generation. Instead all
	 * commands are stored in an internal buffer, and then sent to the actual render API when the buffer is executed.
	 */
	class GLCommandBuffer : public CommandBuffer
	{
	public:
		/** Registers a new command in the command buffer. */
		void queueCommand(const std::function<void()> command);

		/** Appends all commands from the secondary buffer into this command buffer. */
		void appendSecondary(const SPtr<GLCommandBuffer>& secondaryBuffer);

		/** Executes all commands in the command buffer. Not supported on secondary buffer. */
		void executeCommands();

		/** Removes all commands from the command buffer. */
		void clear();

	private:
		friend class GLCommandBufferManager;
		friend class GLRenderAPI;

		GLCommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary);

		Vector<std::function<void()>> mCommands;

		DrawOperationType mCurrentDrawOperation = DOT_TRIANGLE_LIST;
	};

	/** @} */
}}