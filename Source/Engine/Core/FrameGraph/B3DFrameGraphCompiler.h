//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphPass.h"
#include "B3DFrameGraphTypes.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Represents a single use of a resource within a pass.
	 *
	 * Records a snapshot of how a resource is accessed at a specific point in the frame graph.
	 * Combines the pass that performs the access, the usage flags (how it's used), access flags
	 * (read/write), and the image layout required for that usage. Used to build chronological
	 * usage histories for barrier generation.
	 */
	struct ResourceUsage
	{
		FrameGraphPass* Pass = nullptr;
		GpuResourceUseFlags Usage;
		GpuAccessFlags Access;
		ImageLayout Layout = ImageLayout::Undefined;

		ResourceUsage() = default;

		ResourceUsage(FrameGraphPass* pass, GpuResourceUseFlags usage, GpuAccessFlags access, ImageLayout layout = ImageLayout::Undefined)
			: Pass(pass), Usage(usage), Access(access), Layout(layout)
		{}
	};

	/**
	 * Complete chronological history of how a resource is used throughout the frame graph.
	 *
	 * Maintains a timeline of all accesses to a resource in execution order. Used by the
	 * barrier generation pipeline to determine where synchronization and layout transitions
	 * are needed. Each entry in the history represents one access by one pass.
	 */
	struct ResourceUsageHistory
	{
		FrameGraphResourceId Resource;
		Vector<ResourceUsage> Uses;
	};

	/**
	 * Represents a required transition between two consecutive resource usages.
	 *
	 * Captures the source and destination states needed to generate a GPU barrier. When a
	 * resource transitions from one usage to another (e.g., from shader read to render target
	 * write), this struct records both states along with the passes involved. The barrier
	 * generation pass converts these transitions into actual GPU barrier commands.
	 */
	struct ResourceTransition
	{
		FrameGraphResourceId Resource;
		FrameGraphPass* SourcePass;
		FrameGraphPass* DestinationPass;

		GpuResourceUseFlags SourceUsage;
		GpuAccessFlags SourceAccess;
		ImageLayout SourceLayout = ImageLayout::Undefined;

		GpuResourceUseFlags DestinationUsage;
		GpuAccessFlags DestinationAccess;
		ImageLayout DestinationLayout = ImageLayout::Undefined;

		ResourceTransition() = default;

		ResourceTransition(
			FrameGraphResourceId resource,
			FrameGraphPass* sourcePass,
			FrameGraphPass* destinationPass,
			GpuResourceUseFlags sourceUsage,
			GpuAccessFlags sourceAccess,
			ImageLayout sourceLayout,
			GpuResourceUseFlags destinationUsage,
			GpuAccessFlags destinationAccess,
			ImageLayout destinationLayout)
			: Resource(resource)
			, SourcePass(sourcePass)
			, DestinationPass(destinationPass)
			, SourceUsage(sourceUsage)
			, SourceAccess(sourceAccess)
			, SourceLayout(sourceLayout)
			, DestinationUsage(destinationUsage)
			, DestinationAccess(destinationAccess)
			, DestinationLayout(destinationLayout)
		{}
	};

	/**
	 * A batch of barriers to be issued before a specific pass.
	 *
	 * Combines all required memory barriers and layout transitions for resources accessed
	 * by a pass into a single GpuBarriers structure. Batching barriers improves efficiency
	 * by reducing the number of barrier commands issued to the GPU. During execution, all
	 * barriers in a batch are issued together before the destination pass begins.
	 */
	struct FrameGraphBarrierBatch
	{
		FrameGraphPass* DestinationPass = nullptr;
		GpuBarriers Barriers;
	};

	/**
	 * Helper functions for determining optimal image layouts based on resource usage.
	 *
	 * Provides utilities to map high-level resource usage patterns (e.g., "color attachment",
	 * "shader read") to the specific image layouts required by Vulkan and D3D12. Using
	 * optimal layouts improves GPU performance and is required for correctness.
	 */
	class B3D_EXPORT FrameGraphLayoutHelper
	{
	public:
		/**
		 * Determines the optimal image layout for a given resource usage.
		 *
		 * Maps resource usage patterns to the appropriate Vulkan/D3D12 image layout.
		 * For example:
		 * - ColorAttachment → ColorAttachment layout
		 * - DepthStencil (read-only) → DepthStencilReadOnly layout
		 * - DepthStencil (read-write) → DepthStencil layout
		 * - ShaderAccess (read) → ShaderReadOnly layout
		 *
		 * Choosing optimal layouts improves GPU performance by matching hardware expectations.
		 *
		 * @param usage   Resource usage flags (how resource is used)
		 * @param access  Access flags (read/write/both)
		 * @return        Optimal layout for the specified usage pattern
		 */
		static ImageLayout GetLayoutForUsage(GpuResourceUseFlags usage, GpuAccessFlags access);

		/**
		 * Checks if a layout transition is required.
		 *
		 * @param sourceLayout		Current layout
		 * @param destinationLayout	Desired layout
		 * @return					True if transition is needed
		 */
		static bool RequiresTransition(ImageLayout sourceLayout, ImageLayout destinationLayout);
	};

	/**
	 * Compiled frame graph output - the result of the compilation pipeline.
	 *
	 * Contains everything needed to execute the frame graph:
	 * - Passes in topologically sorted execution order (culled passes removed)
	 * - Barrier batches to issue before each pass
	 * - Usage histories for all resources (for debugging/validation)
	 * - Render targets created for render passes
	 *
	 * This is the executable form of the frame graph. Execute() consumes this to run the frame.
	 */
	class B3D_EXPORT CompiledFrameGraph
	{
	public:
		/** Passes in topologically sorted execution order (culled passes removed) */
		Vector<FrameGraphPass*> SortedPasses;

		/** Barrier batches to issue before passes */
		Vector<FrameGraphBarrierBatch> BarrierBatches;

		/** Complete usage history for all resources. */
		UnorderedMap<FrameGraphResourceId, ResourceUsageHistory> UsageHistories;

		/** Render targets for render passes, created during compilation. */
		UnorderedMap<FrameGraphPass*, SPtr<RenderTarget>> RenderTargets; // TODO - Store RenderTargets in passes directly?
	};

	/**
	 * Compiles a frame graph into an executable form.
	 *
	 * The compiler is responsible for:
	 * - Executing pass setup callbacks to collect resource dependencies
	 * - Validating resource usage and access patterns
	 * - Analyzing dependencies and computing execution order
	 * - Culling unused passes
	 * - Calculating synchronization barriers and layout transitions
	 * - Creating render targets for render passes
	 * - Performing resource lifetime analysis
	 *
	 * All compilation logic is contained within this single class for simplicity.
	 */
	class B3D_EXPORT FrameGraphCompiler
	{
	public:
		explicit FrameGraphCompiler(FrameGraph& frameGraph);

		/**
		 * Compiles the frame graph into an executable form.
		 *
		 * This method performs the complete compilation pipeline:
		 * 1. Invokes all pass setup callbacks to collect resource dependencies
		 * 2. Validates resource existence and access patterns
		 * 3. Analyzes dependencies to detect RAW/WAR/WAW hazards
		 * 4. Topologically sorts passes to satisfy dependencies
		 * 5. Culls passes that don't contribute to output resources
		 * 6. Calculates memory barriers and layout transitions for synchronization
		 * 7. Creates render targets for render passes
		 *
		 * If any validation step fails, compilation is aborted and nullptr is returned.
		 * The frame graph will log detailed error messages indicating the failure reason.
		 *
		 * @return  Compiled frame graph ready for execution, or nullptr if validation failed
		 */
		UPtr<CompiledFrameGraph> Compile();

	private:
		//////////////////////////////////////////////////////////////////////////
		// Validation
		//////////////////////////////////////////////////////////////////////////

		/** Executes setup functions for all passes */
		void ExecuteSetupFunctions();

		/**
		 * Validates the entire frame graph after setup callbacks have executed.
		 *
		 * Checks that all resources referenced by passes exist in the frame graph,
		 * validates access patterns are consistent with resource types, and ensures
		 * render passes have valid attachment configurations.
		 *
		 * @return false if validation fails (logs detailed error), preventing compilation
		 */
		bool Validate();

		/**
		 * Validates that a single pass has consistent resource accesses.
		 *
		 * Ensures all resources accessed by the pass have been imported, that access
		 * patterns are consistent (e.g., render attachments must be textures with appropriate
		 * formats, compute passes don't use render target attachments), and that read/write
		 * flags match the declared usage patterns.
		 *
		 * @return false if validation fails, preventing graph compilation
		 */
		bool ValidatePass(FrameGraphPass* pass);

		/** Validates a resource access */
		bool ValidateResourceAccess(FrameGraphPass* pass, const FrameGraphResourceAccess& access);

		/** Validates dependencies (checks for isolated passes, etc.) */
		bool ValidateDependencies();

		/** Validates render pass setup */
		bool ValidateRenderPasses();

		/** Validates layouts */
		bool ValidateLayouts(const CompiledFrameGraph& compiledGraph);

		//////////////////////////////////////////////////////////////////////////
		// Dependency Analysis & Scheduling
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Analyzes resource access patterns to build the pass dependency graph.
		 *
		 * Examines all resource accesses to detect three types of hazards:
		 * - RAW (Read-After-Write): Consumer reads what producer wrote - true dependency
		 * - WAR (Write-After-Read): Consumer writes what producer read - anti-dependency
		 * - WAW (Write-After-Write): Consumer writes what producer wrote - output dependency
		 *
		 * Creates explicit dependency edges between passes, which are then used for
		 * topological sorting to determine execution order. The dependency graph ensures
		 * passes execute in an order that preserves data correctness.
		 *
		 * @return false if dependency analysis reveals structural issues
		 */
		bool AnalyzeDependencies();

		/**
		 * Tracks resource lifetimes across the frame graph execution.
		 *
		 * Records the first and last pass that accesses each resource. This information
		 * will be used for transient resource allocation in the future - transient resources
		 * can be allocated just before first use and released after last use, enabling
		 * memory aliasing optimizations.
		 *
		 * Currently used for validation and debugging only.
		 */
		void TrackResourceLifetimes();

		/**
		 * Removes passes that don't contribute to output resources via reverse depth-first search.
		 *
		 * Starting from resources marked via MarkAsOutput(), performs a reverse DFS through
		 * the dependency graph to mark all passes that contribute to outputs. Passes not
		 * reached by this traversal are marked as culled and won't execute, improving
		 * performance when intermediate results aren't used.
		 *
		 * Preserves all passes in the dependency chain leading to output resources.
		 */
		void CullUnusedPasses();

		/**
		 * Topologically sorts passes using Kahn's algorithm to determine execution order.
		 *
		 * Orders passes such that all dependencies are satisfied - a pass only executes
		 * after all passes it depends on have completed. Uses Kahn's algorithm which
		 * maintains a queue of passes with no remaining dependencies and processes them
		 * in order.
		 *
		 * If the algorithm cannot order all non-culled passes, a dependency cycle exists.
		 * The method will log the passes involved in the cycle to aid debugging.
		 *
		 * @param outSortedPasses  Receives the sorted pass list
		 * @return false if a dependency cycle is detected, preventing execution
		 */
		bool TopologicalSort(Vector<FrameGraphPass*>& outSortedPasses);

		//////////////////////////////////////////////////////////////////////////
		// Barrier Generation & Synchronization
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Builds chronological usage history for all resources across all passes.
		 *
		 * Creates a timeline of how each resource is accessed throughout frame execution,
		 * recording the usage flags, access type (read/write), and image layout for each use.
		 * This history is used to determine where synchronization barriers and layout
		 * transitions are needed.
		 *
		 * @param passes  Sorted list of passes in execution order
		 */
		void BuildUsageHistory(const Vector<FrameGraphPass*>& passes);

		/** Determines the appropriate layout for a resource usage */
		ImageLayout DetermineLayout(GpuResourceUseFlags usage, GpuAccessFlags access) const;

		/**
		 * Builds required resource transitions from usage history.
		 *
		 * Analyzes the chronological usage history of each resource to identify points
		 * where synchronization is required (access type changes, layout transitions, etc.).
		 * Each transition captures source and destination states needed to generate
		 * appropriate GPU barriers.
		 *
		 * @param outTransitions  Receives all required resource transitions
		 */
		void BuildTransitions(Vector<ResourceTransition>& outTransitions);

		/** Checks if a barrier is needed between two consecutive uses */
		bool NeedsBarrier(const ResourceUsage& prevUsage, const ResourceUsage& currUsage) const;

		/**
		 * Converts resource transitions into GPU-specific barriers batched by destination pass.
		 *
		 * Creates buffer and texture barriers from transitions, grouping them by the pass
		 * that requires the barrier. Barriers for multiple resources needed by the same pass
		 * are batched together for efficiency. Each barrier specifies source/destination
		 * access masks and (for textures) layout transitions.
		 *
		 * @param transitions   Required resource transitions
		 * @param outBarriers   Receives barrier batches organized by destination pass
		 */
		void BuildBarriers(
			const Vector<ResourceTransition>& transitions,
			Vector<FrameGraphBarrierBatch>& outBarriers);

		/**
		 * Creates render targets for all render passes based on their attachment declarations.
		 *
		 * Iterates through passes and creates a RenderTarget for each render pass,
		 * configuring it with the color and depth attachments declared via WriteColor(),
		 * WriteDepth(), and ReadDepth(). The created render targets are stored in the
		 * compiled frame graph and used during execution when BeginRenderPass() is called.
		 *
		 * @param passes            Sorted list of passes
		 * @param outRenderTargets  Receives mapping from render pass to render target
		 */
		void CreateRenderTargets(
			const Vector<FrameGraphPass*>& passes,
			UnorderedMap<FrameGraphPass*, SPtr<RenderTarget>>& outRenderTargets);

		//////////////////////////////////////////////////////////////////////////
		// Member Variables
		//////////////////////////////////////////////////////////////////////////

		FrameGraph& mFrameGraph;

		/** Map of resources to passes that write them */
		UnorderedMap<FrameGraphResourceId, Vector<FrameGraphPass*>> mResourceWriters;

		/** Map of resources to passes that read them */
		UnorderedMap<FrameGraphResourceId, Vector<FrameGraphPass*>> mResourceReaders;

		/** Resource lifetime tracking */
		UnorderedMap<FrameGraphResourceId, FrameGraphResourceLifetime> mResourceLifetimes;

		/** Sorted pass execution order */
		Vector<FrameGraphPass*> mSortedPasses;

		/** Number of culled passes */
		u32 mCulledPassCount = 0;

		/** Passes involved in dependency cycles */
		Vector<FrameGraphPass*> mCyclePasses;

		/** Resource usage histories */
		UnorderedMap<FrameGraphResourceId, ResourceUsageHistory> mUsageHistories;
	};

	/** @} */
}
