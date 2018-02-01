//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFBXPrerequisites.h"
#include "Math/BsMatrix4.h"
#include "Image/BsColor.h"
#include "Math/BsVector2.h"
#include "Math/BsVector4.h"
#include "Math/BsQuaternion.h"
#include "Animation/BsAnimationCurve.h"
#include "RenderAPI/BsSubMesh.h"

namespace bs
{
	struct RootMotion;

	/** @addtogroup FBX
	 *  @{
	 */

	/**	Options that control FBX import */
	struct FBXImportOptions
	{
		bool importAnimation = true;
		bool importSkin = true;
		bool importBlendShapes = true;
		bool importNormals = true;
		bool importTangents = true;
		float importScale = 0.01f;
		float animSampleRate = 1.0f / 60.0f;
		bool animResample = false;
		bool reduceKeyframes = true;
	};

	/**	Represents a single node in the FBX transform hierarchy. */
	struct FBXImportNode
	{
		~FBXImportNode();

		Matrix4 geomTransform;
		Matrix4 localTransform;
		Matrix4 worldTransform;
		String name;
		FbxNode* fbxNode;
		bool flipWinding;

		Vector<FBXImportNode*> children;
	};

	/**	Contains geometry from one blend shape frame. */
	struct FBXBlendShapeFrame
	{
		Vector<Vector3> positions;
		Vector<Vector3> normals;
		Vector<Vector3> tangents;
		Vector<Vector3> bitangents;

		float weight;
		String name;
	};

	/**	Contains all geometry for a single blend shape. */
	struct FBXBlendShape
	{
		String name;
		Vector<FBXBlendShapeFrame> frames;
	};

	/**	Contains data about a single bone in a skinned mesh. */
	struct FBXBone
	{
		FBXImportNode* node;
		Matrix4 localTfrm;
		Matrix4 bindPose;
	};

	/** Contains a set of bone weights and indices for a single vertex, used in a skinned mesh. */
	struct FBXBoneInfluence
	{
		FBXBoneInfluence()
		{
			for (UINT32 i = 0; i < FBX_IMPORT_MAX_BONE_INFLUENCES; i++)
			{
				weights[i] = 0.0f;
				indices[i] = -1;
			}
		}

		float weights[FBX_IMPORT_MAX_BONE_INFLUENCES];
		INT32 indices[FBX_IMPORT_MAX_BONE_INFLUENCES];
	};

	/**	Animation curves required to animate a single bone. */
	struct FBXBoneAnimation
	{
		FBXImportNode* node;

		TAnimationCurve<Vector3> translation;
		TAnimationCurve<Quaternion> rotation;
		TAnimationCurve<Vector3> scale;
	};

	/**	Animation curve required to animate a blend shape. */
	struct FBXBlendShapeAnimation
	{
		String blendShape;
		TAnimationCurve<float> curve;
	};

	/** Animation clip containing a set of bone or blend shape animations. */
	struct FBXAnimationClip
	{
		String name;
		float start;
		float end;
		UINT32 sampleRate;

		Vector<FBXBoneAnimation> boneAnimations;
		Vector<FBXBlendShapeAnimation> blendShapeAnimations;
	};

	/** All information required for creating an animation clip. */
	struct FBXAnimationClipData
	{
		FBXAnimationClipData(const String& name, bool isAdditive, UINT32 sampleRate, const SPtr<AnimationCurves>& curves, 
			const SPtr<RootMotion>& rootMotion)
			:name(name), isAdditive(isAdditive), sampleRate(sampleRate), curves(curves), rootMotion(rootMotion)
		{ }

		String name;
		bool isAdditive;
		UINT32 sampleRate;
		SPtr<AnimationCurves> curves;
		SPtr<RootMotion> rootMotion;
	};

	/**	Imported mesh data. */
	struct FBXImportMesh
	{
		FbxMesh* fbxMesh;

		Vector<int> indices;
		Vector<Vector3> positions;
		Vector<Vector3> normals;
		Vector<Vector3> tangents;
		Vector<Vector3> bitangents;
		Vector<RGBA> colors;
		Vector<Vector2> UV[FBX_IMPORT_MAX_UV_LAYERS];
		Vector<int> materials;

		Vector<int> smoothingGroups;
		Vector<FBXBlendShape> blendShapes;

		Vector<FBXBoneInfluence> boneInfluences;
		Vector<FBXBone> bones;

		SPtr<MeshData> meshData;
		Vector<SubMesh> subMeshes;

		Vector<FBXImportNode*> referencedBy;
	};

	/**	Scene information used and modified during FBX import. */
	struct FBXImportScene
	{
		FBXImportScene();
		~FBXImportScene();

		Vector<FBXImportMesh*> meshes;
		FBXImportNode* rootNode;

		UnorderedMap<FbxNode*, FBXImportNode*> nodeMap;
		UnorderedMap<FbxMesh*, UINT32> meshMap;

		Vector<FBXAnimationClip> clips;

		float scaleFactor;
		Matrix4 globalScale;
	};

	/** @} */
}