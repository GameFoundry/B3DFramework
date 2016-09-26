//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMeshUtility.h"
#include "BsVector4.h"
#include "BsVector3.h"
#include "BsVector2.h"
#include "BsPlane.h"

namespace BansheeEngine
{
	struct VertexFaces
	{
		UINT32* faces;
		UINT32 numFaces = 0;
	};

	struct VertexConnectivity
	{
		VertexConnectivity(UINT8* indices, UINT32 numVertices, UINT32 numFaces, UINT32 indexSize)
			:vertexFaces(nullptr), mMaxFacesPerVertex(0), mNumVertices(numVertices), mFaces(nullptr)
		{
			vertexFaces = bs_newN<VertexFaces>(numVertices);

			resizeFaceArray(10);

			for (UINT32 i = 0; i < numFaces; i++)
			{
				for (UINT32 j = 0; j < 3; j++)
				{
					UINT32 idx = i * 3 + j;
					UINT32 vertexIdx = 0;
					memcpy(&vertexIdx, indices + idx * indexSize, indexSize);

					assert(vertexIdx < mNumVertices);
					VertexFaces& faces = vertexFaces[vertexIdx];
					if (faces.numFaces >= mMaxFacesPerVertex)
						resizeFaceArray(mMaxFacesPerVertex * 2);

					faces.faces[faces.numFaces] = i;
					faces.numFaces++;
				}
			}
		}

		~VertexConnectivity()
		{
			if (vertexFaces != nullptr)
				bs_deleteN(vertexFaces, mNumVertices);

			if (mFaces != nullptr)
				bs_free(mFaces);
		}

		VertexFaces* vertexFaces;

	private:
		void resizeFaceArray(UINT32 numFaces)
		{
			UINT32* newFaces = (UINT32*)bs_alloc(numFaces * mNumVertices * sizeof(UINT32));

			if (mFaces != nullptr)
			{
				for (UINT32 i = 0; i < mNumVertices; i++)
					memcpy(newFaces + (i * numFaces), mFaces + (i * mMaxFacesPerVertex), mMaxFacesPerVertex * sizeof(UINT32));

				bs_free(mFaces);
			}

			for (UINT32 i = 0; i < mNumVertices; i++)
				vertexFaces[i].faces = newFaces + (i * numFaces);

			mFaces = newFaces;
			mMaxFacesPerVertex = numFaces;
		}

		UINT32 mMaxFacesPerVertex;
		UINT32 mNumVertices;
		UINT32* mFaces;
	};

	/** Provides base methods required for clipping of arbitrary triangles. */
	class TriangleClipperBase // Implementation from: http://www.geometrictools.com/Documentation/ClipMesh.pdf
	{
	protected:
		/** Single vertex in the clipped mesh. */
		struct ClipVert
		{
			ClipVert() { }

			Vector3 point = Vector3::ZERO;
			Vector2 uv = Vector2::ZERO;
			float distance = 0.0f;
			UINT32 occurs = 0;
			bool visible = true;
		};

		/** Single edge in the clipped mesh. */
		struct ClipEdge
		{
			ClipEdge() { }

			UINT32 verts[2];
			Vector<UINT32> faces;
			bool visible = true;
		};

		/** Single polygon in the clipped mesh. */
		struct ClipFace
		{
			ClipFace() { }

			Vector<UINT32> edges;
			bool visible = true;
			Vector3 normal = Vector3::ZERO;
		};

		/** Contains vertices, edges and faces of the clipped mesh. */
		struct ClipMesh
		{
			ClipMesh() { }

			Vector<ClipVert> verts;
			Vector<ClipEdge> edges;
			Vector<ClipFace> faces;
		};

	protected:
		/** 
		 * Register all edges and faces, using the mesh vertices as a basis. Assumes vertices are not indexed and that
		 * every three vertices form a face
		 */
		void addEdgesAndFaces();

		/** Clips the current mesh with the provided plane. */
		INT32 clipByPlane(const Plane& plane);

		/** Clips vertices of the current mesh by the provided plane. */
		INT32 processVertices(const Plane& plane);

		/** Clips edges of the current mesh. processVertices() must be called beforehand. */
		void processEdges();

		/** Clips the faces (polygons) of the current mesh. processEdges() must be called beforehand. */
		void processFaces();

		/** 
		 * Returns a set of non-culled vertex indices for every visible face in the mesh. This should be called after
		 * clipping operation is complete to retrieve valid vertices.		 
		 */
		void getOrderedFaces(FrameVector<FrameVector<UINT32>>& sortedFaces);

		/** Returns a set of ordered and non-culled vertices for the provided face of the mesh */
		void getOrderedVertices(const ClipFace& face, UINT32* vertices);

		/** Calculates the normal for vertices related to the provided vertex indices. */
		Vector3 getNormal(UINT32* sortedVertices, UINT32 numVertices);

		/** 
		 * Checks is the polygon shape of the provided face open or closed. If open, returns true and outputs endpoints of
		 * the polyline.
		 */
		bool getOpenPolyline(ClipFace& face, UINT32& start, UINT32& end);

		ClipMesh mesh;
	};

	void TriangleClipperBase::addEdgesAndFaces()
	{
		UINT32 numTris = (UINT32)mesh.verts.size() / 3;

		UINT32 numEdges = numTris * 3;
		mesh.edges.resize(numEdges);
		mesh.faces.resize(numTris);

		for (UINT32 i = 0; i < numTris; i++)
		{
			UINT32 idx0 = i * 3 + 0;
			UINT32 idx1 = i * 3 + 1;
			UINT32 idx2 = i * 3 + 2;

			ClipEdge& clipEdge0 = mesh.edges[idx0];
			clipEdge0.verts[0] = idx0;
			clipEdge0.verts[1] = idx1;

			ClipEdge& clipEdge1 = mesh.edges[idx1];
			clipEdge1.verts[0] = idx1;
			clipEdge1.verts[1] = idx2;

			ClipEdge& clipEdge2 = mesh.edges[idx2];
			clipEdge2.verts[0] = idx2;
			clipEdge2.verts[1] = idx0;

			ClipFace& clipFace = mesh.faces[i];

			clipFace.edges.push_back(idx0);
			clipFace.edges.push_back(idx1);
			clipFace.edges.push_back(idx2);

			clipEdge0.faces.push_back(i);
			clipEdge1.faces.push_back(i);
			clipEdge2.faces.push_back(i);

			UINT32 verts[] = { idx0, idx1, idx2, idx0 };
			for (UINT32 j = 0; j < 3; j++)
				clipFace.normal += Vector3::cross(mesh.verts[verts[j]].point, mesh.verts[verts[j + 1]].point);

			clipFace.normal.normalize();
		}
	}

	INT32 TriangleClipperBase::clipByPlane(const Plane& plane)
	{
		int state = processVertices(plane);

		if (state == 1)
			return +1; // Nothing is clipped
		else if (state == -1)
			return -1; // Everything is clipped

		processEdges();
		processFaces();

		return 0;
	}

	INT32 TriangleClipperBase::processVertices(const Plane& plane)
	{
		static const float EPSILON = 0.00001f;

		// Compute signed distances from vertices to plane
		int positive = 0, negative = 0;
		for (UINT32 i = 0; i < (UINT32)mesh.verts.size(); i++)
		{
			ClipVert& vertex = mesh.verts[i];

			if (vertex.visible)
			{
				vertex.distance = Vector3::dot(plane.normal, vertex.point) - plane.d;
				if (vertex.distance >= EPSILON)
				{
					positive++;
				}
				else if (vertex.distance <= -EPSILON)
				{
					negative++;
					vertex.visible = false;
				}
				else
				{
					// Point on the plane within floating point tolerance
					vertex.distance = 0;
				}
			}
		}
		if (negative == 0)
		{
			// All vertices on nonnegative side, no clipping
			return +1;
		}
		if (positive == 0)
		{
			// All vertices on nonpositive side, everything clipped
			return -1;
		}

		return 0;
	}

	void TriangleClipperBase::processEdges()
	{
		for (UINT32 i = 0; i < (UINT32)mesh.edges.size(); i++)
		{
			ClipEdge& edge = mesh.edges[i];

			if (edge.visible)
			{
				const ClipVert& v0 = mesh.verts[edge.verts[0]];
				const ClipVert& v1 = mesh.verts[edge.verts[1]];

				float d0 = v0.distance;
				float d1 = v1.distance;

				if (d0 <= 0 && d1 <= 0)
				{
					// Edge is culled, remove edge from faces sharing it
					for (UINT32 j = 0; j < (UINT32)edge.faces.size(); j++)
					{
						ClipFace& face = mesh.faces[edge.faces[j]];

						auto iterFind = std::find(face.edges.begin(), face.edges.end(), i);
						if (iterFind != face.edges.end())
						{
							face.edges.erase(iterFind);

							if (face.edges.empty())
								face.visible = false;
						}
					}

					edge.visible = false;
					continue;
				}

				if (d0 >= 0 && d1 >= 0)
				{
					// Edge is on nonnegative side, faces retain the edge
					continue;
				}

				// The edge is split by the plane. Compute the point of intersection.
				// If the old edge is <V0,V1> and I is the intersection point, the new
				// edge is <V0,I> when d0 > 0 or <I,V1> when d1 > 0.
				float t = d0 / (d0 - d1);
				Vector3 intersectPt = (1 - t)*v0.point + t*v1.point;
				Vector2 intersectUv = (1 - t)*v0.uv + t*v1.uv;

				UINT32 newVertIdx = (UINT32)mesh.verts.size();
				mesh.verts.push_back(ClipVert());

				ClipVert& newVert = mesh.verts.back();
				newVert.point = intersectPt;
				newVert.uv = intersectUv;

				if (d0 > 0)
					mesh.edges[i].verts[1] = newVertIdx;
				else
					mesh.edges[i].verts[0] = newVertIdx;
			}
		}
	}

	void TriangleClipperBase::processFaces()
	{
		for (UINT32 i = 0; i < (UINT32)mesh.faces.size(); i++)
		{
			ClipFace& face = mesh.faces[i];

			if (face.visible)
			{
				// The edge is culled. If the edge is exactly on the clip
				// plane, it is possible that a visible triangle shares it.
				// The edge will be re-added during the face loop.

				for (UINT32 j = 0; j < (UINT32)face.edges.size(); j++)
				{
					ClipEdge& edge = mesh.edges[face.edges[j]];
					ClipVert& v0 = mesh.verts[edge.verts[0]];
					ClipVert& v1 = mesh.verts[edge.verts[1]];

					v0.occurs = 0;
					v1.occurs = 0;
				}
			}

			UINT32 start, end;
			if (getOpenPolyline(mesh.faces[i], start, end))
			{
				// Polyline is open, close it
				UINT32 closeEdgeIdx = (UINT32)mesh.edges.size();
				mesh.edges.push_back(ClipEdge());
				ClipEdge& closeEdge = mesh.edges.back();

				closeEdge.verts[0] = start;
				closeEdge.verts[1] = end;

				closeEdge.faces.push_back(i);
				face.edges.push_back(closeEdgeIdx);
			}
		}
	}

	bool TriangleClipperBase::getOpenPolyline(ClipFace& face, UINT32& start, UINT32& end)
	{
		// Count the number of occurrences of each vertex in the polyline. The
		// resulting "occurs" values must be 1 or 2.
		for (UINT32 i = 0; i < (UINT32)face.edges.size(); i++)
		{
			ClipEdge& edge = mesh.edges[face.edges[i]];

			if (edge.visible)
			{
				ClipVert& v0 = mesh.verts[edge.verts[0]];
				ClipVert& v1 = mesh.verts[edge.verts[1]];

				v0.occurs++;
				v1.occurs++;
			}
		}

		// Determine if the polyline is open
		bool gotStart = false;
		bool gotEnd = false;
		for (UINT32 i = 0; i < (UINT32)face.edges.size(); i++)
		{
			const ClipEdge& edge = mesh.edges[face.edges[i]];

			const ClipVert& v0 = mesh.verts[edge.verts[0]];
			const ClipVert& v1 = mesh.verts[edge.verts[1]];

			if (v0.occurs == 1)
			{
				if (!gotStart)
				{
					start = edge.verts[0];
					gotStart = true;
				}
				else if (!gotEnd)
				{
					end = edge.verts[0];
					gotEnd = true;
				}
			}

			if (v1.occurs == 1)
			{
				if (!gotStart)
				{
					start = edge.verts[1];
					gotStart = true;
				}
				else if (!gotEnd)
				{
					end = edge.verts[1];
					gotEnd = true;
				}
			}
		}

		return gotStart;
	}

	void TriangleClipperBase::getOrderedFaces(FrameVector<FrameVector<UINT32>>& sortedFaces)
	{
		for (UINT32 i = 0; i < (UINT32)mesh.faces.size(); i++)
		{
			const ClipFace& face = mesh.faces[i];

			if (face.visible)
			{
				// Get the ordered vertices of the face. The first and last
				// element of the array are the same since the polyline is
				// closed.
				UINT32 numSortedVerts = (UINT32)face.edges.size() + 1;
				UINT32* sortedVerts = (UINT32*)bs_stack_alloc(sizeof(UINT32) * numSortedVerts);

				getOrderedVertices(face, sortedVerts);

				FrameVector<UINT32> faceVerts;

				// The convention is that the vertices should be counterclockwise
				// ordered when viewed from the negative side of the plane of the
				// face. If you need the opposite convention, switch the
				// inequality in the if-else statement.
				Vector3 normal = getNormal(sortedVerts, numSortedVerts);
				if (Vector3::dot(mesh.faces[i].normal, normal) < 0)
				{
					// Clockwise, need to swap
					for (INT32 j = (INT32)numSortedVerts - 2; j >= 0; j--)
						faceVerts.push_back(sortedVerts[j]);
				}
				else
				{
					// Counterclockwise
					for (int j = 0; j <= (INT32)numSortedVerts - 2; j++)
						faceVerts.push_back(sortedVerts[j]);
				}

				sortedFaces.push_back(faceVerts);
				bs_stack_free(sortedVerts);
			}
		}
	}

	void TriangleClipperBase::getOrderedVertices(const ClipFace& face, UINT32* sortedVerts)
	{
		UINT32 numEdges = (UINT32)face.edges.size();
		UINT32* sortedEdges = (UINT32*)bs_stack_alloc(sizeof(UINT32) * numEdges);
		for (UINT32 i = 0; i < numEdges; i++)
			sortedEdges[i] = face.edges[i];

		// Bubble sort to arrange edges in contiguous order
		for (UINT32 i0 = 0, i1 = 1, choice = 1; i1 < numEdges - 1; i0 = i1, i1++)
		{
			const ClipEdge& edge0 = mesh.edges[sortedEdges[i0]];

			UINT32 current = edge0.verts[choice];
			for (UINT32 j = i1; j < numEdges; j++)
			{
				const ClipEdge& edge1 = mesh.edges[sortedEdges[j]];

				if (edge1.verts[0] == current || edge1.verts[1] == current)
				{
					std::swap(sortedEdges[i1], sortedEdges[j]);
					choice = 1;
					break;
				}
			}
		}

		// Add the first two vertices
		sortedVerts[0] = mesh.edges[sortedEdges[0]].verts[0];
		sortedVerts[1] = mesh.edges[sortedEdges[0]].verts[1];

		// Add the remaining vertices
		for (UINT32 i = 1; i < numEdges; i++)
		{
			const ClipEdge& edge = mesh.edges[sortedEdges[i]];

			if (edge.verts[0] == sortedVerts[i])
				sortedVerts[i + 1] = edge.verts[1];
			else
				sortedVerts[i + 1] = edge.verts[0];
		}

		bs_stack_free(sortedEdges);
	}

	Vector3 TriangleClipperBase::getNormal(UINT32* sortedVertices, UINT32 numVertices)
	{
		Vector3 normal(BsZero);
		for (UINT32 i = 0; i <= numVertices - 2; i++)
			normal += Vector3::cross(mesh.verts[sortedVertices[i]].point, mesh.verts[sortedVertices[i + 1]].point);

		normal.normalize();
		return normal;
	}

	/** Clips two-dimensional triangles against a set of provided planes. */
	class TriangleClipper2D : public TriangleClipperBase
	{
	public:
		/** @copydoc MeshUtility::clip2D */
		void clip(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
			const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback);

	private:
		/** Converts clipped vertices back into triangles and outputs them via the provided callback. */
		void convertToMesh(const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback);

		static const int BUFFER_SIZE = 64 * 3; // Must be a multiple of three
		Vector2 vertexBuffer[BUFFER_SIZE];
		Vector2 uvBuffer[BUFFER_SIZE];
	};

	void TriangleClipper2D::clip(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
		const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback)
	{
		// Add vertices
		UINT32 numVertices = numTris * 3;
		mesh.verts.resize(numVertices);

		if (uvs != nullptr)
		{
			for (UINT32 i = 0; i < numVertices; i++)
			{
				ClipVert& clipVert = mesh.verts[i];
				Vector2 vector2D = *(Vector2*)(vertices + vertexStride * i);

				clipVert.point = Vector3(vector2D.x, vector2D.y, 0.0f);
				clipVert.uv = *(Vector2*)(uvs + vertexStride * i);
			}
		}
		else
		{
			for (UINT32 i = 0; i < numVertices; i++)
			{
				ClipVert& clipVert = mesh.verts[i];
				Vector2 vector2D = *(Vector2*)(vertices + vertexStride * i);

				clipVert.point = Vector3(vector2D.x, vector2D.y, 0.0f);
			}
		}

		addEdgesAndFaces();

		for (int i = 0; i < 4; i++)
		{
			if (clipByPlane(clipPlanes[i]) == -1)
				return;
		}

		convertToMesh(writeCallback);
	}

	void TriangleClipper2D::convertToMesh(const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback)
	{
		bs_frame_mark();
		{
			FrameVector<FrameVector<UINT32>> allFaces;
			getOrderedFaces(allFaces);

			// Note: Consider using Delaunay triangulation to avoid skinny triangles
			UINT32 numWritten = 0;
			assert(BUFFER_SIZE % 3 == 0);
			for (auto& face : allFaces)
			{
				for (UINT32 i = 0; i < (UINT32)face.size() - 2; i++)
				{
					const Vector3& v0 = mesh.verts[face[0]].point;
					const Vector3& v1 = mesh.verts[face[i + 1]].point;
					const Vector3& v2 = mesh.verts[face[i + 2]].point;

					vertexBuffer[numWritten] = Vector2(v0.x, v0.y);
					uvBuffer[numWritten] = mesh.verts[face[0]].uv;
					numWritten++;

					vertexBuffer[numWritten] = Vector2(v1.x, v1.y);
					uvBuffer[numWritten] = mesh.verts[face[i + 1]].uv;
					numWritten++;

					vertexBuffer[numWritten] = Vector2(v2.x, v2.y);
					uvBuffer[numWritten] = mesh.verts[face[i + 2]].uv;
					numWritten++;

					// Only need to check this here since we guarantee the buffer is in multiples of three
					if (numWritten >= BUFFER_SIZE)
					{
						writeCallback(vertexBuffer, uvBuffer, numWritten);
						numWritten = 0;
					}
				}
			}

			if (numWritten > 0)
				writeCallback(vertexBuffer, uvBuffer, numWritten);
		}
		bs_frame_clear();
	}

	/** Clips three-dimensional triangles against a set of provided planes. */
	class TriangleClipper3D : public TriangleClipperBase
	{
	public:
		/** @copydoc MeshUtility::clip3D */
		void clip(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
			const std::function<void(Vector3*, Vector2*, UINT32)>& writeCallback);

	private:
		/** Converts clipped vertices back into triangles and outputs them via the provided callback. */
		void convertToMesh(const std::function<void(Vector3*, Vector2*, UINT32)>& writeCallback);

		static const int BUFFER_SIZE = 64 * 3; // Must be a multiple of three
		Vector3 vertexBuffer[BUFFER_SIZE];
		Vector2 uvBuffer[BUFFER_SIZE];
	};

	void TriangleClipper3D::clip(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
		const std::function<void(Vector3*, Vector2*, UINT32)>& writeCallback)
	{
		// Add vertices
		UINT32 numVertices = numTris * 3;
		mesh.verts.resize(numVertices);

		if (uvs != nullptr)
		{
			for (UINT32 i = 0; i < numVertices; i++)
			{
				ClipVert& clipVert = mesh.verts[i];

				clipVert.point = *(Vector3*)(vertices + vertexStride * i);
				clipVert.uv = *(Vector2*)(uvs + vertexStride * i);
			}
		}
		else
		{
			for (UINT32 i = 0; i < numVertices; i++)
			{
				ClipVert& clipVert = mesh.verts[i];
				Vector2 vector2D = *(Vector2*)(vertices + vertexStride * i);

				clipVert.point = Vector3(vector2D.x, vector2D.y, 0.0f);
			}
		}

		addEdgesAndFaces();

		for (int i = 0; i < 4; i++)
		{
			if (clipByPlane(clipPlanes[i]) == -1)
				return;
		}

		convertToMesh(writeCallback);
	}

	void TriangleClipper3D::convertToMesh(const std::function<void(Vector3*, Vector2*, UINT32)>& writeCallback)
	{
		bs_frame_mark();
		{
			FrameVector<FrameVector<UINT32>> allFaces;
			getOrderedFaces(allFaces);

			// Note: Consider using Delaunay triangulation to avoid skinny triangles
			UINT32 numWritten = 0;
			assert(BUFFER_SIZE % 3 == 0);
			for (auto& face : allFaces)
			{
				for (UINT32 i = 0; i < (UINT32)face.size() - 2; i++)
				{
					vertexBuffer[numWritten] = mesh.verts[face[0]].point;
					uvBuffer[numWritten] = mesh.verts[face[0]].uv;
					numWritten++;

					vertexBuffer[numWritten] = mesh.verts[face[i + 1]].point;
					uvBuffer[numWritten] = mesh.verts[face[i + 1]].uv;
					numWritten++;

					vertexBuffer[numWritten] = mesh.verts[face[i + 2]].point;
					uvBuffer[numWritten] = mesh.verts[face[i + 2]].uv;
					numWritten++;

					// Only need to check this here since we guarantee the buffer is in multiples of three
					if (numWritten >= BUFFER_SIZE)
					{
						writeCallback(vertexBuffer, uvBuffer, numWritten);
						numWritten = 0;
					}
				}
			}

			if (numWritten > 0)
				writeCallback(vertexBuffer, uvBuffer, numWritten);
		}
		bs_frame_clear();
	}

	void MeshUtility::calculateNormals(Vector3* vertices, UINT8* indices, UINT32 numVertices,
		UINT32 numIndices, Vector3* normals, UINT32 indexSize)
	{
		UINT32 numFaces = numIndices / 3;

		Vector3* faceNormals = bs_newN<Vector3>(numFaces);
		for (UINT32 i = 0; i < numFaces; i++)
		{
			UINT32 triangle[3];
			memcpy(&triangle[0], indices + (i * 3 + 0) * indexSize, indexSize);
			memcpy(&triangle[1], indices + (i * 3 + 1) * indexSize, indexSize);
			memcpy(&triangle[2], indices + (i * 3 + 2) * indexSize, indexSize);

			Vector3 edgeA = vertices[triangle[1]] - vertices[triangle[0]];
			Vector3 edgeB = vertices[triangle[2]] - vertices[triangle[0]];
			faceNormals[i] = Vector3::normalize(Vector3::cross(edgeA, edgeB));

			// Note: Potentially don't normalize here in order to weigh the normals
			// by triangle size
		}

		VertexConnectivity connectivity(indices, numVertices, numFaces, indexSize);
		for (UINT32 i = 0; i < numVertices; i++)
		{
			VertexFaces& faces = connectivity.vertexFaces[i];

			normals[i] = Vector3::ZERO;
			for (UINT32 j = 0; j < faces.numFaces; j++)
			{
				UINT32 faceIdx = faces.faces[j];
				normals[i] += faceNormals[faceIdx];
			}

			normals[i].normalize();
		}

		bs_deleteN(faceNormals, numFaces);
	}

	void MeshUtility::calculateTangents(Vector3* vertices, Vector3* normals, Vector2* uv, UINT8* indices, UINT32 numVertices,
		UINT32 numIndices, Vector3* tangents, Vector3* bitangents, UINT32 indexSize)
	{
		UINT32 numFaces = numIndices / 3;

		Vector3* faceTangents = bs_newN<Vector3>(numFaces);
		Vector3* faceBitangents = bs_newN<Vector3>(numFaces);
		for (UINT32 i = 0; i < numFaces; i++)
		{
			UINT32 triangle[3];
			memcpy(&triangle[0], indices + (i * 3 + 0) * indexSize, indexSize);
			memcpy(&triangle[1], indices + (i * 3 + 1) * indexSize, indexSize);
			memcpy(&triangle[2], indices + (i * 3 + 2) * indexSize, indexSize);

			Vector3 p0 = vertices[triangle[0]];
			Vector3 p1 = vertices[triangle[1]];
			Vector3 p2 = vertices[triangle[2]];

			Vector2 uv0 = uv[triangle[0]];
			Vector2 uv1 = uv[triangle[1]];
			Vector2 uv2 = uv[triangle[2]];

			Vector3 q0 = p1 - p0;
			Vector3 q1 = p2 - p0;

			Vector2 s;
			s.x = uv1.x - uv0.x;
			s.y = uv2.x - uv0.x;

			Vector2 t;
			t.x = uv1.y - uv0.y;
			t.y = uv2.y - uv0.y;

			float denom = s.x*t.y - s.y * t.x;
			if (fabs(denom) >= 0e-8f)
			{
				float r = 1.0f / denom;
				s *= r;
				t *= r;

				faceTangents[i] = t.y * q0 - t.x * q1;
				faceBitangents[i] = s.x * q0 - s.y * q1;

				faceTangents[i].normalize();
				faceBitangents[i].normalize();
			}

			// Note: Potentially don't normalize here in order to weight the normals by triangle size
		}

		VertexConnectivity connectivity(indices, numVertices, numFaces, indexSize);
		for (UINT32 i = 0; i < numVertices; i++)
		{
			VertexFaces& faces = connectivity.vertexFaces[i];

			tangents[i] = Vector3::ZERO;
			bitangents[i] = Vector3::ZERO;

			for (UINT32 j = 0; j < faces.numFaces; j++)
			{
				UINT32 faceIdx = faces.faces[j];
				tangents[i] += faceTangents[faceIdx];
				bitangents[i] += faceBitangents[faceIdx];
			}

			tangents[i].normalize();
			bitangents[i].normalize();

			// Orthonormalize
			float dot0 = normals[i].dot(tangents[i]);
			tangents[i] -= dot0*normals[i];
			tangents[i].normalize();

			float dot1 = tangents[i].dot(bitangents[i]);
			dot0 = normals[i].dot(bitangents[i]);
			bitangents[i] -= dot0*normals[i] + dot1*tangents[i];
			bitangents[i].normalize();
		}

		bs_deleteN(faceTangents, numFaces);
		bs_deleteN(faceBitangents, numFaces);

		// TODO - Consider weighing tangents by triangle size and/or edge angles
	}

	void MeshUtility::calculateTangentSpace(Vector3* vertices, Vector2* uv, UINT8* indices, UINT32 numVertices,
		UINT32 numIndices, Vector3* normals, Vector3* tangents, Vector3* bitangents, UINT32 indexSize)
	{
		calculateNormals(vertices, indices, numVertices, numIndices, normals, indexSize);
		calculateTangents(vertices, normals, uv, indices, numVertices, numIndices, tangents, bitangents, indexSize);
	}

	void MeshUtility::clip2D(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
		const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback)
	{
		TriangleClipper2D clipper;
		clipper.clip(vertices, uvs, numTris, vertexStride, clipPlanes, writeCallback);
	}

	void MeshUtility::clip3D(UINT8* vertices, UINT8* uvs, UINT32 numTris, UINT32 vertexStride, const Vector<Plane>& clipPlanes,
		const std::function<void(Vector3*, Vector2*, UINT32)>& writeCallback)
	{
		TriangleClipper3D clipper;
		clipper.clip(vertices, uvs, numTris, vertexStride, clipPlanes, writeCallback);
	}

	void MeshUtility::packNormals(Vector3* source, UINT8* destination, UINT32 count, UINT32 stride)
	{
		UINT8* ptr = destination;
		for (UINT32 i = 0; i < count; i++)
		{
			PackedNormal& packed = *(PackedNormal*)ptr;
			packed.x = Math::clamp((int)(source[i].x * 127.5f + 127.5f), 0, 255);
			packed.y = Math::clamp((int)(source[i].y * 127.5f + 127.5f), 0, 255);
			packed.z = Math::clamp((int)(source[i].z * 127.5f + 127.5f), 0, 255);
			packed.w = 128;

			ptr += stride;
		}
	}

	void MeshUtility::packNormals(Vector4* source, UINT8* destination, UINT32 count, UINT32 stride)
	{
		UINT8* ptr = destination;
		for (UINT32 i = 0; i < count; i++)
		{
			PackedNormal& packed = *(PackedNormal*)ptr;
			packed.x = Math::clamp((int)(source[i].x * 127.5f + 127.5f), 0, 255);
			packed.y = Math::clamp((int)(source[i].y * 127.5f + 127.5f), 0, 255);
			packed.z = Math::clamp((int)(source[i].z * 127.5f + 127.5f), 0, 255);
			packed.w = Math::clamp((int)(source[i].w * 127.5f + 127.5f), 0, 255);

			ptr += stride;
		}
	}

	void MeshUtility::unpackNormals(UINT8* source, Vector3* destination, UINT32 count, UINT32 stride)
	{
		UINT8* ptr = source;
		for (UINT32 i = 0; i < count; i++)
		{
			PackedNormal& packed = *(PackedNormal*)ptr;

			destination[i].x = (packed.x * 2.0f - 1.0f);
			destination[i].y = (packed.y * 2.0f - 1.0f);
			destination[i].z = (packed.z * 2.0f - 1.0f);

			ptr += stride;
		}
	}

	void MeshUtility::unpackNormals(UINT8* source, Vector4* destination, UINT32 count, UINT32 stride)
	{
		UINT8* ptr = source;
		for (UINT32 i = 0; i < count; i++)
		{
			PackedNormal& packed = *(PackedNormal*)ptr;

			destination[i].x = (packed.x * 2.0f - 1.0f);
			destination[i].y = (packed.y * 2.0f - 1.0f);
			destination[i].z = (packed.z * 2.0f - 1.0f);
			destination[i].w = (packed.w * 2.0f - 1.0f);

			ptr += stride;
		}
	}
}