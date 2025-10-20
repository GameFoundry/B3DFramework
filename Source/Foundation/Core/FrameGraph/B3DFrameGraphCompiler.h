//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphDependencyAnalyzer.h"
#include "B3DFrameGraphTopologicalSort.h"
#include "B3DFrameGraphCulling.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Compiled frame graph output.
	 *
	 * Contains the result of frame graph compilation, including the ordered list of passes
	 * to execute and resource lifetime information.
	 *
	 * Phase 1: Just stores passes in declaration order
	 * Phase 2: Stores topologically sorted passes and resource lifetimes
	 * Later phases: Will include barrier information, resource allocation commands, etc.
	 */
	class B3D_EXPORT CompiledFrameGraph
	{
	public:
		/** Passes in topologically sorted execution order (culled passes removed) */
		Vector<FrameGraphPassNode*> SortedPasses;

		/** Resource lifetime information for all resources */
		UnorderedMap<FrameGraphResourceId, FrameGraphResourceLifetime> ResourceLifetimes;
	};

	/**
	 * Compiles a frame graph into an executable form.
	 *
	 * The compiler is responsible for:
	 * - Executing pass setup callbacks to collect resource dependencies
	 * - Validating resource usage and access patterns
	 * - Computing execution order (Phase 2+)
	 * - Calculating synchronization barriers (Phase 3+)
	 * - Performing resource lifetime analysis (Phase 4+)
	 *
	 * Phase 1 Implementation:
	 * - Executes setup functions to populate resource accesses
	 * - Validates that all referenced resources exist
	 * - Validates usage/access flag consistency
	 * - Stores passes in declaration order
	 *
	 * Future Phases:
	 * - Phase 2: Build dependency DAG, topological sort for execution order
	 * - Phase 3: Calculate memory barriers and layout transitions
	 * - Phase 4: Lifetime analysis for transient resource allocation
	 * - Phase 5: Multi-queue scheduling and async compute optimization
	 */
	class B3D_EXPORT FrameGraphCompiler
	{
	public:
		explicit FrameGraphCompiler(FrameGraph& frameGraph);

		/**
		 * Compiles the frame graph.
		 *
		 * Executes all setup callbacks, validates the graph, and produces a compiled graph
		 * ready for execution.
		 *
		 * @return  Compiled frame graph, or nullptr if validation failed
		 */
		UPtr<CompiledFrameGraph> Compile();

	private:
		/** Validates the graph (checks resources exist) */
		bool Validate();

		/** Validates a single pass */
		bool ValidatePass(FrameGraphPass* pass);

		/** Validates a resource access */
		bool ValidateResourceAccess(FrameGraphPass* pass, const FrameGraphResourceAccess& access);

		/** Validates dependencies (Phase 2: checks for isolated passes, etc.) */
		bool ValidateDependencies(const Vector<UPtr<FrameGraphPassNode>>& nodes);

		/** Executes setup functions for all passes */
		void ExecuteSetupFunctions();

		FrameGraph& mFrameGraph;

		/** Dependency analyzer (Phase 2) */
		UPtr<FrameGraphDependencyAnalyzer> mDependencyAnalyzer;

		/** Topological sorter (Phase 2) */
		UPtr<FrameGraphTopologicalSort> mTopologicalSort;

		/** Pass culler (Phase 2) */
		UPtr<FrameGraphCulling> mCulling;

		/** Sorted pass execution order (Phase 2) */
		Vector<FrameGraphPassNode*> mSortedPasses;
	};

	/** @} */
}
