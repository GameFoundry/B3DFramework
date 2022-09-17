//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsCommandBuffer.h"
#include "BsD3D11RenderAPI.h"

namespace bs { namespace ct
{
	class D3D11EventQuery;
	
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
		void QueueCommand(const std::function<void()> command);

		/** Executes all commands in the command buffer. Not supported on secondary buffer. */
		void ExecuteCommands();

		/** @copydoc CommandBuffer::GetState() */
		CommandBufferState GetState() const override;

		/** @copydoc CommandBuffer::Reset() */
		void Reset() override;

	private:
		friend class D3D11CommandBufferManager;
		friend class D3D11RenderAPI;

		D3D11CommandBuffer(GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx, bool secondary);

		/** Returns true if the command buffer has finished executing on the GPU. */
		bool IsComplete() const;
		
		SPtr<D3D11EventQuery> mFence;
		bool mCommandQueued = false;
		bool mIsSubmitted = false;
	};

	/** @} */
}}
