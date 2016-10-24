//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsCommandBuffer.h"
#include "BsDrawOps.h"
#include "BsGLRenderAPI.h"

namespace BansheeEngine
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

		GLCommandBuffer(CommandBufferType type, UINT32 deviceIdx, UINT32 syncMask, bool secondary);

		UINT32 mDeviceIdx;
		Vector<std::function<void()>> mCommands;

		DrawOperationType mCurrentDrawOperation;
	};

	/** @} */
}