//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFBXPrerequisites.h"
#include "B3DFBXImportData.h"

namespace b3d
{
	/** @addtogroup FBX
	 *  @{
	 */

	/**	Various helper methods to use during FBX import. */
	class FBXUtility
	{
	public:
		/**	Calculates per-index normals based on the provided smoothing groups. */
		static void NormalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices, const Vector<int>& smoothing, Vector<Vector3>& normals);

		/**
		 * Find vertices in the source mesh that have different attributes but have the same indexes, and splits them into
		 * two or more vertexes. Mesh with split vertices is output to @p dest.
		 *
		 * @param[in]	source	Source mesh to perform the split on. It's expected the position values are per-vertex, and
		 *						all other attributes are per-index.
		 * @param[in]	dest	Output mesh with split vertices. Both vertex positions and attributes are per-vertex.
		 */
		static void SplitVertices(const FBXImportMesh& source, FBXImportMesh& dest);

		/**	Flips the triangle window order for all the triangles in the mesh. */
		static void FlipWindingOrder(FBXImportMesh& input);

	private:
		/** Checks if vertex attributes at the specified indexes are similar enough, or does the vertex require a split. */
		static bool NeedsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB);

		/**
		 * Copies vertex attributes from the source mesh at the specified index, to the destination mesh at the specified
		 * index.
		 */
		static void CopyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx);

		/**
		 * Adds a new vertex to the destination mesh and copies the vertex from the source mesh at the vertex index, and
		 * vertex attributes from the source mesh at the source index.
		 */
		static void AddVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh);
	};

	/** @} */
} // namespace b3d
