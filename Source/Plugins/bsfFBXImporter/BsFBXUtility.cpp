//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFBXUtility.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"

namespace bs
{
	struct SmoothNormal
	{
		int group = 0;
		Vector3 normal = Vector3::ZERO;

		void AddNormal(int group, const Vector3& normal)
		{
			this->group |= group;
			this->normal += normal;
		}

		void AddNormal(const SmoothNormal& other)
		{
			this->group |= other.group;
			this->normal += other.normal;
		}

		void Normalize()
		{
			normal.Normalize();
		}
	};

	struct SmoothVertex
	{
		Vector<SmoothNormal> normals;

		void AddNormal(int group, const Vector3& normal)
		{
			bool found = false;

			for (size_t i = 0; i < normals.Size(); i++)
			{
				if ((normals[i].group & group) != 0)
				{
					bool otherGroups = (group & ~normals[i].group) != 0;

					if (otherGroups)
					{
						for (size_t j = i + 1; j < normals.Size(); j++)
						{
							if ((normals[j].group & group) != 0)
							{
								normals[i].AddNormal(normals[j]);
								normals.Erase(normals.begin() + j);
								--j;
							}
						}
					}

					normals[i].AddNormal(group, normal);

					found = true;
					break;;
				}
			}

			if (!found)
			{
				SmoothNormal smoothNormal;
				smoothNormal.AddNormal(group, normal);

				normals.push_back(smoothNormal);
			}
		}

		Vector3 GetNormal(int group) const
		{
			for (size_t i = 0; i < normals.Size(); i++)
			{
				if (normals[i].group & group)
					return normals[i].normal;
			}

			return Vector3::ZERO;
		}

		void Normalize()
		{
			for (size_t i = 0; i < normals.Size(); ++i)
				normals[i].Normalize();
		}
	};

	void FBXUtility::normalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices,
		const Vector<int>& smoothing, Vector<Vector3>& normals)
	{
		std::vector<SmoothVertex> smoothNormals;
		smoothNormals.Resize(positions.size());

		normals.Resize(indices.size(), Vector3::ZERO);

		UINT32 numPolygons = (UINT32)(indices.Size() / 3);

		int idx = 0;
		for (UINT32 i = 0; i < numPolygons; i++)
		{
			for (UINT32 j = 0; j < 3; j++)
			{
				int prevOffset = (j > 0 ? j - 1 : 2);
				int nextOffset = (j < 2 ? j + 1 : 0);

				int current = indices[idx + j];

				Vector3 v0 = (Vector3)positions[indices[idx + prevOffset]];
				Vector3 v1 = (Vector3)positions[current];
				Vector3 v2 = (Vector3)positions[indices[idx + nextOffset]];

				Vector3 normal = Vector3::cross(v0 - v1, v2 - v1);
				smoothNormals[current].AddNormal(smoothing[idx + j], normal);

				normals[idx + j] = normal;
			}

			idx += 3;
		}

		for (size_t i = 0; i < smoothNormals.Size(); ++i)
			smoothNormals[i].Normalize();

		idx = 0;
		for (UINT32 i = 0; i < numPolygons; i++)
		{
			for (UINT32 j = 0; j < 3; j++)
			{
				if (smoothing[idx + j] != 0)
				{
					int current = indices[idx + j];

					normals[idx + j] = smoothNormals[current].GetNormal(smoothing[idx + j]);
				}
			}

			idx += 3;
		}
	}

	void FBXUtility::SplitVertices(const FBXImportMesh& source, FBXImportMesh& dest)
	{
		dest.indices = source.indices;
		dest.materials = source.materials;

		dest.positions = source.positions;
		dest.boneInfluences = source.boneInfluences;

		// Make room for minimal set of vertices
		UINT32 vertexCount = (UINT32)source.positions.Size();
		if (!source.normals.Empty())
			dest.normals.Resize(vertexCount);

		if (!source.tangents.Empty())
			dest.tangents.Resize(vertexCount);

		if (!source.bitangents.Empty())
			dest.bitangents.Resize(vertexCount);

		if (!source.colors.Empty())
			dest.colors.Resize(vertexCount);

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!source.UV[i].Empty())
				dest.UV[i].Resize(vertexCount);
		}

		UINT32 numBlendShapes = (UINT32)source.blendShapes.Size();
		dest.blendShapes.Resize(numBlendShapes);
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = source.blendShapes[i];
			FBXBlendShape& destShape = dest.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.Size();
			destShape.frames.Resize(numFrames);
			destShape.name = sourceShape.name;

			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				destFrame.name = sourceFrame.name;
				destFrame.weight = sourceFrame.weight;
				destFrame.positions = sourceFrame.positions;

				if (!sourceFrame.normals.Empty())
					destFrame.normals.Resize(vertexCount);

				if (!sourceFrame.tangents.Empty())
					destFrame.tangents.Resize(vertexCount);

				if (!sourceFrame.bitangents.Empty())
					destFrame.bitangents.Resize(vertexCount);
			}
		}

		Vector<Vector<int>> splitsPerVertex;
		splitsPerVertex.Resize(source.positions.size());

		Vector<int>& indices = dest.indices;
		int indexCount = (int)dest.indices.Size();
		for (int i = 0; i < indexCount; i++)
		{
			int srcVertIdx = indices[i];
			int dstVertIdx = -1;

			// See if we already processed this vertex and if its attributes
			// are close enough with the previous version
			Vector<int>& splits = splitsPerVertex[srcVertIdx];
			for (auto& splitVertIdx : splits)
			{
				if (!needsSplitAttributes(source, i, dest, splitVertIdx))
					dstVertIdx = splitVertIdx;
			}

			// We didn't find a close-enough match
			if (dstVertIdx == -1)
			{
				// First time we visited this vertex, so just copy over attributes
				if (splits.Empty())
				{
					dstVertIdx = srcVertIdx;
					copyVertexAttributes(source, i, dest, dstVertIdx);
				}
				else // Split occurred, add a brand new vertex
				{
					dstVertIdx = (int)dest.positions.Size();
					addVertex(source, i, srcVertIdx, dest);
				}

				splits.push_back(dstVertIdx);
			}

			indices[i] = dstVertIdx;
		}
	}

	void FBXUtility::FlipWindingOrder(FBXImportMesh& input)
	{
		for (UINT32 i = 0; i < (UINT32)input.materials.Size(); i += 3)
		{
			std::swap(input.materials[i + 1], input.materials[i + 2]);
		}

		for (UINT32 i = 0; i < (UINT32)input.indices.Size(); i += 3)
		{
			std::swap(input.indices[i + 1], input.indices[i + 2]);
		}
	}

	void FBXUtility::CopyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx)
	{
		if (!srcMesh.normals.Empty())
			destMesh.normals[dstIdx] = srcMesh.normals[srcIdx];

		if (!srcMesh.tangents.Empty())
			destMesh.tangents[dstIdx] = srcMesh.tangents[srcIdx];

		if (!srcMesh.bitangents.Empty())
			destMesh.bitangents[dstIdx] = srcMesh.bitangents[srcIdx];

		if (!srcMesh.colors.Empty())
			destMesh.colors[dstIdx] = srcMesh.colors[srcIdx];

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!srcMesh.UV[i].Empty())
				destMesh.UV[i][dstIdx] = srcMesh.UV[i][srcIdx];
		}

		UINT32 numBlendShapes = (UINT32)srcMesh.blendShapes.Size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = srcMesh.blendShapes[i];
			FBXBlendShape& destShape = destMesh.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.Size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				if (!sourceFrame.normals.Empty())
					destFrame.normals[dstIdx] = sourceFrame.normals[srcIdx];

				if (!sourceFrame.tangents.Empty())
					destFrame.tangents[dstIdx] = sourceFrame.tangents[srcIdx];

				if (!sourceFrame.bitangents.Empty())
					destFrame.bitangents[dstIdx] = sourceFrame.bitangents[srcIdx];
			}
		}
	}

	void FBXUtility::AddVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh)
	{
		destMesh.positions.push_back(srcMesh.positions[srcVertex]);

		if (!srcMesh.boneInfluences.Empty())
			destMesh.boneInfluences.push_back(srcMesh.boneInfluences[srcVertex]);

		if (!srcMesh.normals.Empty())
			destMesh.normals.push_back(srcMesh.normals[srcIdx]);

		if (!srcMesh.tangents.Empty())
			destMesh.tangents.push_back(srcMesh.tangents[srcIdx]);

		if (!srcMesh.bitangents.Empty())
			destMesh.bitangents.push_back(srcMesh.bitangents[srcIdx]);

		if (!srcMesh.colors.Empty())
			destMesh.colors.push_back(srcMesh.colors[srcIdx]);

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!srcMesh.UV[i].Empty())
				destMesh.UV[i].push_back(srcMesh.UV[i][srcIdx]);
		}

		UINT32 numBlendShapes = (UINT32)srcMesh.blendShapes.Size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = srcMesh.blendShapes[i];
			FBXBlendShape& destShape = destMesh.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.Size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				destFrame.positions.push_back(sourceFrame.positions[srcVertex]);

				if (!sourceFrame.normals.Empty())
					destFrame.normals.push_back(sourceFrame.normals[srcIdx]);

				if (!sourceFrame.tangents.Empty())
					destFrame.tangents.push_back(sourceFrame.tangents[srcIdx]);

				if (!sourceFrame.bitangents.Empty())
					destFrame.bitangents.push_back(sourceFrame.bitangents[srcIdx]);
			}
		}
	}

	bool FBXUtility::NeedsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB)
	{
		static const float SplitAngleCosine = Math::cos(Degree(1.0f));
		static const float UVEpsilon = 0.001f;

		if (!meshA.colors.Empty())
		{
			if (meshA.colors[idxA] != meshB.colors[idxB])
				return true;
		}

		if (!meshA.normals.Empty())
		{
			float angleCosine = meshA.normals[idxA].Dot(meshB.normals[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		if (!meshA.tangents.Empty())
		{
			float angleCosine = meshA.tangents[idxA].Dot(meshB.tangents[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		if (!meshA.bitangents.Empty())
		{
			float angleCosine = meshA.bitangents[idxA].Dot(meshB.bitangents[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!meshA.UV[i].Empty())
			{
				if (!Math::approxEquals(meshA.UV[i][idxA], meshB.UV[i][idxB], UVEpsilon))
					return true;
			}
		}

		UINT32 numBlendShapes = (UINT32)meshA.blendShapes.Size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& shapeA = meshA.blendShapes[i];
			const FBXBlendShape& shapeB = meshB.blendShapes[i];

			UINT32 numFrames = (UINT32)shapeA.frames.Size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& frameA = shapeA.frames[j];
				const FBXBlendShapeFrame& frameB = shapeB.frames[j];

				if (!frameA.normals.Empty())
				{
					float angleCosine = frameA.normals[idxA].Dot(frameB.normals[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}

				if (!frameA.tangents.Empty())
				{
					float angleCosine = frameA.tangents[idxA].Dot(frameB.tangents[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}

				if (!frameA.bitangents.Empty())
				{
					float angleCosine = frameA.bitangents[idxA].Dot(frameB.bitangents[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}
			}
		}

		return false;
	}
}
