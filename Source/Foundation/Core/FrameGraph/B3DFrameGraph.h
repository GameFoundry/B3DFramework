//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphTypes.h"
#include "B3DFrameGraphPass.h"
#include "B3DFrameGraphResource.h"
#include "RenderAPI/B3DGpuDevice.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	class FrameGraphCompiler;
	class FrameGraphExecutor;
	class CompiledFrameGraph;

	/**
	 * Frame graph for automatic resource management and synchronization.
	 *
	 * The frame graph is a high-level abstraction for managing GPU rendering work. It allows you to
	 * declaratively specify rendering passes and their resource dependencies, and the frame graph
	 * automatically handles synchronization, resource lifetime, and execution ordering.
	 *
	 * Basic Usage Pattern:
	 * @code
	 * FrameGraph graph(device);
	 *
	 * // 1. Import external resources
	 * auto backbuffer = graph.ImportTexture("Backbuffer", swapChainTexture);
	 *
	 * // 2. Declare passes
	 * graph.DeclarePass("Render",
	 *     [=](FrameGraphPass& pass) {
	 *         // Setup: Declare resource dependencies
	 *         pass.Write(backbuffer, GpuResourceUseFlag::ColorAttachment);
	 *     },
	 *     [=](GpuCommandBuffer& cmd) {
	 *         // Execute: Record GPU commands
	 *         cmd.BeginRenderPass(renderTarget);
	 *         cmd.SetPipeline(pipeline);
	 *         cmd.Draw(3, 1);
	 *         cmd.EndRenderPass();
	 *     });
	 *
	 * // 3. Compile and execute
	 * graph.Compile();
	 * graph.Execute();
	 *
	 * // 4. Reset for next frame
	 * graph.Reset();
	 * @endcode
	 *
	 * Phase 1 Implementation (Current):
	 * - Import external resources (textures/buffers)
	 * - Declare passes with resource dependencies
	 * - Basic validation of resource usage
	 * - Sequential execution in declaration order
	 * - Manual synchronization required (no automatic barriers)
	 *
	 * Phase 1 Limitations:
	 * - No automatic memory barriers or layout transitions
	 * - No dependency-based execution ordering
	 * - No transient resource allocation
	 * - No multi-queue optimization
	 * - No resource aliasing or lifetime analysis
	 *
	 * Future Phases:
	 * - Phase 2: Multiple passes with dependency analysis and optimal ordering
	 * - Phase 3: Automatic barrier insertion and layout management
	 * - Phase 4: Transient resource allocation and aliasing
	 * - Phase 5: Async compute and multi-queue optimization
	 *
	 * @note
	 * The frame graph must be compiled via Compile() before Execute() can be called.
	 * Call Reset() between frames to clear passes and resources.
	 */
	class B3D_EXPORT FrameGraph
	{
	public:
		explicit FrameGraph(GpuDevice& device);
		~FrameGraph();

		/**
		 * Imports an external texture into the frame graph.
		 *
		 * @param name      Name for debugging
		 * @param texture   Existing texture
		 * @return          Resource ID for use in pass declarations
		 */
		FrameGraphResourceId ImportTexture( // TODO - Don't split parameters over multiple lines unless it's a very long line
			const StringView& name,
			const SPtr<Texture>& texture);

		/**
		 * Imports an external buffer into the frame graph.
		 *
		 * @param name      Name for debugging
		 * @param buffer    Existing buffer
		 * @return          Resource ID for use in pass declarations
		 */
		FrameGraphResourceId ImportBuffer(
			const StringView& name,
			const SPtr<GpuBuffer>& buffer);

		/**
		 * Declares a render or compute pass.
		 *
		 * @param name          Name for debugging/profiling
		 * @param setupFunc     Lambda that declares resource dependencies
		 * @param executeFunc   Lambda that records GPU commands
		 * @param queue         Which queue to execute on (default: graphics)
		 */
		void DeclarePass(
			const StringView& name,
			FrameGraphPassSetupFunc setupFunc,
			FrameGraphPassExecuteFunc executeFunc,
			GpuQueueUsage queue = GQT_GRAPHICS);

		/**
		 * Compiles the frame graph.
		 *
		 * This method performs the following:
		 * - Executes all pass setup callbacks to declare resource dependencies
		 * - Validates that all referenced resources exist
		 * - Validates resource access patterns (read/write consistency, etc.)
		 * - Creates a compiled frame graph ready for execution
		 *
		 * Must be called after declaring all passes and before Execute().
		 * Can be called multiple times, but the previous compilation will be discarded.
		 *
		 * Phase 1: Basic validation only
		 * Later phases: Dependency analysis, topological sort, barrier calculation, lifetime analysis
		 */
		void Compile();

		/**
		 * Executes the compiled frame graph.
		 *
		 * This method performs the following for each pass:
		 * - Acquires a command buffer from the appropriate queue's pool
		 * - Invokes the pass's execute callback to record GPU commands
		 * - Submits the command buffer to the GPU queue
		 *
		 * Must be called after Compile(). Can be called multiple times with the same compilation,
		 * but typically you should Reset() and re-compile for each frame.
		 *
		 * Phase 1: Executes passes sequentially in declaration order
		 * Later phases: Executes in dependency order, potentially in parallel across multiple queues
		 */
		void Execute();

		/**
		 * Resets the frame graph for the next frame.
		 *
		 * This method:
		 * - Releases all cached render targets
		 * - Clears all resource accesses from passes
		 * - Clears all passes and resources
		 * - Resets internal state for reuse
		 *
		 * Must be called before reusing the frame graph for the next frame.
		 * After Reset(), you must re-import resources and re-declare passes.
		 */
		void Reset();

		/** Returns the GPU device */
		GpuDevice& GetDevice() { return mDevice; }

		/** Returns all resources (internal) */
		const Vector<UPtr<FrameGraphResource>>& GetResources() const { return mResources; }

		/** Returns all passes (internal) */
		const Vector<UPtr<FrameGraphPass>>& GetPasses() const { return mPasses; }

		/** Looks up a resource by ID (internal) */
		FrameGraphResource* GetResource(FrameGraphResourceId id) const;

	private:
		GpuDevice& mDevice;

		Vector<UPtr<FrameGraphResource>> mResources;
		Vector<UPtr<FrameGraphPass>> mPasses;

		UPtr<FrameGraphCompiler> mCompiler;
		UPtr<FrameGraphExecutor> mExecutor;
		UPtr<CompiledFrameGraph> mCompiledGraph;

		u32 mNextResourceId = 0;
		u32 mNextPassIndex = 0;
	};

	/** @} */
}
