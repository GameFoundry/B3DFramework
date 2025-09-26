//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Contains information about a single tetrahedron. */
	struct Tetrahedron
	{
		/** Indices of vertices that form the tetrahedron pointing to an external point array. */
		i32 Vertices[4];

		/**
		 * Indices pointing to neighbor tetrahedrons. Each neighbor index maps to the @p vertices array, so neighbor/vertex
		 * pair at the same location will be the only neighbor not containing that vertex (i.e. neighbor opposite to
		 * the vertex). If a tetrahedron is on the volume edge, it has only three neighbors and its last neighbor will be
		 * set to -1.
		 */
		i32 Neighbors[4];
	};

	/** Contains information about a single face of a tetrahedron. */
	struct TetrahedronFace
	{
		i32 Vertices[3];
		i32 Tetrahedron;
	};

	/** Contains information about a volume made out of tetrahedrons. */
	struct TetrahedronVolume
	{
		Vector<Tetrahedron> Tetrahedra;
		Vector<TetrahedronFace> OuterFaces;
	};

	/** Contains helper methods that triangulate point data. */
	class B3D_EXPORT Triangulation
	{
	public:
		/**
		 * Converts a set of input points into a set of tetrahedrons generated using Delaunay tetrahedralization
		 * algorithm. Minimum of 4 points must be provided in order for the process to work.
		 */
		static TetrahedronVolume Tetrahedralize(const Vector<Vector3>& points);
	};

	/** @} */
} // namespace b3d
