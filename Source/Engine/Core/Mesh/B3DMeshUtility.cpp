//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Mesh/B3DMeshUtility.h"
#include "Math/B3DVector4.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Math/B3DPlane.h"

using namespace b3d;

struct VertexFaces
{
	u32* Faces;
	u32 NumFaces = 0;
};

struct VertexConnectivity
{
	VertexConnectivity(u8* indices, u32 numVertices, u32 numFaces, u32 indexSize)
		: VertexFaces(nullptr), mMaxFacesPerVertex(0), mNumVertices(numVertices), mFaces(nullptr)
	{
		VertexFaces = B3DNewMultiple<struct VertexFaces>(numVertices);

		ResizeFaceArray(10);

		for(u32 i = 0; i < numFaces; i++)
		{
			for(u32 j = 0; j < 3; j++)
			{
				u32 idx = i * 3 + j;
				u32 vertexIdx = 0;
				memcpy(&vertexIdx, indices + idx * indexSize, indexSize);

				B3D_ASSERT(vertexIdx < mNumVertices);
				struct VertexFaces& faces = VertexFaces[vertexIdx];
				if(faces.NumFaces >= mMaxFacesPerVertex)
					ResizeFaceArray(mMaxFacesPerVertex * 2);

				faces.Faces[faces.NumFaces] = i;
				faces.NumFaces++;
			}
		}
	}

	~VertexConnectivity()
	{
		if(VertexFaces != nullptr)
			B3DDeleteMultiple(VertexFaces, mNumVertices);

		if(mFaces != nullptr)
			B3DFree(mFaces);
	}

	VertexFaces* VertexFaces;

private:
	void ResizeFaceArray(u32 numFaces)
	{
		u32* newFaces = (u32*)B3DAllocate(numFaces * mNumVertices * sizeof(u32));

		if(mFaces != nullptr)
		{
			for(u32 i = 0; i < mNumVertices; i++)
				memcpy(newFaces + (i * numFaces), mFaces + (i * mMaxFacesPerVertex), mMaxFacesPerVertex * sizeof(u32));

			B3DFree(mFaces);
		}

		for(u32 i = 0; i < mNumVertices; i++)
			VertexFaces[i].Faces = newFaces + (i * numFaces);

		mFaces = newFaces;
		mMaxFacesPerVertex = numFaces;
	}

	u32 mMaxFacesPerVertex;
	u32 mNumVertices;
	u32* mFaces;
};

/** Provides base methods required for clipping of arbitrary triangles. */
class TriangleClipperBase // Implementation from: http://www.geometrictools.com/Documentation/ClipMesh.pdf
{
protected:
	/** Single vertex in the clipped mesh. */
	struct ClipVert
	{
		ClipVert() {}

		Vector3 Point = Vector3::kZero;
		Vector2 Uv = Vector2::kZero;
		float Distance = 0.0f;
		u32 Occurs = 0;
		bool Visible = true;
	};

	/** Single edge in the clipped mesh. */
	struct ClipEdge
	{
		ClipEdge() {}

		u32 Verts[2];
		Vector<u32> Faces;
		bool Visible = true;
	};

	/** Single polygon in the clipped mesh. */
	struct ClipFace
	{
		ClipFace() {}

		Vector<u32> Edges;
		bool Visible = true;
		Vector3 Normal = Vector3::kZero;
	};

	/** Contains vertices, edges and faces of the clipped mesh. */
	struct ClipMesh
	{
		ClipMesh() {}

		Vector<ClipVert> Verts;
		Vector<ClipEdge> Edges;
		Vector<ClipFace> Faces;
	};

protected:
	/**
	 * Register all edges and faces, using the mesh vertices as a basis. Assumes vertices are not indexed and that
	 * every three vertices form a face
	 */
	void AddEdgesAndFaces();

	/** Clips the current mesh with the provided plane. */
	i32 ClipByPlane(const Plane& plane);

	/** Clips vertices of the current mesh by the provided plane. */
	i32 ProcessVertices(const Plane& plane);

	/** Clips edges of the current mesh. processVertices() must be called beforehand. */
	void ProcessEdges();

	/** Clips the faces (polygons) of the current mesh. processEdges() must be called beforehand. */
	void ProcessFaces();

	/**
	 * Returns a set of non-culled vertex indices for every visible face in the mesh. This should be called after
	 * clipping operation is complete to retrieve valid vertices.
	 */
	void GetOrderedFaces(FrameVector<FrameVector<u32>>& sortedFaces);

	/** Returns a set of ordered and non-culled vertices for the provided face of the mesh */
	void GetOrderedVertices(const ClipFace& face, u32* vertices);

	/** Calculates the normal for vertices related to the provided vertex indices. */
	Vector3 GetNormal(u32* sortedVertices, u32 numVertices);

	/**
	 * Checks is the polygon shape of the provided face open or closed. If open, returns true and outputs endpoints of
	 * the polyline.
	 */
	bool GetOpenPolyline(ClipFace& face, u32& start, u32& end);

	ClipMesh mesh;
};

void TriangleClipperBase::AddEdgesAndFaces()
{
	u32 numTris = (u32)mesh.Verts.size() / 3;

	u32 numEdges = numTris * 3;
	mesh.Edges.resize(numEdges);
	mesh.Faces.resize(numTris);

	for(u32 i = 0; i < numTris; i++)
	{
		u32 idx0 = i * 3 + 0;
		u32 idx1 = i * 3 + 1;
		u32 idx2 = i * 3 + 2;

		ClipEdge& clipEdge0 = mesh.Edges[idx0];
		clipEdge0.Verts[0] = idx0;
		clipEdge0.Verts[1] = idx1;

		ClipEdge& clipEdge1 = mesh.Edges[idx1];
		clipEdge1.Verts[0] = idx1;
		clipEdge1.Verts[1] = idx2;

		ClipEdge& clipEdge2 = mesh.Edges[idx2];
		clipEdge2.Verts[0] = idx2;
		clipEdge2.Verts[1] = idx0;

		ClipFace& clipFace = mesh.Faces[i];

		clipFace.Edges.push_back(idx0);
		clipFace.Edges.push_back(idx1);
		clipFace.Edges.push_back(idx2);

		clipEdge0.Faces.push_back(i);
		clipEdge1.Faces.push_back(i);
		clipEdge2.Faces.push_back(i);

		u32 verts[] = { idx0, idx1, idx2, idx0 };
		for(u32 j = 0; j < 3; j++)
			clipFace.Normal += Vector3::Cross(mesh.Verts[verts[j]].Point, mesh.Verts[verts[j + 1]].Point);

		clipFace.Normal.Normalize();
	}
}

i32 TriangleClipperBase::ClipByPlane(const Plane& plane)
{
	int state = ProcessVertices(plane);

	if(state == 1)
		return +1; // Nothing is clipped
	else if(state == -1)
		return -1; // Everything is clipped

	ProcessEdges();
	ProcessFaces();

	return 0;
}

i32 TriangleClipperBase::ProcessVertices(const Plane& plane)
{
	static const float kEpsilon = 0.00001f;

	// Compute signed distances from vertices to plane
	int positive = 0, negative = 0;
	for(u32 i = 0; i < (u32)mesh.Verts.size(); i++)
	{
		ClipVert& vertex = mesh.Verts[i];

		if(vertex.Visible)
		{
			vertex.Distance = Vector3::Dot(plane.Normal, vertex.Point) - plane.D;
			if(vertex.Distance >= kEpsilon)
			{
				positive++;
			}
			else if(vertex.Distance <= -kEpsilon)
			{
				negative++;
				vertex.Visible = false;
			}
			else
			{
				// Point on the plane within floating point tolerance
				vertex.Distance = 0;
			}
		}
	}
	if(negative == 0)
	{
		// All vertices on nonnegative side, no clipping
		return +1;
	}
	if(positive == 0)
	{
		// All vertices on nonpositive side, everything clipped
		return -1;
	}

	return 0;
}

void TriangleClipperBase::ProcessEdges()
{
	for(u32 i = 0; i < (u32)mesh.Edges.size(); i++)
	{
		ClipEdge& edge = mesh.Edges[i];

		if(edge.Visible)
		{
			const ClipVert& v0 = mesh.Verts[edge.Verts[0]];
			const ClipVert& v1 = mesh.Verts[edge.Verts[1]];

			float d0 = v0.Distance;
			float d1 = v1.Distance;

			if(d0 <= 0 && d1 <= 0)
			{
				// Edge is culled, remove edge from faces sharing it
				for(u32 j = 0; j < (u32)edge.Faces.size(); j++)
				{
					ClipFace& face = mesh.Faces[edge.Faces[j]];

					auto iterFind = std::find(face.Edges.begin(), face.Edges.end(), i);
					if(iterFind != face.Edges.end())
					{
						face.Edges.erase(iterFind);

						if(face.Edges.empty())
							face.Visible = false;
					}
				}

				edge.Visible = false;
				continue;
			}

			if(d0 >= 0 && d1 >= 0)
			{
				// Edge is on nonnegative side, faces retain the edge
				continue;
			}

			// The edge is split by the plane. Compute the point of intersection.
			// If the old edge is <V0,V1> and I is the intersection point, the new
			// edge is <V0,I> when d0 > 0 or <I,V1> when d1 > 0.
			float t = d0 / (d0 - d1);
			Vector3 intersectPt = (1 - t) * v0.Point + t * v1.Point;
			Vector2 intersectUv = (1 - t) * v0.Uv + t * v1.Uv;

			u32 newVertIdx = (u32)mesh.Verts.size();
			mesh.Verts.push_back(ClipVert());

			ClipVert& newVert = mesh.Verts.back();
			newVert.Point = intersectPt;
			newVert.Uv = intersectUv;

			if(d0 > 0)
				mesh.Edges[i].Verts[1] = newVertIdx;
			else
				mesh.Edges[i].Verts[0] = newVertIdx;
		}
	}
}

void TriangleClipperBase::ProcessFaces()
{
	for(u32 i = 0; i < (u32)mesh.Faces.size(); i++)
	{
		ClipFace& face = mesh.Faces[i];

		if(face.Visible)
		{
			// The edge is culled. If the edge is exactly on the clip
			// plane, it is possible that a visible triangle shares it.
			// The edge will be re-added during the face loop.

			for(u32 j = 0; j < (u32)face.Edges.size(); j++)
			{
				ClipEdge& edge = mesh.Edges[face.Edges[j]];
				ClipVert& v0 = mesh.Verts[edge.Verts[0]];
				ClipVert& v1 = mesh.Verts[edge.Verts[1]];

				v0.Occurs = 0;
				v1.Occurs = 0;
			}
		}

		u32 start, end;
		if(GetOpenPolyline(mesh.Faces[i], start, end))
		{
			// Polyline is open, close it
			u32 closeEdgeIdx = (u32)mesh.Edges.size();
			mesh.Edges.push_back(ClipEdge());
			ClipEdge& closeEdge = mesh.Edges.back();

			closeEdge.Verts[0] = start;
			closeEdge.Verts[1] = end;

			closeEdge.Faces.push_back(i);
			face.Edges.push_back(closeEdgeIdx);
		}
	}
}

bool TriangleClipperBase::GetOpenPolyline(ClipFace& face, u32& start, u32& end)
{
	// Count the number of occurrences of each vertex in the polyline. The
	// resulting "occurs" values must be 1 or 2.
	for(u32 i = 0; i < (u32)face.Edges.size(); i++)
	{
		ClipEdge& edge = mesh.Edges[face.Edges[i]];

		if(edge.Visible)
		{
			ClipVert& v0 = mesh.Verts[edge.Verts[0]];
			ClipVert& v1 = mesh.Verts[edge.Verts[1]];

			v0.Occurs++;
			v1.Occurs++;
		}
	}

	// Determine if the polyline is open
	bool gotStart = false;
	bool gotEnd = false;
	for(u32 i = 0; i < (u32)face.Edges.size(); i++)
	{
		const ClipEdge& edge = mesh.Edges[face.Edges[i]];

		const ClipVert& v0 = mesh.Verts[edge.Verts[0]];
		const ClipVert& v1 = mesh.Verts[edge.Verts[1]];

		if(v0.Occurs == 1)
		{
			if(!gotStart)
			{
				start = edge.Verts[0];
				gotStart = true;
			}
			else if(!gotEnd)
			{
				end = edge.Verts[0];
				gotEnd = true;
			}
		}

		if(v1.Occurs == 1)
		{
			if(!gotStart)
			{
				start = edge.Verts[1];
				gotStart = true;
			}
			else if(!gotEnd)
			{
				end = edge.Verts[1];
				gotEnd = true;
			}
		}
	}

	return gotStart;
}

void TriangleClipperBase::GetOrderedFaces(FrameVector<FrameVector<u32>>& sortedFaces)
{
	for(u32 i = 0; i < (u32)mesh.Faces.size(); i++)
	{
		const ClipFace& face = mesh.Faces[i];

		if(face.Visible)
		{
			// Get the ordered vertices of the face. The first and last
			// element of the array are the same since the polyline is
			// closed.
			u32 numSortedVerts = (u32)face.Edges.size() + 1;
			u32* sortedVerts = (u32*)B3DStackAllocate(sizeof(u32) * numSortedVerts);

			GetOrderedVertices(face, sortedVerts);

			FrameVector<u32> faceVerts;

			// The convention is that the vertices should be counterclockwise
			// ordered when viewed from the negative side of the plane of the
			// face. If you need the opposite convention, switch the
			// inequality in the if-else statement.
			Vector3 normal = GetNormal(sortedVerts, numSortedVerts);
			if(Vector3::Dot(mesh.Faces[i].Normal, normal) < 0)
			{
				// Clockwise, need to swap
				for(i32 j = (i32)numSortedVerts - 2; j >= 0; j--)
					faceVerts.push_back(sortedVerts[j]);
			}
			else
			{
				// Counterclockwise
				for(int j = 0; j <= (i32)numSortedVerts - 2; j++)
					faceVerts.push_back(sortedVerts[j]);
			}

			sortedFaces.push_back(faceVerts);
			B3DStackFree(sortedVerts);
		}
	}
}

void TriangleClipperBase::GetOrderedVertices(const ClipFace& face, u32* sortedVerts)
{
	u32 numEdges = (u32)face.Edges.size();
	u32* sortedEdges = (u32*)B3DStackAllocate(sizeof(u32) * numEdges);
	for(u32 i = 0; i < numEdges; i++)
		sortedEdges[i] = face.Edges[i];

	// Bubble sort to arrange edges in contiguous order
	for(u32 i0 = 0, i1 = 1, choice = 1; i1 < numEdges - 1; i0 = i1, i1++)
	{
		const ClipEdge& edge0 = mesh.Edges[sortedEdges[i0]];

		u32 current = edge0.Verts[choice];
		for(u32 j = i1; j < numEdges; j++)
		{
			const ClipEdge& edge1 = mesh.Edges[sortedEdges[j]];

			if(edge1.Verts[0] == current || edge1.Verts[1] == current)
			{
				std::swap(sortedEdges[i1], sortedEdges[j]);
				choice = 1;
				break;
			}
		}
	}

	// Add the first two vertices
	sortedVerts[0] = mesh.Edges[sortedEdges[0]].Verts[0];
	sortedVerts[1] = mesh.Edges[sortedEdges[0]].Verts[1];

	// Add the remaining vertices
	for(u32 i = 1; i < numEdges; i++)
	{
		const ClipEdge& edge = mesh.Edges[sortedEdges[i]];

		if(edge.Verts[0] == sortedVerts[i])
			sortedVerts[i + 1] = edge.Verts[1];
		else
			sortedVerts[i + 1] = edge.Verts[0];
	}

	B3DStackFree(sortedEdges);
}

Vector3 TriangleClipperBase::GetNormal(u32* sortedVertices, u32 numVertices)
{
	Vector3 normal(kZeroTag);
	for(u32 i = 0; i <= numVertices - 2; i++)
		normal += Vector3::Cross(mesh.Verts[sortedVertices[i]].Point, mesh.Verts[sortedVertices[i + 1]].Point);

	normal.Normalize();
	return normal;
}

/** Clips two-dimensional triangles against a set of provided planes. */
class TriangleClipper2D : public TriangleClipperBase
{
public:
	/** @copydoc MeshUtility::Clip2D */
	void Clip(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback);

private:
	/** Converts clipped vertices back into triangles and outputs them via the provided callback. */
	void ConvertToMesh(const std::function<void(Vector2*, Vector2*, u32)>& writeCallback);

	static const int kBufferSize = 64 * 3; // Must be a multiple of three
	Vector2 vertexBuffer[kBufferSize];
	Vector2 uvBuffer[kBufferSize];
};

void TriangleClipper2D::Clip(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback)
{
	// Add vertices
	u32 vertexCount = triangleCount * 3;
	mesh.Verts.resize(vertexCount);

	if(uvs != nullptr)
	{
		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			ClipVert& clipVert = mesh.Verts[vertexIndex];
			Vector2 vector2D = *(Vector2*)(vertices + vertexStride * vertexIndex);

			clipVert.Point = Vector3(vector2D.X, vector2D.Y, 0.0f);
			clipVert.Uv = *(Vector2*)(uvs + vertexStride * vertexIndex);
		}
	}
	else
	{
		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			ClipVert& clipVert = mesh.Verts[vertexIndex];
			Vector2 vector2D = *(Vector2*)(vertices + vertexStride * vertexIndex);

			clipVert.Point = Vector3(vector2D.X, vector2D.Y, 0.0f);
		}
	}

	AddEdgesAndFaces();

	for(int planeIndex = 0; planeIndex < 4; planeIndex++)
	{
		if(ClipByPlane(clipPlanes[planeIndex]) == -1)
			return;
	}

	ConvertToMesh(writeCallback);
}

void TriangleClipper2D::ConvertToMesh(const std::function<void(Vector2*, Vector2*, u32)>& writeCallback)
{
	B3DMarkAllocatorFrame();
	{
		FrameVector<FrameVector<u32>> allFaces;
		GetOrderedFaces(allFaces);

		// Note: Consider using Delaunay triangulation to avoid skinny triangles
		u32 numWritten = 0;
		B3D_ASSERT(kBufferSize % 3 == 0);
		for(auto& face : allFaces)
		{
			for(u32 i = 0; i < (u32)face.size() - 2; i++)
			{
				const Vector3& v0 = mesh.Verts[face[0]].Point;
				const Vector3& v1 = mesh.Verts[face[i + 1]].Point;
				const Vector3& v2 = mesh.Verts[face[i + 2]].Point;

				vertexBuffer[numWritten] = Vector2(v0.X, v0.Y);
				uvBuffer[numWritten] = mesh.Verts[face[0]].Uv;
				numWritten++;

				vertexBuffer[numWritten] = Vector2(v1.X, v1.Y);
				uvBuffer[numWritten] = mesh.Verts[face[i + 1]].Uv;
				numWritten++;

				vertexBuffer[numWritten] = Vector2(v2.X, v2.Y);
				uvBuffer[numWritten] = mesh.Verts[face[i + 2]].Uv;
				numWritten++;

				// Only need to check this here since we guarantee the buffer is in multiples of three
				if(numWritten >= kBufferSize)
				{
					writeCallback(vertexBuffer, uvBuffer, numWritten);
					numWritten = 0;
				}
			}
		}

		if(numWritten > 0)
			writeCallback(vertexBuffer, uvBuffer, numWritten);
	}
	B3DClearAllocatorFrame();
}

/** Clips three-dimensional triangles against a set of provided planes. */
class TriangleClipper3D : public TriangleClipperBase
{
public:
	/** @copydoc MeshUtility::Clip3D */
	void Clip(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector3*, Vector2*, u32)>& writeCallback);

private:
	/** Converts clipped vertices back into triangles and outputs them via the provided callback. */
	void ConvertToMesh(const std::function<void(Vector3*, Vector2*, u32)>& writeCallback);

	static const int kBufferSize = 64 * 3; // Must be a multiple of three
	Vector3 vertexBuffer[kBufferSize];
	Vector2 uvBuffer[kBufferSize];
};

void TriangleClipper3D::Clip(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector3*, Vector2*, u32)>& writeCallback)
{
	// Add vertices
	u32 vertexCount = triangleCount * 3;
	mesh.Verts.resize(vertexCount);

	if(uvs != nullptr)
	{
		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			ClipVert& clipVert = mesh.Verts[vertexIndex];

			clipVert.Point = *(Vector3*)(vertices + vertexStride * vertexIndex);
			clipVert.Uv = *(Vector2*)(uvs + vertexStride * vertexIndex);
		}
	}
	else
	{
		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		{
			ClipVert& clipVert = mesh.Verts[vertexIndex];
			Vector2 vector2D = *(Vector2*)(vertices + vertexStride * vertexIndex);

			clipVert.Point = Vector3(vector2D.X, vector2D.Y, 0.0f);
		}
	}

	AddEdgesAndFaces();

	for(int planeIndex = 0; planeIndex < 4; planeIndex++)
	{
		if(ClipByPlane(clipPlanes[planeIndex]) == -1)
			return;
	}

	ConvertToMesh(writeCallback);
}

void TriangleClipper3D::ConvertToMesh(const std::function<void(Vector3*, Vector2*, u32)>& writeCallback)
{
	B3DMarkAllocatorFrame();
	{
		FrameVector<FrameVector<u32>> allFaces;
		GetOrderedFaces(allFaces);

		// Note: Consider using Delaunay triangulation to avoid skinny triangles
		u32 numWritten = 0;
		B3D_ASSERT(kBufferSize % 3 == 0);
		for(auto& face : allFaces)
		{
			for(u32 i = 0; i < (u32)face.size() - 2; i++)
			{
				vertexBuffer[numWritten] = mesh.Verts[face[0]].Point;
				uvBuffer[numWritten] = mesh.Verts[face[0]].Uv;
				numWritten++;

				vertexBuffer[numWritten] = mesh.Verts[face[i + 1]].Point;
				uvBuffer[numWritten] = mesh.Verts[face[i + 1]].Uv;
				numWritten++;

				vertexBuffer[numWritten] = mesh.Verts[face[i + 2]].Point;
				uvBuffer[numWritten] = mesh.Verts[face[i + 2]].Uv;
				numWritten++;

				// Only need to check this here since we guarantee the buffer is in multiples of three
				if(numWritten >= kBufferSize)
				{
					writeCallback(vertexBuffer, uvBuffer, numWritten);
					numWritten = 0;
				}
			}
		}

		if(numWritten > 0)
			writeCallback(vertexBuffer, uvBuffer, numWritten);
	}
	B3DClearAllocatorFrame();
}

void MeshUtility::CalculateNormals(Vector3* vertices, u8* indices, u32 vertexCount, u32 indexCount, Vector3* normals, u32 indexSize)
{
	u32 faceCount = indexCount / 3;

	Vector3* faceNormals = B3DNewMultiple<Vector3>(faceCount);
	for(u32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		u32 triangle[3];
		memcpy(&triangle[0], indices + (faceIndex * 3 + 0) * indexSize, indexSize);
		memcpy(&triangle[1], indices + (faceIndex * 3 + 1) * indexSize, indexSize);
		memcpy(&triangle[2], indices + (faceIndex * 3 + 2) * indexSize, indexSize);

		Vector3 edgeA = vertices[triangle[1]] - vertices[triangle[0]];
		Vector3 edgeB = vertices[triangle[2]] - vertices[triangle[0]];
		faceNormals[faceIndex] = Vector3::Normalize(Vector3::Cross(edgeA, edgeB));

		// Note: Potentially don't normalize here in order to weigh the normals
		// by triangle size
	}

	VertexConnectivity connectivity(indices, vertexCount, faceCount, indexSize);
	for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		VertexFaces& faces = connectivity.VertexFaces[vertexIndex];

		normals[vertexIndex] = Vector3::kZero;
		for(u32 faceArrayIndex = 0; faceArrayIndex < faces.NumFaces; faceArrayIndex++)
		{
			u32 faceIndex = faces.Faces[faceArrayIndex];
			normals[vertexIndex] += faceNormals[faceIndex];
		}

		normals[vertexIndex].Normalize();
	}

	B3DDeleteMultiple(faceNormals, faceCount);
}

void MeshUtility::CalculateTangents(Vector3* vertices, Vector3* normals, Vector2* uv, u8* indices, u32 vertexCount, u32 indexCount, Vector3* tangents, Vector3* bitangents, u32 indexSize, u32 vertexStride)
{
	u32 faceCount = indexCount / 3;
	u32 vector2Stride = vertexStride == 0 ? sizeof(Vector2) : vertexStride;
	u32 vector3Stride = vertexStride == 0 ? sizeof(Vector3) : vertexStride;

	u8* positionBytes = (u8*)vertices;
	u8* normalBytes = (u8*)normals;
	u8* uvBytes = (u8*)uv;

	Vector3* faceTangents = B3DNewMultiple<Vector3>(faceCount);
	Vector3* faceBitangents = B3DNewMultiple<Vector3>(faceCount);
	for(u32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		u32 triangle[3];
		memcpy(&triangle[0], indices + (faceIndex * 3 + 0) * indexSize, indexSize);
		memcpy(&triangle[1], indices + (faceIndex * 3 + 1) * indexSize, indexSize);
		memcpy(&triangle[2], indices + (faceIndex * 3 + 2) * indexSize, indexSize);

		Vector3 position0 = *(Vector3*)&positionBytes[triangle[0] * vector3Stride];
		Vector3 position1 = *(Vector3*)&positionBytes[triangle[1] * vector3Stride];
		Vector3 position2 = *(Vector3*)&positionBytes[triangle[2] * vector3Stride];

		Vector2 uv0 = *(Vector2*)&uvBytes[triangle[0] * vector2Stride];
		Vector2 uv1 = *(Vector2*)&uvBytes[triangle[1] * vector2Stride];
		Vector2 uv2 = *(Vector2*)&uvBytes[triangle[2] * vector2Stride];

		Vector3 edge0 = position1 - position0;
		Vector3 edge1 = position2 - position0;

		Vector2 deltaUV1 = uv1 - uv0;
		Vector2 deltaUV2 = uv2 - uv0;

		float denominator = deltaUV1.X * deltaUV2.Y - deltaUV2.X * deltaUV1.Y;
		if(fabs(denominator) >= 0e-8f)
		{
			float reciprocal = 1.0f / denominator;

			faceTangents[faceIndex] = (deltaUV2.Y * edge0 - deltaUV1.Y * edge1) * reciprocal;
			faceBitangents[faceIndex] = (deltaUV1.X * edge1 - deltaUV2.X * edge0) * reciprocal;

			faceTangents[faceIndex].Normalize();
			faceBitangents[faceIndex].Normalize();
		}

		// Note: Potentially don't normalize here in order to weight the normals by triangle size
	}

	VertexConnectivity connectivity(indices, vertexCount, faceCount, indexSize);
	for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		VertexFaces& faces = connectivity.VertexFaces[vertexIndex];

		tangents[vertexIndex] = Vector3::kZero;
		bitangents[vertexIndex] = Vector3::kZero;

		for(u32 faceArrayIndex = 0; faceArrayIndex < faces.NumFaces; faceArrayIndex++)
		{
			u32 faceIndex = faces.Faces[faceArrayIndex];
			tangents[vertexIndex] += faceTangents[faceIndex];
			bitangents[vertexIndex] += faceBitangents[faceIndex];
		}

		tangents[vertexIndex].Normalize();
		bitangents[vertexIndex].Normalize();

		Vector3 normal = *(Vector3*)&normalBytes[vertexIndex * vector3Stride];

		// Orthonormalize
		float dotProduct0 = normal.Dot(tangents[vertexIndex]);
		tangents[vertexIndex] -= dotProduct0 * normal;
		tangents[vertexIndex].Normalize();

		float dotProduct1 = tangents[vertexIndex].Dot(bitangents[vertexIndex]);
		dotProduct0 = normal.Dot(bitangents[vertexIndex]);
		bitangents[vertexIndex] -= dotProduct0 * normal + dotProduct1 * tangents[vertexIndex];
		bitangents[vertexIndex].Normalize();
	}

	B3DDeleteMultiple(faceTangents, faceCount);
	B3DDeleteMultiple(faceBitangents, faceCount);

	// TODO - Consider weighing tangents by triangle size and/or edge angles
}

void MeshUtility::CalculateTangentSpace(Vector3* vertices, Vector2* uv, u8* indices, u32 vertexCount, u32 indexCount, Vector3* normals, Vector3* tangents, Vector3* bitangents, u32 indexSize)
{
	CalculateNormals(vertices, indices, vertexCount, indexCount, normals, indexSize);
	CalculateTangents(vertices, normals, uv, indices, vertexCount, indexCount, tangents, bitangents, indexSize);
}

void MeshUtility::Clip2D(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback)
{
	TriangleClipper2D clipper;
	clipper.Clip(vertices, uvs, triangleCount, vertexStride, clipPlanes, writeCallback);
}

void MeshUtility::Clip3D(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector3*, Vector2*, u32)>& writeCallback)
{
	TriangleClipper3D clipper;
	clipper.Clip(vertices, uvs, triangleCount, vertexStride, clipPlanes, writeCallback);
}

void MeshUtility::PackNormals(Vector3* source, u8* destination, u32 count, u32 inputStride, u32 outputStride)
{
	u8* sourcePointer = (u8*)source;
	u8* destinationPointer = destination;
	for(u32 normalIndex = 0; normalIndex < count; normalIndex++)
	{
		Vector3 sourceNormal = *(Vector3*)sourcePointer;

		PackedNormal& packed = *(PackedNormal*)destinationPointer;
		packed.X = Math::Clamp((int)(sourceNormal.X * 127.5f + 127.5f), 0, 255);
		packed.Y = Math::Clamp((int)(sourceNormal.Y * 127.5f + 127.5f), 0, 255);
		packed.Z = Math::Clamp((int)(sourceNormal.Z * 127.5f + 127.5f), 0, 255);
		packed.W = 128;

		sourcePointer += inputStride;
		destinationPointer += outputStride;
	}
}

void MeshUtility::PackNormals(Vector4* source, u8* destination, u32 count, u32 inputStride, u32 outputStride)
{
	u8* sourcePointer = (u8*)source;
	u8* destinationPointer = destination;
	for(u32 normalIndex = 0; normalIndex < count; normalIndex++)
	{
		Vector4 sourceNormal = *(Vector4*)sourcePointer;
		PackedNormal& packed = *(PackedNormal*)destinationPointer;

		packed.X = Math::Clamp((int)(sourceNormal.X * 127.5f + 127.5f), 0, 255);
		packed.Y = Math::Clamp((int)(sourceNormal.Y * 127.5f + 127.5f), 0, 255);
		packed.Z = Math::Clamp((int)(sourceNormal.Z * 127.5f + 127.5f), 0, 255);
		packed.W = Math::Clamp((int)(sourceNormal.W * 127.5f + 127.5f), 0, 255);

		sourcePointer += inputStride;
		destinationPointer += outputStride;
	}
}

void MeshUtility::UnpackNormals(u8* source, Vector3* destination, u32 count, u32 stride)
{
	u8* pointer = source;
	for(u32 normalIndex = 0; normalIndex < count; normalIndex++)
	{
		destination[normalIndex] = UnpackNormal(pointer);

		pointer += stride;
	}
}

void MeshUtility::UnpackNormals(u8* source, Vector4* destination, u32 count, u32 stride)
{
	u8* pointer = source;
	for(u32 normalIndex = 0; normalIndex < count; normalIndex++)
	{
		PackedNormal& packed = *(PackedNormal*)pointer;

		const float inverse = (1.0f / 255.0f) * 2.0f;
		destination[normalIndex].X = (packed.X * inverse - 1.0f);
		destination[normalIndex].Y = (packed.Y * inverse - 1.0f);
		destination[normalIndex].Z = (packed.Z * inverse - 1.0f);
		destination[normalIndex].W = (packed.W * inverse - 1.0f);

		pointer += stride;
	}
}
