//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DTriangulation.h"
#include "Math/B3DVector3.h"

// Third party
#include "TetGen/tetgen.h"

using namespace b3d;

TetrahedronVolume Triangulation::Tetrahedralize(const Vector<Vector3>& points)
{
	TetrahedronVolume volume;
	if(points.size() < 4)
		return volume;

	tetgenio input;
	input.numberofpoints = (int)points.size();
	input.pointlist = new REAL[input.numberofpoints * 3]; // Must be allocated with "new" because TetGen deallocates it using "delete"
	for(u32 pointIndex = 0; pointIndex < (u32)points.size(); ++pointIndex)
	{
		input.pointlist[pointIndex * 3 + 0] = points[pointIndex].X;
		input.pointlist[pointIndex * 3 + 1] = points[pointIndex].Y;
		input.pointlist[pointIndex * 3 + 2] = points[pointIndex].Z;
	}

	tetgenbehavior options;
	options.neighout = 2; // Generate adjacency information between tets and outer faces
	options.facesout = 1; // Output face adjacency
	options.quiet = 1; // Don't print anything

	tetgenio output;
	::tetrahedralize(&options, &input, &output);

	u32 tetrahedronCount = (u32)output.numberoftetrahedra;
	volume.Tetrahedra.resize(tetrahedronCount);

	for(u32 tetrahedronIndex = 0; tetrahedronIndex < tetrahedronCount; ++tetrahedronIndex)
	{
		memcpy(volume.Tetrahedra[tetrahedronIndex].Vertices, &output.tetrahedronlist[tetrahedronIndex * 4], sizeof(i32) * 4);
		memcpy(volume.Tetrahedra[tetrahedronIndex].Neighbors, &output.neighborlist[tetrahedronIndex * 4], sizeof(i32) * 4);
	}

	// Generate boundary faces
	u32 faceCount = (u32)output.numberoftrifaces;
	for(u32 faceIndex = 0; faceIndex < faceCount; ++faceIndex)
	{
		i32 tetrahedronIndex = -1;
		if(output.adjtetlist[faceIndex * 2] == -1)
			tetrahedronIndex = output.adjtetlist[faceIndex * 2 + 1];
		else if(output.adjtetlist[faceIndex * 2 + 1] == -1)
			tetrahedronIndex = output.adjtetlist[faceIndex * 2];
		else // Not a boundary face
			continue;

		volume.OuterFaces.push_back(TetrahedronFace());
		TetrahedronFace& face = volume.OuterFaces.back();

		memcpy(face.Vertices, &output.trifacelist[faceIndex * 3], sizeof(i32) * 3);
		face.Tetrahedron = tetrahedronIndex;
	}

	// Ensure that vertex at the specified location points a neighbor opposite to it
	for(u32 tetrahedronIndex = 0; tetrahedronIndex < tetrahedronCount; ++tetrahedronIndex)
	{
		i32 neighbors[4];
		memcpy(neighbors, volume.Tetrahedra[tetrahedronIndex].Neighbors, sizeof(i32) * 4);

		for(u32 vertexIndex = 0; vertexIndex < 4; ++vertexIndex)
		{
			i32 vertex = volume.Tetrahedra[tetrahedronIndex].Vertices[vertexIndex];

			for(u32 neighborListIndex = 0; neighborListIndex < 4; ++neighborListIndex)
			{
				i32 neighborIndex = neighbors[neighborListIndex];
				if(neighborIndex == -1)
					continue;

				Tetrahedron& neighbor = volume.Tetrahedra[neighborIndex];
				if(vertex != neighbor.Vertices[0] && vertex != neighbor.Vertices[1] &&
				   vertex != neighbor.Vertices[2] && vertex != neighbor.Vertices[3])
				{
					volume.Tetrahedra[tetrahedronIndex].Neighbors[vertexIndex] = neighborIndex;
					break;
				}
			}
		}
	}

	return volume;
}
