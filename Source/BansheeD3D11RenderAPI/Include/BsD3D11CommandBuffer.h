//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsCommandBuffer.h"
#include "BsD3D11RenderAPI.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**
	 * Command buffer implementation for DirectX 11, which doesn't support multi-threaded command generation. Instead all
	 * commands are stored in an internal buffer, and then sent to the actual render API when the buffer is executed.
	 */
	class D3D11CommandBuffer : public CommandBuffer
	{
	public:
		/** Registers a new command in the command buffer. */
		void queueCommand(const std::function<void()> command);

		/** Appends all commands from the secondary buffer into this command buffer. */
		void appendSecondary(const SPtr<D3D11CommandBuffer>& secondaryBuffer);

		/** Executes all commands in the command buffer. Not supported on secondary buffer. */
		void executeCommands();

		/** Removes all commands from the command buffer. */
		void clear();

	private:
		friend class D3D11CommandBufferManager;
		friend class D3D11RenderAPI;

		D3D11CommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary);

		Vector<std::function<void()>> mCommands;

		DrawOperationType mActiveDrawOp;
	};

	/** @} */
}