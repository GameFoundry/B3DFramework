//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Contains information about a single tetrahedron. */
	struct Tetrahedron
	{
		/** Indices of vertices that form the tetrahedron pointing to an external point array. */
		INT32 vertices[4];

		/** 
		 * Indices pointing to neighbor tetrahedrons. Each neighbor index maps to the @p vertices array, so neighbor/vertex
		 * pair at the same location will be the only neighbor not containing that vertex (i.e. neighbor opposite to
		 * the vertex). If a tetrahedron is on the volume edge, it has only three neighbors and its last neighbor will be
		 * set to -1.
		 */
		INT32 neighbors[4];
	};

	/** Contains information about a single face of a tetrahedron. */
	struct TetrahedronFace
	{
		INT32 vertices[3];
		INT32 tetrahedron;
	};

	/** Contains information about a volume made out of tetrahedrons. */
	struct TetrahedronVolume
	{
		Vector<Tetrahedron> tetrahedra;
		Vector<TetrahedronFace> outerFaces;
	};

	/** Contains helper methods that triangulate point data. */
	class BS_UTILITY_EXPORT Triangulation
	{
	public:
		/** 
		 * Converts a set of input points into a set of tetrahedrons generated using Delaunay tetrahedralization 
		 * algorithm. Minimum of 4 points must be provided in order for the process to work.
		 */
		static TetrahedronVolume tetrahedralize(const Vector<Vector3>& points);
	};

	/** @} */
}