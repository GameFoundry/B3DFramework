//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFBXUtility.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"

using namespace b3d;

struct SmoothNormal
{
	int Group = 0;
	Vector3 Normal = Vector3::kZero;

	void AddNormal(int group, const Vector3& normal)
	{
		this->Group |= group;
		this->Normal += normal;
	}

	void AddNormal(const SmoothNormal& other)
	{
		this->Group |= other.Group;
		this->Normal += other.Normal;
	}

	void Normalize()
	{
		Normal.Normalize();
	}
};

struct SmoothVertex
{
	Vector<SmoothNormal> Normals;

	void AddNormal(int group, const Vector3& normal)
	{
		bool found = false;

		for(size_t i = 0; i < Normals.size(); i++)
		{
			if((Normals[i].Group & group) != 0)
			{
				bool otherGroups = (group & ~Normals[i].Group) != 0;

				if(otherGroups)
				{
					for(size_t j = i + 1; j < Normals.size(); j++)
					{
						if((Normals[j].Group & group) != 0)
						{
							Normals[i].AddNormal(Normals[j]);
							Normals.erase(Normals.begin() + j);
							--j;
						}
					}
				}

				Normals[i].AddNormal(group, normal);

				found = true;
				break;
				;
			}
		}

		if(!found)
		{
			SmoothNormal smoothNormal;
			smoothNormal.AddNormal(group, normal);

			Normals.push_back(smoothNormal);
		}
	}

	Vector3 GetNormal(int group) const
	{
		for(size_t i = 0; i < Normals.size(); i++)
		{
			if(Normals[i].Group & group)
				return Normals[i].Normal;
		}

		return Vector3::kZero;
	}

	void Normalize()
	{
		for(size_t i = 0; i < Normals.size(); ++i)
			Normals[i].Normalize();
	}
};

void FBXUtility::NormalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices, const Vector<int>& smoothing, Vector<Vector3>& normals)
{
	std::vector<SmoothVertex> smoothNormals;
	smoothNormals.resize(positions.size());

	normals.resize(indices.size(), Vector3::kZero);

	u32 numPolygons = (u32)(indices.size() / 3);

	int idx = 0;
	for(u32 i = 0; i < numPolygons; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			int prevOffset = (j > 0 ? j - 1 : 2);
			int nextOffset = (j < 2 ? j + 1 : 0);

			int current = indices[idx + j];

			Vector3 v0 = (Vector3)positions[indices[idx + prevOffset]];
			Vector3 v1 = (Vector3)positions[current];
			Vector3 v2 = (Vector3)positions[indices[idx + nextOffset]];

			Vector3 normal = Vector3::Cross(v0 - v1, v2 - v1);
			smoothNormals[current].AddNormal(smoothing[idx + j], normal);

			normals[idx + j] = normal;
		}

		idx += 3;
	}

	for(size_t i = 0; i < smoothNormals.size(); ++i)
		smoothNormals[i].Normalize();

	idx = 0;
	for(u32 i = 0; i < numPolygons; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			if(smoothing[idx + j] != 0)
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
	dest.Indices = source.Indices;
	dest.Materials = source.Materials;

	dest.Positions = source.Positions;
	dest.BoneInfluences = source.BoneInfluences;

	// Make room for minimal set of vertices
	u32 vertexCount = (u32)source.Positions.size();
	if(!source.Normals.empty())
		dest.Normals.resize(vertexCount);

	if(!source.Tangents.empty())
		dest.Tangents.resize(vertexCount);

	if(!source.Bitangents.empty())
		dest.Bitangents.resize(vertexCount);

	if(!source.Colors.empty())
		dest.Colors.resize(vertexCount);

	for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
	{
		if(!source.UV[i].empty())
			dest.UV[i].resize(vertexCount);
	}

	u32 numBlendShapes = (u32)source.BlendShapes.size();
	dest.BlendShapes.resize(numBlendShapes);
	for(u32 i = 0; i < numBlendShapes; i++)
	{
		const FBXBlendShape& sourceShape = source.BlendShapes[i];
		FBXBlendShape& destShape = dest.BlendShapes[i];

		u32 numFrames = (u32)sourceShape.Frames.size();
		destShape.Frames.resize(numFrames);
		destShape.Name = sourceShape.Name;

		for(u32 j = 0; j < numFrames; j++)
		{
			const FBXBlendShapeFrame& sourceFrame = sourceShape.Frames[j];
			FBXBlendShapeFrame& destFrame = destShape.Frames[j];

			destFrame.Name = sourceFrame.Name;
			destFrame.Weight = sourceFrame.Weight;
			destFrame.Positions = sourceFrame.Positions;

			if(!sourceFrame.Normals.empty())
				destFrame.Normals.resize(vertexCount);

			if(!sourceFrame.Tangents.empty())
				destFrame.Tangents.resize(vertexCount);

			if(!sourceFrame.Bitangents.empty())
				destFrame.Bitangents.resize(vertexCount);
		}
	}

	Vector<Vector<int>> splitsPerVertex;
	splitsPerVertex.resize(source.Positions.size());

	Vector<int>& indices = dest.Indices;
	int indexCount = (int)dest.Indices.size();
	for(int i = 0; i < indexCount; i++)
	{
		int srcVertIdx = indices[i];
		int dstVertIdx = -1;

		// See if we already processed this vertex and if its attributes
		// are close enough with the previous version
		Vector<int>& splits = splitsPerVertex[srcVertIdx];
		for(auto& splitVertIdx : splits)
		{
			if(!NeedsSplitAttributes(source, i, dest, splitVertIdx))
				dstVertIdx = splitVertIdx;
		}

		// We didn't find a close-enough match
		if(dstVertIdx == -1)
		{
			// First time we visited this vertex, so just copy over attributes
			if(splits.empty())
			{
				dstVertIdx = srcVertIdx;
				CopyVertexAttributes(source, i, dest, dstVertIdx);
			}
			else // Split occurred, add a brand new vertex
			{
				dstVertIdx = (int)dest.Positions.size();
				AddVertex(source, i, srcVertIdx, dest);
			}

			splits.push_back(dstVertIdx);
		}

		indices[i] = dstVertIdx;
	}
}

void FBXUtility::FlipWindingOrder(FBXImportMesh& input)
{
	for(u32 i = 0; i < (u32)input.Materials.size(); i += 3)
	{
		std::swap(input.Materials[i + 1], input.Materials[i + 2]);
	}

	for(u32 i = 0; i < (u32)input.Indices.size(); i += 3)
	{
		std::swap(input.Indices[i + 1], input.Indices[i + 2]);
	}
}

void FBXUtility::CopyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx)
{
	if(!srcMesh.Normals.empty())
		destMesh.Normals[dstIdx] = srcMesh.Normals[srcIdx];

	if(!srcMesh.Tangents.empty())
		destMesh.Tangents[dstIdx] = srcMesh.Tangents[srcIdx];

	if(!srcMesh.Bitangents.empty())
		destMesh.Bitangents[dstIdx] = srcMesh.Bitangents[srcIdx];

	if(!srcMesh.Colors.empty())
		destMesh.Colors[dstIdx] = srcMesh.Colors[srcIdx];

	for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
	{
		if(!srcMesh.UV[i].empty())
			destMesh.UV[i][dstIdx] = srcMesh.UV[i][srcIdx];
	}

	u32 numBlendShapes = (u32)srcMesh.BlendShapes.size();
	for(u32 i = 0; i < numBlendShapes; i++)
	{
		const FBXBlendShape& sourceShape = srcMesh.BlendShapes[i];
		FBXBlendShape& destShape = destMesh.BlendShapes[i];

		u32 numFrames = (u32)sourceShape.Frames.size();
		for(u32 j = 0; j < numFrames; j++)
		{
			const FBXBlendShapeFrame& sourceFrame = sourceShape.Frames[j];
			FBXBlendShapeFrame& destFrame = destShape.Frames[j];

			if(!sourceFrame.Normals.empty())
				destFrame.Normals[dstIdx] = sourceFrame.Normals[srcIdx];

			if(!sourceFrame.Tangents.empty())
				destFrame.Tangents[dstIdx] = sourceFrame.Tangents[srcIdx];

			if(!sourceFrame.Bitangents.empty())
				destFrame.Bitangents[dstIdx] = sourceFrame.Bitangents[srcIdx];
		}
	}
}

void FBXUtility::AddVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh)
{
	destMesh.Positions.push_back(srcMesh.Positions[srcVertex]);

	if(!srcMesh.BoneInfluences.empty())
		destMesh.BoneInfluences.push_back(srcMesh.BoneInfluences[srcVertex]);

	if(!srcMesh.Normals.empty())
		destMesh.Normals.push_back(srcMesh.Normals[srcIdx]);

	if(!srcMesh.Tangents.empty())
		destMesh.Tangents.push_back(srcMesh.Tangents[srcIdx]);

	if(!srcMesh.Bitangents.empty())
		destMesh.Bitangents.push_back(srcMesh.Bitangents[srcIdx]);

	if(!srcMesh.Colors.empty())
		destMesh.Colors.push_back(srcMesh.Colors[srcIdx]);

	for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
	{
		if(!srcMesh.UV[i].empty())
			destMesh.UV[i].push_back(srcMesh.UV[i][srcIdx]);
	}

	u32 numBlendShapes = (u32)srcMesh.BlendShapes.size();
	for(u32 i = 0; i < numBlendShapes; i++)
	{
		const FBXBlendShape& sourceShape = srcMesh.BlendShapes[i];
		FBXBlendShape& destShape = destMesh.BlendShapes[i];

		u32 numFrames = (u32)sourceShape.Frames.size();
		for(u32 j = 0; j < numFrames; j++)
		{
			const FBXBlendShapeFrame& sourceFrame = sourceShape.Frames[j];
			FBXBlendShapeFrame& destFrame = destShape.Frames[j];

			destFrame.Positions.push_back(sourceFrame.Positions[srcVertex]);

			if(!sourceFrame.Normals.empty())
				destFrame.Normals.push_back(sourceFrame.Normals[srcIdx]);

			if(!sourceFrame.Tangents.empty())
				destFrame.Tangents.push_back(sourceFrame.Tangents[srcIdx]);

			if(!sourceFrame.Bitangents.empty())
				destFrame.Bitangents.push_back(sourceFrame.Bitangents[srcIdx]);
		}
	}
}

bool FBXUtility::NeedsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB)
{
	static const float kSplitAngleCosine = Math::Cos(Degree(1.0f));
	static const float kUvEpsilon = 0.001f;

	if(!meshA.Colors.empty())
	{
		if(meshA.Colors[idxA] != meshB.Colors[idxB])
			return true;
	}

	if(!meshA.Normals.empty())
	{
		float angleCosine = meshA.Normals[idxA].Dot(meshB.Normals[idxB]);
		if(angleCosine < kSplitAngleCosine)
			return true;
	}

	if(!meshA.Tangents.empty())
	{
		float angleCosine = meshA.Tangents[idxA].Dot(meshB.Tangents[idxB]);
		if(angleCosine < kSplitAngleCosine)
			return true;
	}

	if(!meshA.Bitangents.empty())
	{
		float angleCosine = meshA.Bitangents[idxA].Dot(meshB.Bitangents[idxB]);
		if(angleCosine < kSplitAngleCosine)
			return true;
	}

	for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
	{
		if(!meshA.UV[i].empty())
		{
			if(!Math::ApproxEquals(meshA.UV[i][idxA], meshB.UV[i][idxB], kUvEpsilon))
				return true;
		}
	}

	u32 numBlendShapes = (u32)meshA.BlendShapes.size();
	for(u32 i = 0; i < numBlendShapes; i++)
	{
		const FBXBlendShape& shapeA = meshA.BlendShapes[i];
		const FBXBlendShape& shapeB = meshB.BlendShapes[i];

		u32 numFrames = (u32)shapeA.Frames.size();
		for(u32 j = 0; j < numFrames; j++)
		{
			const FBXBlendShapeFrame& frameA = shapeA.Frames[j];
			const FBXBlendShapeFrame& frameB = shapeB.Frames[j];

			if(!frameA.Normals.empty())
			{
				float angleCosine = frameA.Normals[idxA].Dot(frameB.Normals[idxB]);
				if(angleCosine < kSplitAngleCosine)
					return true;
			}

			if(!frameA.Tangents.empty())
			{
				float angleCosine = frameA.Tangents[idxA].Dot(frameB.Tangents[idxB]);
				if(angleCosine < kSplitAngleCosine)
					return true;
			}

			if(!frameA.Bitangents.empty())
			{
				float angleCosine = frameA.Bitangents[idxA].Dot(frameB.Bitangents[idxB]);
				if(angleCosine < kSplitAngleCosine)
					return true;
			}
		}
	}

	return false;
}
