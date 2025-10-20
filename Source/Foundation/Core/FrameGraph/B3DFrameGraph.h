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

	/**
	 * @page FrameGraphPhase2 Frame Graph Phase 2: Dependency Analysis
	 *
	 * Phase 2 adds automatic dependency analysis and pass ordering based on resource usage.
	 *
	 * ## Features
	 *
	 * - **Automatic Dependency Detection**: Passes are analyzed to determine Read-After-Write (RAW),
	 *   Write-After-Read (WAR), and Write-After-Write (WAW) dependencies
	 * - **Topological Sorting**: Passes are executed in an order that satisfies all dependencies
	 * - **Pass Culling**: Passes that don't contribute to final outputs are automatically removed
	 * - **Output Resources**: Resources can be explicitly marked as outputs to prevent culling
	 *
	 * ## Usage Example
	 *
	 * ```cpp
	 * FrameGraph graph(device);
	 *
	 * // Import resources
	 * auto inputTexture = graph.ImportTexture("Input", myInputTexture);
	 * auto intermediateBuffer = graph.ImportBuffer("Intermediate", myBuffer);
	 * auto outputTexture = graph.ImportTexture("Output", myOutputTexture);
	 *
	 * // Mark output
	 * graph.MarkAsOutput(outputTexture);
	 *
	 * // Pass B - depends on Pass A (will be automatically ordered)
	 * graph.DeclarePass("PassB",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Read(intermediateBuffer, GpuResourceUseFlag::Buffer, GpuAccessFlag::Read);
	 *         pass.Write(outputTexture, GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Write);
	 *     },
	 *     [=](GpuCommandBuffer& cmd) { rendering });
	 *
	 * // Pass A - produces data for Pass B
	 * graph.DeclarePass("PassA",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Read(inputTexture, GpuResourceUseFlag::Texture, GpuAccessFlag::Read);
	 *         pass.Write(intermediateBuffer, GpuResourceUseFlag::UnorderedAccess, GpuAccessFlag::Write);
	 *     },
	 *     [=](GpuCommandBuffer& cmd) { compute });
	 *
	 * // Compile: PassA and PassB will be automatically ordered (A before B)
	 * graph.Compile();
	 *
	 * // Execute in correct order
	 * graph.Execute();
	 * ```
	 *
	 * ## Dependency Types
	 *
	 * - **RAW (Read-After-Write)**: Consumer reads what producer wrote (true dependency)
	 * - **WAR (Write-After-Read)**: Consumer writes what producer read (anti-dependency)
	 * - **WAW (Write-After-Write)**: Consumer writes what producer wrote (output dependency)
	 *
	 * ## Limitations
	 *
	 * Phase 2 does not yet include:
	 * - Automatic barrier insertion (Phase 3)
	 * - Automatic layout transitions (Phase 3)
	 * - Transient resource allocation (Phase 4)
	 * - Multi-queue optimization (Phase 5)
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
	 * // 2. Declare passes (can be in any order - will be sorted automatically)
	 * graph.DeclarePass("Render",
	 *     [=](FrameGraphPass& pass) {
	 *         // Setup: Declare resource dependencies
	 *         pass.Write(backbuffer, GpuResourceUseFlag::ColorAttachment, GpuAccessFlag::Write);
	 *     },
	 *     [=](GpuCommandBuffer& cmd) {
	 *         // Execute: Record GPU commands
	 *         cmd.BeginRenderPass(renderTarget);
	 *         cmd.SetPipeline(pipeline);
	 *         cmd.Draw(3, 1);
	 *         cmd.EndRenderPass();
	 *     });
	 *
	 * // 3. Mark outputs (prevents culling)
	 * graph.MarkAsOutput(backbuffer);
	 *
	 * // 4. Compile and execute
	 * graph.Compile();
	 * graph.Execute();
	 *
	 * // 5. Reset for next frame
	 * graph.Reset();
	 * @endcode
	 *
	 * Phase 2 Implementation (Current):
	 * - Automatic dependency analysis (RAW, WAR, WAW)
	 * - Topological sorting for optimal execution order
	 * - Pass culling (removes unused passes)
	 * - Output resource marking
	 * - Resource lifetime tracking
	 * - Manual synchronization still required (no automatic barriers)
	 *
	 * Phase 2 Limitations:
	 * - No automatic memory barriers or layout transitions
	 * - No transient resource allocation
	 * - No multi-queue optimization
	 * - No resource aliasing
	 *
	 * Future Phases:
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
		 * - Analyzes dependencies between passes (RAW, WAR, WAW)
		 * - Performs topological sorting to determine execution order
		 * - Culls unused passes that don't contribute to outputs
		 * - Creates a compiled frame graph ready for execution
		 *
		 * Must be called after declaring all passes and before Execute().
		 * Can be called multiple times, but the previous compilation will be discarded.
		 *
		 * Phase 2: Dependency analysis, topological sort, pass culling
		 * Later phases: Barrier calculation, transient resource allocation
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
		 * Phase 2: Executes passes in topologically sorted order (culled passes skipped)
		 * Later phases: Automatic barrier insertion, parallel execution across multiple queues
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

		/**
		 * Mark a resource as an output.
		 * Output resources will not be culled even if no passes explicitly write to them.
		 * Use this for resources that need to be visible outside the frame graph.
		 *
		 * @param resource The resource to mark as output
		 */
		void MarkAsOutput(FrameGraphResourceId resource);

		/** Returns the GPU device */
		GpuDevice& GetDevice() { return mDevice; }

		/** Returns all resources (internal) */
		const Vector<UPtr<FrameGraphResource>>& GetResources() const { return mResources; }

		/** Returns all passes (internal) */
		const Vector<UPtr<FrameGraphPass>>& GetPasses() const { return mPasses; }

		/** Looks up a resource by ID (internal) */
		FrameGraphResource* GetResource(FrameGraphResourceId id) const;

		/**
		 * Get output resources (for debugging/inspection).
		 */
		const UnorderedSet<FrameGraphResourceId>& GetOutputResources() const
		{
			return mOutputResources;
		}

	private:
		GpuDevice& mDevice;

		Vector<UPtr<FrameGraphResource>> mResources;
		Vector<UPtr<FrameGraphPass>> mPasses;

		UPtr<FrameGraphCompiler> mCompiler;
		UPtr<FrameGraphExecutor> mExecutor;
		UPtr<CompiledFrameGraph> mCompiledGraph;

		u32 mNextResourceId = 0;
		u32 mNextPassIndex = 0;

		/** Resources explicitly marked as outputs (Phase 2) */
		UnorderedSet<FrameGraphResourceId> mOutputResources;
	};

	/** @} */
}
