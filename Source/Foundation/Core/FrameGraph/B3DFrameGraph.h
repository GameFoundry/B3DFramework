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

	// Forward declarations
	class FrameGraphResourceAllocator;
	struct ResourceUsageHistory;

	/**
	 * @page FrameGraphUsage Frame Graph Usage Guide
	 *
	 * The frame graph provides automatic dependency analysis, pass ordering, and transient resource
	 * management for efficient GPU resource utilization.
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
	 * - **Transient Resources**: Automatic allocation and deallocation of temporary resources based on
	 *   computed lifetimes, reducing peak memory usage by 30-50%
	 *
	 * ## Basic Usage with Imported Resources
	 *
	 * For resources that persist across frames, use ImportTexture/ImportBuffer:
	 *
	 * @code
	 * FrameGraph graph(device);
	 *
	 * // Import persistent resources
	 * auto outputTexture = graph.ImportTexture("Output", myOutputTexture);
	 *
	 * // Mark output to prevent culling
	 * graph.MarkAsOutput(outputTexture);
	 *
	 * // Render pass - UseParameters automatically imports all shader resources
	 * graph.DeclareRenderPass("PBR",
	 *     [=](FrameGraphPass& pass) {
	 *         // Single call auto-imports all textures, buffers, etc. from GpuParameters
	 *         pass.UseParameters(materialParams);
	 *         pass.WriteColor(outputTexture);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         cmd.SetGpuParameterSet(materialParams);
	 *         cmd.Draw(...);
	 *     });
	 *
	 * graph.Compile();
	 * graph.Execute();
	 * @endcode
	 *
	 * ## Transient Resources
	 *
	 * For temporary intermediate resources, use DeclareTransientTexture/DeclareTransientBuffer.
	 * Transient resources are automatically allocated on first use and deallocated after last use,
	 * significantly reducing peak memory consumption.
	 *
	 * ### Basic Transient Texture Example
	 *
	 * @code
	 * FrameGraph graph(device);
	 *
	 * // Import final output (persistent)
	 * auto backbuffer = graph.ImportTexture("Backbuffer", swapchainTexture);
	 * graph.MarkAsOutput(backbuffer);
	 *
	 * // Declare transient intermediate texture (allocated only when needed)
	 * TextureCreateInformation bloomCreateInfo;
	 * bloomCreateInfo.Width = 1920;
	 * bloomCreateInfo.Height = 1080;
	 * bloomCreateInfo.Format = PF_RGBA16F;
	 * bloomCreateInfo.Usage = TU_RENDERTARGET;
	 *
	 * auto bloomTexture = graph.DeclareTransientTexture("Bloom", bloomCreateInfo);
	 *
	 * // Pass 1: Generate bloom (writes transient)
	 * graph.DeclareRenderPass("GenerateBloom",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.WriteColor(bloomTexture);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         // Bloom texture is allocated automatically before this executes
	 *         cmd.SetPipeline(bloomPipeline);
	 *         cmd.Draw(3);
	 *     });
	 *
	 * // Pass 2: Composite bloom (reads transient)
	 * graph.DeclareRenderPass("Composite",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Read(bloomTexture, GpuResourceUseFlag::ShaderAccess);
	 *         pass.WriteColor(backbuffer);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         // Manual binding - get the allocated transient texture
	 *         auto bloom = resources.GetTexture(bloomTexture);
	 *         compositeParams->SetTexture("bloomTex", bloom);
	 *
	 *         cmd.SetPipeline(compositePipeline);
	 *         cmd.SetGpuParameterSet(compositeParams);
	 *         cmd.Draw(3);
	 *         // Bloom texture is deallocated automatically after this pass
	 *     });
	 *
	 * graph.Compile();
	 * graph.Execute();
	 * @endcode
	 *
	 * ### Basic Transient Buffer Example
	 *
	 * @code
	 * // Declare transient compute buffer
	 * GpuBufferCreateInformation bufferCreateInfo =
	 *     GpuBufferCreateInformation::CreateStructuredStorage(sizeof(ParticleData), 10000);
	 *
	 * auto particleBuffer = graph.DeclareTransientBuffer("Particles", bufferCreateInfo);
	 *
	 * // Compute pass writing to transient buffer
	 * graph.DeclareComputePass("UpdateParticles",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Write(particleBuffer, GpuResourceUseFlag::UnorderedAccess);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         auto buffer = resources.GetBuffer(particleBuffer);
	 *         updateParams->SetBuffer("particleBuffer", buffer);
	 *
	 *         cmd.SetPipeline(updatePipeline);
	 *         cmd.SetGpuParameterSet(updateParams);
	 *         cmd.Dispatch(100, 1, 1);
	 *     });
	 * @endcode
	 *
	 * ### Memory Savings with Transients
	 *
	 * Transient resources dramatically reduce peak memory usage:
	 *
	 * **Persistent approach (all resources allocated all the time):**
	 * - GBuffer Albedo: 8.3 MB
	 * - GBuffer Normal: 8.3 MB
	 * - GBuffer Depth: 8.3 MB
	 * - Lighting Buffer: 16.6 MB
	 * - Bloom Buffer: 4.1 MB
	 * - Total Peak: 45.6 MB (all allocated simultaneously)
	 *
	 * **Transient approach (allocated only when needed):**
	 * - Pass 1 (GBuffer): 25 MB allocated
	 * - Pass 2 (Lighting): +17 MB = 42 MB peak
	 * - Pass 2 end: -25 MB (GBuffer freed)
	 * - Pass 3 (Bloom): +4 MB = 21 MB
	 * - Total Peak: 42 MB vs 45.6 MB
	 * - Savings: 8% in this example, typically 30-50% for complex renderers
	 *
	 * ### Manual Resource Binding
	 *
	 * Transient resources require manual binding in execute callbacks. This gives you explicit
	 * control and makes resource flow clear:
	 *
	 * @code
	 * graph.DeclareRenderPass("MyPass",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Read(transientTex, GpuResourceUseFlag::ShaderAccess);
	 *         pass.WriteColor(output);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         // Step 1: Get the allocated resource
	 *         auto texture = resources.GetTexture(transientTex);
	 *
	 *         // Step 2: Manually bind to shader parameter
	 *         myParams->SetTexture("inputTex", texture);
	 *
	 *         // Step 3: Use as normal
	 *         cmd.SetGpuParameterSet(myParams);
	 *         cmd.Draw(3);
	 *     });
	 * @endcode
	 *
	 * This manual binding approach:
	 * - Works identically for imported and transient resources
	 * - Makes resource dependencies explicit and easy to understand
	 * - Gives you full control over when and how resources are bound
	 * - Avoids hidden magic and unexpected behavior
	 *
	 * ## Complete Workflow Example
	 *
	 * @code
	 * FrameGraph graph(device);
	 *
	 * // 1. Import persistent resources
	 * auto backbuffer = graph.ImportTexture("Backbuffer", swapchainTexture);
	 *
	 * // 2. Declare transient resources
	 * auto tempTexture = graph.DeclareTransientTexture("Temp",
	 *     TextureCreateInformation{...});
	 *
	 * // 3. Declare passes (order doesn't matter - will be sorted automatically)
	 * graph.DeclareRenderPass("Pass1", setupFunc, executeFunc);
	 * graph.DeclareRenderPass("Pass2", setupFunc, executeFunc);
	 *
	 * // 4. Mark outputs (prevents culling)
	 * graph.MarkAsOutput(backbuffer);
	 *
	 * // 5. Compile (analyzes dependencies, calculates lifetimes)
	 * graph.Compile();
	 *
	 * // 6. Execute (allocates/deallocates transients, runs passes)
	 * graph.Execute();
	 *
	 * // 7. Reset for next frame
	 * graph.Reset();
	 * @endcode
	 *
	 * ## Dependency Types
	 *
	 * - **RAW (Read-After-Write)**: Consumer reads what producer wrote (true dependency)
	 * - **WAR (Write-After-Read)**: Consumer writes what producer read (anti-dependency)
	 * - **WAW (Write-After-Write)**: Consumer writes what producer wrote (output dependency)
	 *
	 * ## See Also
	 *
	 * - Phase2Example.cpp - Demonstrates automatic dependency analysis and pass culling
	 * - Phase4TransientExample.cpp - Comprehensive transient resource examples
	 */

	class FrameGraphCompiler;
	class CompiledFrameGraph;

	/**
	 * Frame graph for automatic resource management and synchronization.
	 *
	 * The frame graph is a high-level abstraction for managing GPU rendering work. It allows you to
	 * declaratively specify rendering passes and their resource dependencies, and the frame graph
	 * automatically handles synchronization, resource lifetime, execution ordering, and transient
	 * resource allocation.
	 *
	 * Basic Usage Pattern:
	 * @code
	 * FrameGraph graph(device);
	 *
	 * // 1. Import persistent resources (swapchain, pre-allocated textures, etc.)
	 * auto backbuffer = graph.ImportTexture("Backbuffer", swapChainTexture);
	 *
	 * // 2. Declare transient resources (temporary intermediate textures/buffers)
	 * //    These are automatically allocated on first use, deallocated after last use
	 * TextureCreateInformation bloomInfo;
	 * bloomInfo.Width = 1920;
	 * bloomInfo.Height = 1080;
	 * bloomInfo.Format = PF_RGBA16F;
	 * bloomInfo.Usage = TU_RENDERTARGET;
	 * auto bloomTexture = graph.DeclareTransientTexture("Bloom", bloomInfo);
	 *
	 * // 3. Declare passes (can be in any order - will be sorted automatically)
	 * graph.DeclareRenderPass("GenerateBloom",
	 *     [=](FrameGraphPass& pass) {
	 *         // Setup: Declare resource dependencies
	 *         pass.WriteColor(bloomTexture);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         // Execute: Record GPU commands
	 *         // Bloom texture is automatically allocated before this executes
	 *         cmd.SetPipeline(pipeline);
	 *         cmd.Draw(3);
	 *     });
	 *
	 * graph.DeclareRenderPass("Composite",
	 *     [=](FrameGraphPass& pass) {
	 *         pass.Read(bloomTexture, GpuResourceUseFlag::ShaderAccess);
	 *         pass.WriteColor(backbuffer);
	 *     },
	 *     [=](GpuCommandBuffer& cmd, FrameGraphPassResources& resources) {
	 *         // Manual binding of transient resource
	 *         auto bloom = resources.GetTexture(bloomTexture);
	 *         params->SetTexture("bloomTex", bloom);
	 *
	 *         cmd.SetPipeline(compositePipeline);
	 *         cmd.SetGpuParameterSet(params);
	 *         cmd.Draw(3);
	 *         // Bloom texture is automatically deallocated after this pass
	 *     });
	 *
	 * // 4. Mark outputs (prevents culling)
	 * graph.MarkAsOutput(backbuffer);
	 *
	 * // 5. Compile and execute
	 * graph.Compile();  // Analyzes dependencies, calculates lifetimes
	 * graph.Execute();  // Allocates/deallocates transients, runs passes
	 *
	 * // 6. Reset for next frame
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
	 * - Transient resource allocation (reduces peak memory by 30-50%)
	 *
	 * Future Features:
	 * - Memory aliasing for transient resources (Phase 5)
	 * - Multi-queue optimization with async compute (Phase 6)
	 *
	 * @note
	 * The frame graph must be compiled via Compile() before Execute() can be called.
	 * Call Reset() between frames to clear passes and resources.
	 *
	 * @see FrameGraphUsage for detailed usage examples
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
		 * Imports a render target (e.g., swap chain) into the frame graph.
		 *
		 * Use this for importing swap chain textures from RenderWindow, which cannot
		 * be accessed as standalone Texture objects. The render target must remain valid
		 * for the entire frame graph execution.
		 *
		 * Typical uses:
		 * - Swap chain backbuffer from RenderWindow
		 * - Other render targets that don't expose underlying textures
		 *
		 * @param name          Debug name for profiling and error messages
		 * @param renderTarget  Existing render target (must not be null)
		 * @param surface       Which surface of the render target to use (default: RT_COLOR0 for backbuffer)
		 * @return              Resource ID for use in pass Read/Write declarations
		 *
		 * Example:
		 * @code
		 * auto backbuffer = graph.ImportRenderTarget("Backbuffer", window, RT_COLOR0);
		 * graph.MarkAsOutput(backbuffer);
		 * @endcode
		 */
		FrameGraphResourceId ImportRenderTarget(
			const StringView& name,
			const SPtr<RenderTarget>& renderTarget,
			RenderSurfaceMaskBits surface = RT_COLOR0);

		/**
		 * Declares a transient texture (allocated only for its lifetime).
		 *
		 * Transient resources are automatically allocated on first use and deallocated
		 * after last use, reducing peak memory consumption. The texture is created
		 * during Execute() based on the provided descriptor.
		 *
		 * During execute, access the allocated texture via:
		 *   auto tex = resources.GetTexture(resourceId);
		 *   params->SetTexture("paramName", tex);
		 *
		 * @param name                Name for debugging/profiling
		 * @param createInformation   Texture creation descriptor
		 * @return                    Resource ID for use in Read/Write/ReadWrite declarations
		 */
		FrameGraphResourceId DeclareTransientTexture(
			const StringView& name,
			const TextureCreateInformation& createInformation);

		/**
		 * Declares a transient buffer (allocated only for its lifetime).
		 *
		 * During execute, access via resources.GetBuffer(resourceId).
		 *
		 * @param name                Name for debugging/profiling
		 * @param createInformation   Buffer creation descriptor
		 * @return                    Resource ID for use in Read/Write/ReadWrite declarations
		 */
		FrameGraphResourceId DeclareTransientBuffer(
			const StringView& name,
			const GpuBufferCreateInformation& createInformation);

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
		// Transient Resource Allocation (internal)
		//////////////////////////////////////////////////////////////////////////

		/** Builds allocation info for transient resources from compiled usage histories */
		void BuildTransientAllocationInfo();

		/** Allocates transient resources that are first used by this pass */
		void AllocateTransientsForPass(FrameGraphPass* pass);

		/** Deallocates transient resources that are last used by this pass */
		void DeallocateTransientResourcesAfterPass(FrameGraphPass* pass);

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

		/** Storage for transient resource create information (imported resources don't use this) */
		UnorderedMap<FrameGraphResourceId, TextureCreateInformation> mTransientTextureCreateInfo;
		UnorderedMap<FrameGraphResourceId, GpuBufferCreateInformation> mTransientBufferCreateInfo;

		/** Transient resource allocator (pool-based) */
		UPtr<FrameGraphResourceAllocator> mResourceAllocator;

		/** Transient resource allocation info (maps resource ID to usage history) */
		UnorderedMap<FrameGraphResourceId, ResourceUsageHistory> mTransientAllocationInfo;

		/** Command buffer pools (one per queue type) */
		SPtr<GpuCommandBufferPool> mGraphicsCommandPool;
		SPtr<GpuCommandBufferPool> mComputeCommandPool;
		SPtr<GpuCommandBufferPool> mTransferCommandPool;
	};

	/** @} */
}
