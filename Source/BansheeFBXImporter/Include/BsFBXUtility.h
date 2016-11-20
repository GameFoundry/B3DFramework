//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFBXPrerequisites.h"
#include "BsFBXImportData.h"

namespace bs
{
	/** @addtogroup FBX
	 *  @{
	 */

	/**	Various helper methods to use during FBX import. */
	class FBXUtility
	{
	public:
		/**	Calculates per-index normals based on the provided smoothing groups. */
		static void normalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices,
			const Vector<int>& smoothing, Vector<Vector3>& normals);

		/**
		 * Find vertices in the source mesh that have different attributes but have the same indexes, and splits them into
		 * two or more vertexes. Mesh with split vertices is output to @p dest.
		 *
		 * @param[in]	source	Source mesh to perform the split on. It's expected the position values are per-vertex, and
		 *						all other attributes are per-index.
		 * @param[in]	dest	Output mesh with split vertices. Both vertex positions and attributes are per-vertex.
		 */
		static void splitVertices(const FBXImportMesh& source, FBXImportMesh& dest);

		/**	Flips the triangle window order for all the triangles in the mesh. */
		static void flipWindingOrder(FBXImportMesh& input);

	private:
		/** Checks if vertex attributes at the specified indexes are similar enough, or does the vertex require a split. */
		static bool needsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB);

		/**
		 * Copies vertex attributes from the source mesh at the specified index, to the destination mesh at the specified
		 * index.
		 */
		static void copyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx);

		/**
		 * Adds a new vertex to the destination mesh and copies the vertex from the source mesh at the vertex index, and
		 * vertex attributes from the source mesh at the source index.
		 */
		static void addVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh);
	};

	/** @} */
}