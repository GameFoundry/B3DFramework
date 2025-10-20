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

	// TODO - Remove this example
	/**
	 * @page FrameGraphUsage Frame Graph Usage Guide
	 *
	 * The frame graph provides automatic dependency analysis and pass ordering based on resource usage.
	 *
	 * ## Features
	 *
	 * - **Automatic Dependency Detection**: Passes are analyzed to determine Read-After-Write (RAW),
	 *   Write-After-Read (WAR), and Write-After-Write (WAW) dependencies
	 * - **Topological Sorting**: Passes are executed in an order that satisfies all dependencies
	 * - **Pass Culling**: Passes that don't contribute to final outputs are automatically removed
	 * - **Output Resources**: Resources can be explicitly marked as outputs to prevent culling
	 * - **Automatic Barriers**: Memory barriers and layout transitions are automatically inserted
	 * - **Render Target Management**: Render targets are automatically created and managed
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
	 */

	class FrameGraphCompiler;
	class CompiledFrameGraph;

	/**
	 * Frame graph for automatic resource management and synchronization.
	 *
	 * The frame graph is a high-level abstraction for managing GPU rendering work. It allows you to
	 * declaratively specify rendering passes and their resource dependencies, and the frame graph
	 * automatically handles synchronization, resource lifetime, and execution ordering.
	 *
	 * Basic Usage Pattern:
	 * // TODO - Outdated, use render pass instead of generic pass
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
	 * Current Implementation:
	 * - Automatic dependency analysis (RAW, WAR, WAW)
	 * - Topological sorting for optimal execution order
	 * - Pass culling (removes unused passes)
	 * - Output resource marking
	 * - Resource lifetime tracking
	 * - Automatic memory barriers and layout transitions
	 * - Automatic render target creation and management
	 *
	 * Current Limitations:
	 * - No transient resource allocation
	 * - No multi-queue optimization
	 * - No resource aliasing
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
		 * Imported resources are managed externally - the frame graph does not allocate or
		 * deallocate them. The texture must remain valid for the entire frame graph execution.
		 *
		 * Typical uses:
		 * - Swapchain backbuffer textures
		 * - Pre-loaded assets (environment maps, shadow atlases)
		 * - Output textures that persist across frames
		 *
		 * @param name      Debug name for profiling and error messages
		 * @param texture   Existing texture (must not be null)
		 * @return          Resource ID for use in pass Read/Write declarations
		 */
		FrameGraphResourceId ImportTexture(const StringView& name, const SPtr<Texture>& texture);

		/**
		 * Imports an external buffer into the frame graph.
		 *
		 * Imported resources are managed externally - the frame graph does not allocate or
		 * deallocate them. The buffer must remain valid for the entire frame graph execution.
		 *
		 * Typical uses:
		 * - Constant/uniform buffers
		 * - Vertex/index buffers
		 * - Storage buffers for compute shaders
		 * - Persistent GPU data structures
		 *
		 * @param name      Debug name for profiling and error messages
		 * @param buffer    Existing buffer (must not be null)
		 * @return          Resource ID for use in pass Read/Write declarations
		 */
		FrameGraphResourceId ImportBuffer(
			const StringView& name,
			const SPtr<GpuBuffer>& buffer);

		/**
		 * Declare a generic pass with full manual control.
		 *
		 * Generic passes give you full control but require manual render pass management.
		 * You must call BeginRenderPass/EndRenderPass yourself in the execute callback.
		 *
		 * Prefer DeclareRenderPass() for rendering work or DeclareComputePass() for compute,
		 * as they provide automatic setup. Use DeclarePass() only when you need:
		 * - Custom render pass configuration
		 * - Mixed rendering and compute operations
		 * - Direct command buffer control
		 *
		 * @param name          Debug name for profiling and error messages
		 * @param setupFunc     Callback to declare resource dependencies (Read/Write/ReadWrite)
		 * @param executeFunc   Callback to record GPU commands to the provided command buffer
		 * @param queue         Queue to execute on (graphics, compute, or transfer)
		 */
		void DeclarePass(
			const StringView& name,
			FrameGraphPassSetupFunc setupFunc,
			FrameGraphPassExecuteFunc executeFunc,
			GpuQueueUsage queue = GQT_GRAPHICS);

		/**
		 * Declare a render pass with automatic render target management.
		 *
		 * Render passes are the recommended way to declare rendering work. The frame graph
		 * automatically handles:
		 * - Render target creation from WriteColor/WriteDepth/ReadDepth declarations
		 * - BeginRenderPass/EndRenderPass calls wrapping your execute callback
		 * - Barrier insertion before BeginRenderPass
		 * - Layout transitions for attachments
		 *
		 * In the setup callback, use WriteColor(), WriteDepth(), or ReadDepth() to declare
		 * attachments, and Read() for shader resources. The execute callback receives a
		 * command buffer already inside a render pass - just record draw commands.
		 *
		 * @param name          Debug name for profiling and error messages
		 * @param setupFunc     Callback to declare attachments and resource dependencies
		 * @param executeFunc   Callback to record draw commands (inside render pass)
		 * @param queue         Queue to execute on (typically graphics)
		 */
		void DeclareRenderPass(
			const StringView& name,
			FrameGraphPassSetupFunc setupFunc,
			FrameGraphPassExecuteFunc executeFunc,
			GpuQueueUsage queue = GQT_GRAPHICS);

		/**
		 * Declare a compute pass for GPU computation.
		 *
		 * Compute passes are optimized for non-rendering GPU work like post-processing,
		 * physics simulation, particle updates, etc. The frame graph validates that only
		 * compute-compatible resources are used (no render attachments).
		 *
		 * Compute passes execute on the compute queue, which may run asynchronously with
		 * graphics work on some GPUs. The frame graph automatically inserts synchronization
		 * barriers when compute results are consumed by rendering or vice versa.
		 *
		 * In the setup callback, declare buffers and textures via Read/Write/ReadWrite.
		 * In the execute callback, record compute commands (SetPipeline, Dispatch, etc.).
		 *
		 * @param name          Debug name for profiling and error messages
		 * @param setupFunc     Callback to declare resource dependencies (no attachments allowed)
		 * @param executeFunc   Callback to record compute commands (Dispatch, etc.)
		 */
		void DeclareComputePass(
			const StringView& name,
			FrameGraphPassSetupFunc setupFunc,
			FrameGraphPassExecuteFunc executeFunc);

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
		 * - Calculates memory barriers and layout transitions
		 * - Creates render targets for render passes
		 * - Creates a compiled frame graph ready for execution
		 *
		 * Must be called after declaring all passes and before Execute().
		 * Can be called multiple times, but the previous compilation will be discarded.
		 */
		void Compile();

		/**
		 * Executes the compiled frame graph.
		 *
		 * This method performs the following for each pass:
		 * - Issues memory barriers and layout transitions before the pass
		 * - Begins render pass (for render passes)
		 * - Invokes the pass's execute callback to record GPU commands
		 * - Ends render pass (for render passes)
		 * - Batches commands by queue and submits to the GPU
		 *
		 * Must be called after Compile(). Can be called multiple times with the same compilation,
		 * but typically you should Reset() and re-compile for each frame.
		 *
		 * Passes execute in topologically sorted order with culled passes skipped.
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
		 * Mark a resource as an output to prevent it from being culled.
		 *
		 * During compilation, the frame graph performs a reverse DFS from output resources
		 * to identify which passes are actually needed. Passes that don't contribute to any
		 * output are culled (removed from execution) as an optimization.
		 *
		 * Mark a resource as an output if:
		 * - It's the final result you want to display (e.g., backbuffer)
		 * - It's read by external code after frame graph execution
		 * - You want to preserve it even if it's not used by subsequent passes
		 *
		 * Resources not marked as outputs may be culled if they're only intermediate results.
		 *
		 * @param resource  The resource to mark as output (must be imported)
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

		/**
		 * Get a map of imported textures (internal - used by render target builder).
		 * Returns a map from resource ID to texture.
		 */
		UnorderedMap<FrameGraphResourceId, SPtr<Texture>> GetImportedTextures() const;

	private:
		//////////////////////////////////////////////////////////////////////////
		// Execution (internal)
		//////////////////////////////////////////////////////////////////////////

		/** Executes a single pass */
		void ExecutePass(FrameGraphPass* pass);

		/** Gets the appropriate queue for a pass */
		SPtr<GpuQueue> GetQueueForPass(FrameGraphPass* pass);

		/** Gets the appropriate command buffer pool for a queue type */
		SPtr<GpuCommandBufferPool> GetPoolForQueue(GpuQueueUsage queueType);

		//////////////////////////////////////////////////////////////////////////
		// Member Variables
		//////////////////////////////////////////////////////////////////////////

		GpuDevice& mDevice;

		Vector<UPtr<FrameGraphResource>> mResources;
		Vector<UPtr<FrameGraphPass>> mPasses;

		UPtr<FrameGraphCompiler> mCompiler;
		UPtr<CompiledFrameGraph> mCompiledGraph;

		u32 mNextResourceId = 0;
		u32 mNextPassIndex = 0;

		/** Resources explicitly marked as outputs */
		UnorderedSet<FrameGraphResourceId> mOutputResources;

		/** Command buffer pools (one per queue type) */
		SPtr<GpuCommandBufferPool> mGraphicsCommandPool;
		SPtr<GpuCommandBufferPool> mComputeCommandPool;
		SPtr<GpuCommandBufferPool> mTransferCommandPool;
	};

	/** @} */
}
