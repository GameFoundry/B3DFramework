//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFBXPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Image/B3DColor.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector4.h"
#include "Math/B3DQuaternion.h"
#include "Animation/B3DAnimationCurve.h"
#include "RenderAPI/B3DSubMesh.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	struct RootMotion;

	/** @addtogroup FBX
	 *  @{
	 */

	/**	Options that control FBX import */
	struct FBXImportOptions
	{
		bool ImportAnimation = true;
		bool ImportSkin = true;
		bool ImportBlendShapes = true;
		bool ImportNormals = true;
		bool ImportTangents = true;
		float ImportScale = 0.01f;
		float AnimSampleRate = 1.0f / 60.0f;
		bool AnimResample = false;
		bool ReduceKeyframes = true;
	};

	/**	Represents a single node in the FBX transform hierarchy. */
	struct FBXImportNode
	{
		~FBXImportNode();

		Matrix4 GeomTransform;
		Transform LocalTransform;
		Matrix4 WorldTransform;
		String Name;
		FbxNode* FbxNode;
		bool FlipWinding;

		Vector<FBXImportNode*> Children;
	};

	/**	Contains geometry from one blend shape frame. */
	struct FBXBlendShapeFrame
	{
		Vector<Vector3> Positions;
		Vector<Vector3> Normals;
		Vector<Vector3> Tangents;
		Vector<Vector3> Bitangents;

		float Weight;
		String Name;
	};

	/**	Contains all geometry for a single blend shape. */
	struct FBXBlendShape
	{
		String Name;
		Vector<FBXBlendShapeFrame> Frames;
	};

	/**	Contains data about a single bone in a skinned mesh. */
	struct FBXBone
	{
		FBXImportNode* Node;
		Transform LocalTfrm;
		Matrix4 BindPose;
	};

	/** Contains a set of bone weights and indices for a single vertex, used in a skinned mesh. */
	struct FBXBoneInfluence
	{
		FBXBoneInfluence()
		{
			for(u32 i = 0; i < FBX_IMPORT_MAX_BONE_INFLUENCES; i++)
			{
				Weights[i] = 0.0f;
				Indices[i] = -1;
			}
		}

		float Weights[FBX_IMPORT_MAX_BONE_INFLUENCES];
		i32 Indices[FBX_IMPORT_MAX_BONE_INFLUENCES];
	};

	/**	Animation curves required to animate a single bone. */
	struct FBXBoneAnimation
	{
		FBXImportNode* Node;

		TAnimationCurve<Vector3> Translation;
		TAnimationCurve<Quaternion> Rotation;
		TAnimationCurve<Vector3> Scale;
	};

	/**	Animation curve required to animate a blend shape. */
	struct FBXBlendShapeAnimation
	{
		String BlendShape;
		TAnimationCurve<float> Curve;
	};

	/** Animation clip containing a set of bone or blend shape animations. */
	struct FBXAnimationClip
	{
		String Name;
		float Start;
		float End;
		u32 SampleRate;

		Vector<FBXBoneAnimation> BoneAnimations;
		Vector<FBXBlendShapeAnimation> BlendShapeAnimations;
	};

	/** All information required for creating an animation clip. */
	struct FBXAnimationClipData
	{
		FBXAnimationClipData(const String& name, bool isAdditive, u32 sampleRate, const SPtr<AnimationCurves>& curves, const SPtr<RootMotion>& rootMotion)
			: Name(name), IsAdditive(isAdditive), SampleRate(sampleRate), Curves(curves), RootMotion(rootMotion)
		{}

		String Name;
		bool IsAdditive;
		u32 SampleRate;
		SPtr<AnimationCurves> Curves;
		SPtr<RootMotion> RootMotion;
	};

	/**	Imported mesh data. */
	struct FBXImportMesh
	{
		FbxMesh* FbxMesh;

		Vector<int> Indices;
		Vector<Vector3> Positions;
		Vector<Vector3> Normals;
		Vector<Vector3> Tangents;
		Vector<Vector3> Bitangents;
		Vector<RGBA> Colors;
		Vector<Vector2> UV[FBX_IMPORT_MAX_UV_LAYERS];
		Vector<int> Materials;

		Vector<int> SmoothingGroups;
		Vector<FBXBlendShape> BlendShapes;

		Vector<FBXBoneInfluence> BoneInfluences;
		Vector<FBXBone> Bones;

		SPtr<MeshData> MeshData;
		Vector<SubMesh> SubMeshes;

		Vector<FBXImportNode*> ReferencedBy;
	};

	/**	Scene information used and modified during FBX import. */
	struct FBXImportScene
	{
		FBXImportScene() = default;
		~FBXImportScene();

		Vector<FBXImportMesh*> Meshes;
		FBXImportNode* RootNode = nullptr;

		UnorderedMap<FbxNode*, FBXImportNode*> NodeMap;
		UnorderedMap<FbxMesh*, u32> MeshMap;

		Vector<FBXAnimationClip> Clips;
	};

	/** @} */
} // namespace b3d
