//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsTriangulation.h"
#include "Math/BsVector3.h"

// Third party
#include "TetGen/tetgen.h"

namespace bs
{
	TetrahedronVolume Triangulation::Tetrahedralize(const Vector<Vector3>& points)
	{
		TetrahedronVolume volume;
		if (points.size() < 4)
			return volume;

		tetgenio input;
		input.numberofpoints = (int)points.size();
		input.pointlist = new REAL[input.numberofpoints * 3]; // Must be allocated with "new" because TetGen deallocates it using "delete"
		for(UINT32 i = 0; i < (UINT32)points.size(); ++i)
		{
			input.pointlist[i * 3 + 0] = points[i].x;
			input.pointlist[i * 3 + 1] = points[i].y;
			input.pointlist[i * 3 + 2] = points[i].z;
		}

		tetgenbehavior options;
		options.neighout = 2; // Generate adjacency information between tets and outer faces
		options.facesout = 1; // Output face adjacency
		options.quiet = 1; // Don't print anything

		tetgenio output;
		::tetrahedralize(&options, &input, &output);

		UINT32 numTetrahedra = (UINT32)output.numberoftetrahedra;
		volume.tetrahedra.Resize(numTetrahedra);

		for (UINT32 i = 0; i < numTetrahedra; ++i)
		{
			memcpy(volume.tetrahedra[i].vertices, &output.tetrahedronlist[i * 4], sizeof(INT32) * 4);
			memcpy(volume.tetrahedra[i].neighbors, &output.neighborlist[i * 4], sizeof(INT32) * 4);
		}

		// Generate boundary faces
		UINT32 numFaces = (UINT32)output.numberoftrifaces;
		for (UINT32 i = 0; i < numFaces; ++i)
		{
			INT32 tetIdx = -1;
			if (output.adjtetlist[i * 2] == -1)
				tetIdx = output.adjtetlist[i * 2 + 1];
			else if (output.adjtetlist[i * 2 + 1] == -1)
				tetIdx = output.adjtetlist[i * 2];
			else // Not a boundary face
				continue;

			volume.outerFaces.push_back(TetrahedronFace());
			TetrahedronFace& face = volume.outerFaces.back();

			memcpy(face.vertices, &output.trifacelist[i * 3], sizeof(INT32) * 3);
			face.tetrahedron = tetIdx;
		}

		// Ensure that vertex at the specified location points a neighbor opposite to it
		for(UINT32 i = 0; i < numTetrahedra; ++i)
		{
			INT32 neighbors[4];
			memcpy(neighbors, volume.tetrahedra[i].neighbors, sizeof(INT32) * 4);

			for(UINT32 j = 0; j < 4; ++j)
			{
				INT32 vert = volume.tetrahedra[i].vertices[j];

				for (UINT32 k = 0; k < 4; ++k)
				{
					INT32 neighborIdx = neighbors[k];
					if (neighborIdx == -1)
						continue;

					Tetrahedron& neighbor = volume.tetrahedra[neighborIdx];
					if (vert != neighbor.vertices[0] && vert != neighbor.vertices[1] &&
						vert != neighbor.vertices[2] && vert != neighbor.vertices[3])
					{
						volume.tetrahedra[i].neighbors[j] = neighborIdx;
						break;
					}
				}
			}
		}

		return volume;
	}
}
