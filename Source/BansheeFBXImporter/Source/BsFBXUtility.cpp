//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFBXUtility.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"

namespace BansheeEngine
{
	struct SmoothNormal
	{
		int group = 0;
		Vector3 normal = Vector3::ZERO;

		void addNormal(int group, const Vector3& normal)
		{
			this->group |= group;
			this->normal += normal;
		}

		void addNormal(const SmoothNormal& other)
		{
			this->group |= other.group;
			this->normal += other.normal;
		}

		void normalize()
		{
			normal.normalize();
		}
	};

	struct SmoothVertex
	{
		Vector<SmoothNormal> normals;

		void addNormal(int group, const Vector3& normal)
		{
			bool found = false;

			for (size_t i = 0; i < normals.size(); i++)
			{
				if ((normals[i].group & group) != 0)
				{
					bool otherGroups = (group & ~normals[i].group) != 0;

					if (otherGroups)
					{
						for (size_t j = i + 1; j < normals.size(); j++)
						{
							if ((normals[j].group & group) != 0)
							{
								normals[i].addNormal(normals[j]);
								normals.erase(normals.begin() + j);
								--j;
							}
						}
					}

					normals[i].addNormal(group, normal);

					found = true;
					break;;
				}
			}

			if (!found)
			{
				SmoothNormal smoothNormal;
				smoothNormal.addNormal(group, normal);

				normals.push_back(smoothNormal);
			}
		}

		Vector3 getNormal(int group) const
		{
			for (size_t i = 0; i < normals.size(); i++)
			{
				if (normals[i].group & group)
					return normals[i].normal;
			}

			return Vector3::ZERO;
		}

		void normalize()
		{
			for (size_t i = 0; i < normals.size(); ++i)
				normals[i].normalize();
		}
	};

	void FBXUtility::normalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices,
		const Vector<int>& smoothing, Vector<Vector3>& normals)
	{
		std::vector<SmoothVertex> smoothNormals;
		smoothNormals.resize(positions.size());

		normals.resize(indices.size(), Vector3::ZERO);

		UINT32 numPolygons = (UINT32)(indices.size() / 3);

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
				smoothNormals[current].addNormal(smoothing[idx + j], normal);

				normals[idx + j] = normal;
			}

			idx += 3;
		}

		for (size_t i = 0; i < smoothNormals.size(); ++i)
			smoothNormals[i].normalize();

		idx = 0;
		for (UINT32 i = 0; i < numPolygons; i++)
		{
			for (UINT32 j = 0; j < 3; j++)
			{
				if (smoothing[idx + i] != 0)
				{
					int current = indices[idx + i];

					normals[idx + i] = smoothNormals[current].getNormal(smoothing[idx + i]);
				}
			}

			idx += 3;
		}
	}

	void FBXUtility::splitVertices(const FBXImportMesh& source, FBXImportMesh& dest)
	{
		dest.indices = source.indices;
		dest.materials = source.materials;

		dest.positions = source.positions;
		dest.boneInfluences = source.boneInfluences;

		// Make room for minimal set of vertices
		UINT32 vertexCount = (UINT32)source.positions.size();
		if (!source.normals.empty())
			dest.normals.resize(vertexCount);

		if (!source.tangents.empty())
			dest.tangents.resize(vertexCount);

		if (!source.bitangents.empty())
			dest.bitangents.resize(vertexCount);

		if (!source.colors.empty())
			dest.colors.resize(vertexCount);

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!source.UV[i].empty())
				dest.UV[i].resize(vertexCount);
		}

		UINT32 numBlendShapes = (UINT32)source.blendShapes.size();
		dest.blendShapes.resize(numBlendShapes);
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = source.blendShapes[i];
			FBXBlendShape& destShape = dest.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.size();
			destShape.frames.resize(numFrames);
			destShape.name = sourceShape.name;

			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				destFrame.weight = sourceFrame.weight;
				destFrame.positions = sourceFrame.positions;

				if (!sourceFrame.normals.empty())
					destFrame.normals.resize(vertexCount);

				if (!sourceFrame.tangents.empty())
					destFrame.tangents.resize(vertexCount);

				if (!sourceFrame.bitangents.empty())
					destFrame.bitangents.resize(vertexCount);
			}
		}

		Vector<Vector<int>> splitsPerVertex;
		splitsPerVertex.resize(source.positions.size());

		Vector<int>& indices = dest.indices;
		int indexCount = (int)dest.indices.size();
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
				if (splits.empty())
				{
					dstVertIdx = srcVertIdx;
					copyVertexAttributes(source, i, dest, srcVertIdx);
				}
				else // Split occurred, add a brand new vertex
				{
					dstVertIdx = (int)dest.positions.size();
					addVertex(source, i, srcVertIdx, dest);
				}

				splits.push_back(dstVertIdx);
			}

			indices[i] = dstVertIdx;
		}
	}

	void FBXUtility::flipWindingOrder(FBXImportMesh& input)
	{
		for (UINT32 i = 0; i < (UINT32)input.materials.size(); i += 3)
		{
			std::swap(input.materials[i + 1], input.materials[i + 2]);
		}

		for (UINT32 i = 0; i < (UINT32)input.indices.size(); i += 3)
		{
			std::swap(input.indices[i + 1], input.indices[i + 2]);
		}
	}

	void FBXUtility::copyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx)
	{
		if (!srcMesh.normals.empty())
			destMesh.normals[dstIdx] = srcMesh.normals[srcIdx];

		if (!srcMesh.tangents.empty())
			destMesh.tangents[dstIdx] = srcMesh.tangents[srcIdx];

		if (!srcMesh.bitangents.empty())
			destMesh.bitangents[dstIdx] = srcMesh.bitangents[srcIdx];

		if (!srcMesh.colors.empty())
			destMesh.colors[dstIdx] = srcMesh.colors[srcIdx];

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!srcMesh.UV[i].empty())
				destMesh.UV[i][dstIdx] = srcMesh.UV[i][srcIdx];
		}

		UINT32 numBlendShapes = (UINT32)srcMesh.blendShapes.size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = srcMesh.blendShapes[i];
			FBXBlendShape& destShape = destMesh.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				if (!sourceFrame.normals.empty())
					destFrame.normals[dstIdx] = sourceFrame.normals[srcIdx];

				if (!sourceFrame.tangents.empty())
					destFrame.tangents[dstIdx] = sourceFrame.tangents[srcIdx];

				if (!sourceFrame.bitangents.empty())
					destFrame.bitangents[dstIdx] = sourceFrame.bitangents[srcIdx];
			}
		}
	}

	void FBXUtility::addVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh)
	{
		destMesh.positions.push_back(srcMesh.positions[srcVertex]);

		if (!srcMesh.boneInfluences.empty())
			destMesh.boneInfluences.push_back(srcMesh.boneInfluences[srcVertex]);

		if (!srcMesh.normals.empty())
			destMesh.normals.push_back(srcMesh.normals[srcIdx]);

		if (!srcMesh.tangents.empty())
			destMesh.tangents.push_back(srcMesh.tangents[srcIdx]);

		if (!srcMesh.bitangents.empty())
			destMesh.bitangents.push_back(srcMesh.bitangents[srcIdx]);

		if (!srcMesh.colors.empty())
			destMesh.colors.push_back(srcMesh.colors[srcIdx]);

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!srcMesh.UV[i].empty())
				destMesh.UV[i].push_back(srcMesh.UV[i][srcIdx]);
		}

		UINT32 numBlendShapes = (UINT32)srcMesh.blendShapes.size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& sourceShape = srcMesh.blendShapes[i];
			FBXBlendShape& destShape = destMesh.blendShapes[i];

			UINT32 numFrames = (UINT32)sourceShape.frames.size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& sourceFrame = sourceShape.frames[j];
				FBXBlendShapeFrame& destFrame = destShape.frames[j];

				destFrame.positions.push_back(sourceFrame.positions[srcVertex]);

				if (!sourceFrame.normals.empty())
					destFrame.normals.push_back(sourceFrame.normals[srcIdx]);

				if (!sourceFrame.tangents.empty())
					destFrame.tangents.push_back(sourceFrame.tangents[srcIdx]);

				if (!sourceFrame.bitangents.empty())
					destFrame.bitangents.push_back(sourceFrame.bitangents[srcIdx]);
			}
		}
	}

	bool FBXUtility::needsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB)
	{
		static const float SplitAngleCosine = Math::cos(Degree(1.0f));
		static const float UVEpsilon = 0.001f;

		if (!meshA.colors.empty())
		{
			if (meshA.colors[idxA] != meshB.colors[idxB])
				return true;
		}

		if (!meshA.normals.empty())
		{
			float angleCosine = meshA.normals[idxA].dot(meshB.normals[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		if (!meshA.tangents.empty())
		{
			float angleCosine = meshA.tangents[idxA].dot(meshB.tangents[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		if (!meshA.bitangents.empty())
		{
			float angleCosine = meshA.bitangents[idxA].dot(meshB.bitangents[idxB]);
			if (angleCosine < SplitAngleCosine)
				return true;
		}

		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if (!meshA.UV[i].empty())
			{
				if (!Math::approxEquals(meshA.UV[i][idxA], meshB.UV[i][idxB], UVEpsilon))
					return true;
			}
		}

		UINT32 numBlendShapes = (UINT32)meshA.blendShapes.size();
		for (UINT32 i = 0; i < numBlendShapes; i++)
		{
			const FBXBlendShape& shapeA = meshA.blendShapes[i];
			const FBXBlendShape& shapeB = meshB.blendShapes[i];

			UINT32 numFrames = (UINT32)shapeA.frames.size();
			for (UINT32 j = 0; j < numFrames; j++)
			{
				const FBXBlendShapeFrame& frameA = shapeA.frames[j];
				const FBXBlendShapeFrame& frameB = shapeB.frames[j];

				if (!frameA.normals.empty())
				{
					float angleCosine = frameA.normals[idxA].dot(frameB.normals[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}

				if (!frameA.tangents.empty())
				{
					float angleCosine = frameA.tangents[idxA].dot(frameB.tangents[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}

				if (!frameA.bitangents.empty())
				{
					float angleCosine = frameA.bitangents[idxA].dot(frameB.bitangents[idxB]);
					if (angleCosine < SplitAngleCosine)
						return true;
				}
			}
		}

		return false;
	}
}