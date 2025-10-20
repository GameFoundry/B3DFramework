//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphDependency.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Debugging utilities for frame graph visualization and inspection.
	 */
	class B3D_EXPORT FrameGraphDebug
	{
	public:
		/**
		 * Generate a text representation of the dependency graph.
		 * Useful for debugging and visualization.
		 *
		 * @param nodes The pass nodes to visualize
		 * @return String representation of the graph
		 */
		static String GenerateGraphText(const Vector<UPtr<FrameGraphPassNode>>& nodes);

		/**
		 * Generate a DOT format representation for visualization tools like Graphviz.
		 *
		 * @param nodes The pass nodes to visualize
		 * @return DOT format string
		 */
		static String GenerateGraphDOT(const Vector<UPtr<FrameGraphPassNode>>& nodes);
	};

	/** @} */
}
