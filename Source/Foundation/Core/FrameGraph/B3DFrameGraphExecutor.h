//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphCompiler.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Executes a compiled frame graph.
	 *
	 * The executor is responsible for:
	 * - Allocating command buffers from appropriate queue pools
	 * - Invoking pass execute callbacks to record GPU commands
	 * - Submitting command buffers to GPU queues
	 * - Managing resource lifetimes (Phase 4+)
	 * - Injecting synchronization barriers (Phase 3+)
	 *
	 * Phase 1 Implementation:
	 * - Executes passes sequentially in the order provided by compilation
	 * - Creates/reuses command buffers from per-queue pools
	 * - Invokes user execute callbacks with command buffers
	 * - Submits command buffers to appropriate GPU queues
	 *
	 * Future Phases:
	 * - Phase 3: Inject memory barriers and layout transitions automatically
	 * - Phase 4: Allocate and free transient resources
	 * - Phase 5: Multi-queue parallel execution with cross-queue synchronization
	 */
	class B3D_EXPORT FrameGraphExecutor
	{
	public:
		explicit FrameGraphExecutor(FrameGraph& frameGraph);
		~FrameGraphExecutor();

		/**
		 * Executes the compiled frame graph.
		 *
		 * For each pass in the compiled graph:
		 * 1. Acquires a command buffer from the appropriate queue pool
		 * 2. Invokes the pass's execute callback
		 * 3. Ends and submits the command buffer
		 *
		 * @param compiledGraph  The compiled frame graph to execute
		 */
		void Execute(const CompiledFrameGraph& compiledGraph);

		/**
		 * Resets executor state for the next frame.
		 *
		 * Phase 1: No-op (command buffer pools handle reuse automatically)
		 * Later phases: Will reset transient resource allocators
		 */
		void Reset();

	private:
		/** Executes a single pass */
		void ExecutePass(FrameGraphPass* pass);

		/** Gets the appropriate queue for a pass */
		SPtr<GpuQueue> GetQueueForPass(FrameGraphPass* pass);

		/** Gets the appropriate command buffer pool for a queue type */
		SPtr<GpuCommandBufferPool> GetPoolForQueue(GpuQueueUsage queueType);

		FrameGraph& mFrameGraph;

		// Command buffer pools (one per queue type)
		SPtr<GpuCommandBufferPool> mGraphicsCommandPool;
		SPtr<GpuCommandBufferPool> mComputeCommandPool;
		SPtr<GpuCommandBufferPool> mTransferCommandPool;
	};

	/** @} */
}
